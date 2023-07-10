/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "core.h"
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
#include "base_tile.h"
#include "ground_layer.h"
#include "hi_score.h"
#include "tile.h"
#include "explosion.h"
#include <comm/comm.h>
#include "defs.h"
#include "settings.h"
#include "collectibles.h"

static tBitMap *s_pTiles;
static UWORD s_pPaletteRef[1 << GAME_BPP];
static UWORD *s_pColorBg;

static tView *s_pView;
static tVPort *s_pVpMain;
static tBitMap *s_pBombMarker, *s_pBombMarkerMask;
static const char *s_szLangPrefix;

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
	baseTileProcess();

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
	defsInit();
	langCreate(s_szLangPrefix);
	hiScoreLoad();
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_PALETTE, 1,
	TAG_END);

	textBobManagerCreate(g_pFont);
	s_pTiles = bitmapCreateFromFile("data/tiles.bm", 0);
	dinoReset();
	questGateReset();
	collectiblesCreate();

	hudCreate(s_pView, g_pFont);

	s_pVpMain = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, GAME_BPP,
	TAG_END);
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

	paletteLoad("data/aminer.plt", s_pPaletteRef, 1 << GAME_BPP);
	memset(s_pVpMain->pPalette, 0, sizeof(s_pVpMain->pPalette));
	s_pColorBg = &s_pVpMain->pPalette[0];

	baseTileCreate(g_pMainBuffer);
	ptplayerCreate(1);
	ptplayerSetChannelsForPlayer(0b0111);
	ptplayerSetMasterVolume(8);
	audioMixerCreate();
	g_pSfxDrill = ptplayerSfxCreateFromFile("data/sfx/drill1.sfx", 1);
	g_pSfxOre = ptplayerSfxCreateFromFile("data/sfx/ore2.sfx", 1);
	g_pSfxPenalty = ptplayerSfxCreateFromFile("data/sfx/penalty.sfx", 1);
	for(UBYTE i = 0; i < GAME_MOD_COUNT; ++i) {
		char szModPath[30];
		sprintf(szModPath, "data/music/game%hhu.mod", i);
		g_pGameMods[i] = ptplayerModCreate(szModPath);
	}
	g_pMenuMod = ptplayerModCreate("data/music/menu.mod");
	g_pModSampleData = ptplayerSampleDataCreate("data/music/samples.samplepack");

#ifdef GAME_DEBUG
	randInit(&g_sRand, 2184, 1911);
#else
	// Seed from beam pos Y & X
	tRayPos sRayPos = getRayPos();
	randInit(&g_sRand, 1 + (sRayPos.bfPosY << 8), 1 + sRayPos.bfPosX);
#endif

	tileReset(0, 1);

	bobManagerCreate(
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	explosionManagerCreate();
	groundLayerCreate(s_pVpMain);
	commCreate();
	vehicleBitmapsCreate();
	vehicleCreate(&g_pVehicles[0], PLAYER_1);
	vehicleCreate(&g_pVehicles[1], PLAYER_2);

	s_pBombMarker = bitmapCreateFromFile("data/bomb_marker.bm", 0);
	s_pBombMarkerMask = bitmapCreateFromFile("data/bomb_marker_mask.bm", 0);

	for(UBYTE i = 0; i < 3; ++i) {
		bobInit(
			&g_pBombMarkers[i], 16, 10, 1,
			bobCalcFrameAddress(s_pBombMarker, 0),
			bobCalcFrameAddress(s_pBombMarkerMask, 0),
			0, 0
		);
	}

	menuPreload();
	bobReallocateBgBuffers();
	systemUnuse();

	g_pMainBuffer->pCamera->uPos.uwX = 32;
	// Initial background
	tileBufferRedrawAll(g_pMainBuffer);

	// Load the view
	viewLoad(s_pView);

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
	baseTileDestroy();
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
