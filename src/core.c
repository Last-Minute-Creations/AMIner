/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "core.h"
#include <ace/generic/screen.h>
#include <ace/managers/rand.h>
#include <ace/managers/system.h>
#include <ace/managers/key.h>
#include <ace/utils/palette.h>
#include <ace/contrib/managers/audio_mixer.h>
#include "menu.h"
#include "dino.h"
#include "quest_gate.h"
#include "game.h"
#include "hud.h"
#include "vehicle.h"
#include "fade.h"
#include "debug.h"
#include "base.h"
#include "ground_layer.h"
#include "hi_score.h"
#include "tile.h"
#include "explosion.h"
#include <comm/comm.h>
#include "defs.h"
#include "settings.h"
#include "collectibles.h"
#include "progress_bar.h"
#include "assets.h"
#include "bob_sequence.h"
#include "language.h"

#define CORE_INIT_BAR_MARGIN 10
#define CORE_INIT_BAR_WIDTH (SCREEN_PAL_WIDTH - 2 * CORE_INIT_BAR_MARGIN)
#define CORE_INIT_BAR_HEIGHT 10
#define CORE_INIT_BAR_BORDER_DISTANCE 2
#define DRIP_ANIM_LENGTH 11

static tBitMap *s_pTiles;
static UWORD s_pPaletteRef[1 << GAME_BPP];
static UWORD *s_pColorBg;

static tView *s_pView;
static tVPort *s_pVpMain;
static tBitMap *s_pDripBitmap;
static tBitMap *s_pDripMask;
static tBobAnimFrame s_pDripFrames[DRIP_ANIM_LENGTH];

static const tProgressBarConfig s_sProgressBarConfig = {
	.sBarPos = {
		.uwX = CORE_INIT_BAR_MARGIN,
		.uwY = SCREEN_PAL_HEIGHT - HUD_HEIGHT - CORE_INIT_BAR_MARGIN - CORE_INIT_BAR_HEIGHT
	},
	.uwWidth =  CORE_INIT_BAR_WIDTH,
	.uwHeight = CORE_INIT_BAR_HEIGHT,
	.ubBorderDistance = CORE_INIT_BAR_BORDER_DISTANCE,
	.ubColorBorder = COMM_DISPLAY_COLOR_TEXT,
	.ubColorBar = COMM_DISPLAY_COLOR_TEXT_DARK,
};

static void mainPaletteProcess(UBYTE ubFadeLevel) {
	tFadeState eState = fadeGetState();
	if(eState == FADE_STATE_IN_MORPHING || eState == FADE_STATE_OUT_MORPHING) {
		*s_pColorBg = paletteColorDim(s_pPaletteRef[0], ubFadeLevel);
		paletteDim(s_pPaletteRef, g_pCustom->color, 27, ubFadeLevel);
	}
}

static void coreBobSequencesCreate(void) {
	s_pDripMask = bitmapCreate(16, 20, GAME_BPP, BMF_INTERLEAVED);
	blitRect(s_pDripMask, 0, 0, 16, 20, (1 << GAME_BPP) - 1);
	bobSequenceReset(s_pDripMask->Planes[0]);

	s_pDripBitmap = bitmapCreateFromFile("data/bg_factory_drip.bm", 0);
	for(UBYTE i = 0; i < DRIP_ANIM_LENGTH; ++i) {
		s_pDripFrames[i].pAddrFrame = bobCalcFrameAddress(s_pDripBitmap, i * 20);
	}

	bobSequenceAdd((tUwRect){.uwX = 32 + 247, .uwY = 233, .uwWidth = 16, .uwHeight = 20}, s_pDripFrames, DRIP_ANIM_LENGTH, 5);
}

static void coreBobSequencesDestroy(void) {
	bitmapDestroy(s_pDripMask);
	bitmapDestroy(s_pDripBitmap);
}

void coreProcessBeforeBobs(void) {
	// Undraw all bobs
	debugColor(0x008);
	bobBegin(g_pMainBuffer->pScroll->pBack);

	// Draw pending tiles
	tileBufferQueueProcess(g_pMainBuffer);

	// Draw collectibles and bg anims before anything else
	bobSequenceProcess(g_pMainBuffer);
	collectiblesProcess();
}

