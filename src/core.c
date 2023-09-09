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

#define CORE_INIT_BAR_MARGIN 10
#define CORE_INIT_BAR_WIDTH (SCREEN_PAL_WIDTH - 2 * CORE_INIT_BAR_MARGIN)
#define CORE_INIT_BAR_HEIGHT 10
#define CORE_INIT_BAR_BORDER_DISTANCE 2

static tBitMap *s_pTiles;
static UWORD s_pPaletteRef[1 << GAME_BPP];
static UWORD *s_pColorBg;

static tView *s_pView;
static tVPort *s_pVpMain;
static tBitMap *s_pBombMarker, *s_pBombMarkerMask;
static const char *s_szLangPrefix;

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

void coreProcessBeforeBobs(void) {
	// Undraw all bobs
	debugColor(0x008);
	bobBegin(g_pMainBuffer->pScroll->pBack);

	// Draw pending tiles
	tileBufferQueueProcess(g_pMainBuffer);

	// Draw collectibles before anything else
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
	// blitRect(g_pMainBuffer->pScroll->pFront, 50, 50, 100, 100, 3);
	// blitRect(g_pMainBuffer->pScroll->pBack, 200, 50, 100, 100, 4); // This one gets displayed
	viewProcessManagers(s_pView);
	copProcessBlocks();
	progressBarInit(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront);

	defsInit();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 3);
	langCreate(s_szLangPrefix);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 6);
	hiScoreLoad();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 9);
	textBobManagerCreate(g_pFont);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 12);
	dinoReset();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 15);
	questGateReset();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 18);
	collectiblesCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 21);
	hudCreate(pVpHud, g_pFont);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 24);

	baseCreate(g_pMainBuffer);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 27);
	ptplayerCreate(1);
	ptplayerSetChannelsForPlayer(0b0111);
	ptplayerSetMasterVolume(8);
	audioMixerCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 30);
	g_pSfxFlyLoop = ptplayerSfxCreateFromFile("data/sfx/fly_loop.sfx", 1);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 33);
	g_pSfxDrill = ptplayerSfxCreateFromFile("data/sfx/drill1.sfx", 1);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 36);
	g_pSfxOre = ptplayerSfxCreateFromFile("data/sfx/ore2.sfx", 1);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 39);
	g_pSfxPenalty = ptplayerSfxCreateFromFile("data/sfx/penalty.sfx", 1);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 42);
	for(UBYTE i = 0; i < GAME_MOD_COUNT; ++i) {
		char szModPath[30];
		sprintf(szModPath, "data/music/game%hhu.mod", i);
		g_pGameMods[i] = ptplayerModCreate(szModPath);
	}
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 45);
	g_pMenuMod = ptplayerModCreate("data/music/menu.mod");
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 48);
	g_pModSampleData = ptplayerSampleDataCreate("data/music/samples.samplepack");
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 51);

#ifdef GAME_DEBUG
	randInit(&g_sRand, 2184, 1911);
#else
	// Seed from beam pos Y & X
	tRayPos sRayPos = getRayPos();
	randInit(&g_sRand, 1 + (sRayPos.bfPosY << 8), 1 + sRayPos.bfPosX);
#endif

	tileReset(0, 1);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 54);

	bobManagerCreate(
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 57);
	explosionManagerCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 60);
	groundLayerCreate(s_pVpMain);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 63);
	commCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 66);
	vehicleBitmapsCreate();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 69);
	vehicleCreate(&g_pVehicles[0], PLAYER_1);
	vehicleCreate(&g_pVehicles[1], PLAYER_2);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 72);

	s_pBombMarker = bitmapCreateFromFile("data/bomb_marker.bm", 0);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 75);
	s_pBombMarkerMask = bitmapCreateFromFile("data/bomb_marker_mask.bm", 0);
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 78);

	for(UBYTE i = 0; i < 3; ++i) {
		bobInit(
			&g_pBombMarkers[i], 16, 10, 1,
			bobCalcFrameAddress(s_pBombMarker, 0),
			bobCalcFrameAddress(s_pBombMarkerMask, 0),
			0, 0
		);
	}

	menuPreload();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 81);
	bobReallocateBgBuffers();
	progressBarAdvance(&s_sProgressBarConfig, g_pMainBuffer->pScroll->pFront, 84);
	systemUnuse();

	g_pMainBuffer->pCamera->uPos.uwX = 32;
	memset(pVpHud->pPalette, 0, sizeof(pVpHud->pPalette));
	s_pColorBg = &pVpHud->pPalette[0];
	viewUpdatePalette(s_pView);

	// Initial background
	tileBufferRedrawAll(g_pMainBuffer);

	// Default config
	g_is2pPlaying = 0;

	hudReset(0, 0);
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
	fontDestroy(g_pFont);
	vehicleDestroy(&g_pVehicles[0]);
	vehicleDestroy(&g_pVehicles[1]);
	vehicleBitmapsDestroy();
	commDestroy();
	bobManagerDestroy();

	audioMixerDestroy();
	for(UBYTE i = 0; i < GAME_MOD_COUNT; ++i) {
		ptplayerModDestroy(g_pGameMods[i]);
	}
	ptplayerSamplePackDestroy(g_pModSampleData);
	ptplayerModDestroy(g_pMenuMod);
	ptplayerSfxDestroy(g_pSfxDrill);
	ptplayerSfxDestroy(g_pSfxFlyLoop);
	ptplayerSfxDestroy(g_pSfxOre);
	ptplayerSfxDestroy(g_pSfxPenalty);
	ptplayerDestroy();

	bitmapDestroy(s_pBombMarker);
	bitmapDestroy(s_pBombMarkerMask);

	explosionManagerDestroy();
	langDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}

void coreSetLangPrefix(const char * const szPrefix) {
	logBlockBegin("coreSetLangPrefix(szPrefix: %s)", szPrefix);
	s_szLangPrefix = szPrefix;
	logBlockEnd("coreSetLangPrefix()");
}

const char * coreGetLangPrefix(void) {
	return s_szLangPrefix;
}

//---------------------------------------------------------------------- GLOBALS

tTileBufferManager *g_pMainBuffer;
tFont *g_pFont;
tRandManager g_sRand;
tPtplayerSamplePack *g_pModSampleData;

tState g_sStateCore = {
	.cbCreate = coreGsCreate, .cbLoop = coreGsLoop, .cbDestroy = coreGsDestroy
};
