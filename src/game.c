/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/custom.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include "bob_new.h"
#include "vehicle.h"
#include "hud.h"
#include "tile.h"
#include "window.h"
#include "vendor.h"
#include "menu.h"

static tView *s_pView;
static tVPort *s_pVpMain;
tTileBufferManager *g_pMainBuffer;

static tBitMap *s_pTiles;
static UBYTE s_isDebug = 0;
static UWORD s_uwColorBg;
tFont *g_pFont;
UBYTE g_is2pPlaying;
UBYTE g_is1pKbd, g_is2pKbd;

static void goToMenu(void) {
	// Switch to menu, after popping it will process gameGsLoop
	gamePushState(menuGsCreate, menuGsLoop, menuGsDestroy);
}

void gameStart(void) {
	tileInit(0);
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	hudReset();
}

void gameGsCreate(void) {
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_END);

	g_pFont = fontCreate("data/silkscreen5.fnt");
	s_pTiles = bitmapCreateFromFile("data/tiles.bm");
	hudCreate(s_pView, g_pFont);

	s_pVpMain = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 4,
	TAG_END);
	g_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, 11,
		TAG_TILEBUFFER_BOUND_TILE_Y, 2047,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 5,
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
		TAG_TILEBUFFER_TILESET, s_pTiles,
	TAG_END);

	paletteLoad("data/aminer.plt", s_pVpMain->pPalette, 16);
	s_uwColorBg = s_pVpMain->pPalette[0];

	randInit(2184);

	tileInit(0);

	bobNewManagerCreate(
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	windowInit();
	vehicleBitmapsCreate();
	vehicleCreate(&g_pVehicles[0], PLAYER_1);
	vehicleCreate(&g_pVehicles[1], PLAYER_2);
	menuPreload();
	bobNewAllocateBgBuffers();
	systemUnuse();

	g_pMainBuffer->pCamera->uPos.sUwCoord.uwX = 32;

	s_isDebug = 0;

	// Default config
	g_is2pPlaying = 0;
	g_is1pKbd = 0;
	g_is2pKbd = 1;

	// Initial background
	tileBufferInitialDraw(g_pMainBuffer);

	// Load the view
	viewLoad(s_pView);
	goToMenu();
}

static void gameProcessInput(void) {
	BYTE bDirX = 0, bDirY = 0;
	if(g_is1pKbd) {
		if(keyCheck(KEY_D)) { bDirX += 1; }
		if(keyCheck(KEY_A)) { bDirX -= 1; }
		if(keyCheck(KEY_S)) { bDirY += 1; }
		if(keyCheck(KEY_W)) { bDirY -= 1; }
	}
	else {
		if(joyCheck(JOY1_RIGHT)) { bDirX += 1; }
		if(joyCheck(JOY1_LEFT)) { bDirX -= 1; }
		if(joyCheck(JOY1_DOWN)) { bDirY += 1; }
		if(joyCheck(JOY1_UP)) { bDirY -= 1; }
	}
	vehicleMove(&g_pVehicles[0], bDirX, bDirY);

	bDirX = 0;
	bDirY = 0;
	if(g_is2pKbd) {
		if(keyCheck(KEY_RIGHT)) { bDirX += 1; }
		if(keyCheck(KEY_LEFT)) { bDirX -= 1; }
		if(keyCheck(KEY_DOWN)) { bDirY += 1; }
		if(keyCheck(KEY_UP)) { bDirY -= 1; }
	}
	else {
		if(joyCheck(JOY2_RIGHT)) { bDirX += 1; }
		if(joyCheck(JOY2_LEFT)) { bDirX -= 1; }
		if(joyCheck(JOY2_DOWN)) { bDirY += 1; }
		if(joyCheck(JOY2_UP)) { bDirY -= 1; }
	}
	vehicleMove(&g_pVehicles[1], bDirX, bDirY);
}

static inline void debugColor(UWORD uwColor) {
	if(s_isDebug) {
		g_pCustom->color[0] = uwColor;
	}
}

void gameGsLoop(void) {
  if(keyUse(KEY_ESCAPE)) {
    goToMenu();
		return;
  }
	if(keyUse(KEY_B)) {
		s_isDebug = !s_isDebug;
	}
	if(keyUse(KEY_L)) {
		gamePushState(vendorGsCreate, vendorGsLoop, vendorGsDestroy);
		return;
	}

	debugColor(0x008);
	bobNewBegin();
	tileBufferQueueProcess(g_pMainBuffer);
	gameProcessInput();
	vehicleProcessText();
	debugColor(0x080);
	vehicleProcess(&g_pVehicles[0]);
	if(g_is2pPlaying) {
		debugColor(0x880);
		vehicleProcess(&g_pVehicles[1]);
	}
	debugColor(0x088);
	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	UWORD uwCamX = 32, uwCamY;
	if(!g_is2pPlaying) {
		// One player only
		// uwCamX = fix16_to_int(g_pVehicles[0].fX) + VEHICLE_WIDTH / 2;
		uwCamY = fix16_to_int(g_pVehicles[0].fY) + VEHICLE_HEIGHT / 2;
	}
	else {
		// Two players
		// uwCamX = (fix16_to_int(g_pVehicles[0].fX) + fix16_to_int(g_pVehicles[1].fX) + VEHICLE_WIDTH) / 2;
		uwCamY = (fix16_to_int(g_pVehicles[0].fY) + fix16_to_int(g_pVehicles[1].fY) + VEHICLE_HEIGHT) / 2;
	}
	cameraCenterAt(
		g_pMainBuffer->pCamera, uwCamX, uwCamY);
	if(g_pMainBuffer->pCamera->uPos.sUwCoord.uwX < 32) {
		g_pMainBuffer->pCamera->uPos.sUwCoord.uwX = 32;
	}
	debugColor(0x800);
	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugColor(s_uwColorBg);
	vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  // Cleanup when leaving this gamestate
  systemUse();

	menuUnload();
	bitmapDestroy(s_pTiles);
	fontDestroy(g_pFont);
	vehicleDestroy(&g_pVehicles[0]);
	vehicleDestroy(&g_pVehicles[1]);
	vehicleBitmapsDestroy();
	windowDeinit();
	bobNewManagerDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}