void coreProcessAfterBobs(void) {

	// Finish bob drawing
	bobPushingDone();
	bobEnd();

	// Update HUD state machine and draw stuff
	hudUpdate();

	// Load next base tiles, if needed
	baseProcess();

	// Update palette for new ground layers, also take into account fade level
	fadeProcess();
	UBYTE ubFadeLevel = fadeGetLevel();
	groundLayerProcess(g_pMainBuffer->pCamera->uPos.uwY, ubFadeLevel);
	mainPaletteProcess(ubFadeLevel);

	debugColor(0x800);
	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugColor(*s_pColorBg);
	systemIdleBegin();
	vPortWaitForEnd(s_pVpMain);
	systemIdleEnd();
}

static void coreGsCreate(void) {
	// Create bare-minimum display
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_PALETTE, 1,
	TAG_END);

  tVPort *pVpHud = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, GAME_BPP,
    TAG_VPORT_HEIGHT, HUD_HEIGHT,
  TAG_END);

	s_pVpMain = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, GAME_BPP,
	TAG_END);

	s_pTiles = bitmapCreateFromFile("data/tiles.bm", 0);
	g_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, 11,
		TAG_TILEBUFFER_BOUND_TILE_Y, 32768 / 32,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 5,
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
		TAG_TILEBUFFER_TILESET, s_pTiles,
	TAG_END);

	// Load the view and draw the progress bar
	paletteLoad("data/aminer.plt", s_pPaletteRef, 1 << GAME_BPP);
	memcpy(pVpHud->pPalette, s_pPaletteRef, sizeof(pVpHud->pPalette));
	viewLoad(s_pView);
	viewProcessManagers(s_pView);
	copProcessBlocks();
	progressBarInit(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront);

	defsInit();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 5);
	langCreate(languageGetPrefix());
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 10);
	hiScoreLoad();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 15);
	textBobManagerCreate(g_pFont);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 20);
	dinoReset();
	questGateReset();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 25);
	collectiblesCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 30);
	hudCreate(pVpHud, g_pFont);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 35);

	baseCreate(g_pMainBuffer);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 40);
	audioMixerCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 45);
	assetsAudioCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 50);

#ifdef GAME_DEBUG
	randInit(&g_sRand, 2184, 1911);
#else
	// Seed from beam pos Y & X
	tRayPos sRayPos = getRayPos();
	randInit(&g_sRand, 1 + (sRayPos.bfPosY << 8), 1 + sRayPos.bfPosX);
#endif

	tileReset(0, 1);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 55);

	bobManagerCreate(
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 60);
	explosionManagerCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 65);
	groundLayerCreate(s_pVpMain);
	coreBobSequencesCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 70);
	commCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 75);
	vehicleManagerCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 80);

	assetsBombMarkersCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 85);
	gameInitBombMarkerBobs();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 90);

	menuPreload();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 95);
	bobReallocateBgBuffers();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 100);
	systemUnuse();

	// Prepare for game display
	g_pMainBuffer->pCamera->uPos.uwX = 32;
	memset(pVpHud->pPalette, 0, sizeof(pVpHud->pPalette));
	s_pColorBg = &pVpHud->pPalette[0];
	viewUpdatePalette(s_pView);

	// Initial background
	tileBufferRedrawAll(g_pMainBuffer);

	// Default config
	g_is2pPlaying = 0;

	hudReset(0, 0);
	fadeMorphTo(FADE_STATE_IN);
	statePush(g_pGameStateManager, &g_sStateMenu);
}

static void coreGsLoop(void) {
	// you shouldn't be here!
	statePopAll(g_pGameStateManager);
}

static void coreGsDestroy(void) {
	systemUse();

	menuUnload();
	bitmapDestroy(s_pTiles);
	collectiblesDestroy();
	baseDestroy();
	textBobManagerDestroy();
	vehicleManagerDestroy();
	commDestroy();
	bobManagerDestroy();

	audioMixerDestroy();
	assetsAudioDestroy();

	assetsBombMarkersDestroy();
	explosionManagerDestroy();
	coreBobSequencesDestroy();
	langDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}

//---------------------------------------------------------------------- GLOBALS

tTileBufferManager *g_pMainBuffer;
tRandManager g_sRand;

tState g_sStateCore = {
	.cbCreate = coreGsCreate, .cbLoop = coreGsLoop, .cbDestroy = coreGsDestroy
};
