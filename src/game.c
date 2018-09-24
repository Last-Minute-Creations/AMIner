/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
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

static tView *s_pView;
static tVPort *s_pVpMain;
tTileBufferManager *g_pMainBuffer;

static tBitMap *s_pTiles;
static UBYTE s_isDebug = 0;

void gameGsCreate(void) {
  s_pView = viewCreate(0,
    TAG_VIEW_GLOBAL_CLUT, 1,
  TAG_END);

	hudCreate(s_pView);
	s_pTiles = bitmapCreateFromFile("data/tiles.bm");

  s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 4,
  TAG_END);
  g_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, 10,
		TAG_TILEBUFFER_BOUND_TILE_Y, 2047,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 5,
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
		TAG_TILEBUFFER_TILESET, s_pTiles,
  TAG_END);

	paletteLoad("data/aminer.plt", s_pVpMain->pPalette, 16);

	randInit(2184);

	tileInit();

	bobNewManagerCreate(
		1, VEHICLE_HEIGHT * (VEHICLE_WIDTH/16 + 1),
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	windowInit();
	vehicleCreate();
	systemUnuse();

	s_isDebug = 0;
	tileBufferInitialDraw(g_pMainBuffer);

  // Load the view
  viewLoad(s_pView);
}

static void gameProcessInput(void) {
	BYTE bDirX = 0, bDirY = 0;
	if(keyCheck(KEY_D)) {
		bDirX += 1;
	}
	if(keyCheck(KEY_A)) {
		bDirX -= 1;
	}
	if(keyCheck(KEY_S)) {
		bDirY += 1;
	}
	if(keyCheck(KEY_W)) {
		bDirY -= 1;
	}
	vehicleMove(bDirX, bDirY);
}

void gameGsLoop(void) {
  if(keyCheck(KEY_ESCAPE)) {
    gameClose();
		return;
  }
	if(keyUse(KEY_B)) {
		s_isDebug = !s_isDebug;
	}
	if(keyUse(KEY_L)) {
		gamePushState(vendorGsCreate, vendorGsLoop, vendorGsDestroy);
		return;
	}

	if(s_isDebug) {
		g_pCustom->color[0] = 0x008;
	}

	bobNewBegin();
	tileBufferQueueProcess(g_pMainBuffer);
	gameProcessInput();
	vehicleProcess();
	hudSetDepth(g_sVehicle.sBob.sPos.sUwCoord.uwY + VEHICLE_HEIGHT);
	hudSetScore(g_sVehicle.ulScore);
	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	cameraCenterAt(
		g_pMainBuffer->pCamera,
		g_sVehicle.sBob.sPos.sUwCoord.uwX + VEHICLE_WIDTH / 2,
		g_sVehicle.sBob.sPos.sUwCoord.uwY + VEHICLE_HEIGHT / 2
	);
	if(s_isDebug) {
		g_pCustom->color[0] = 0x800;
	}
	viewProcessManagers(s_pView);
	copProcessBlocks();
	g_pCustom->color[0] = 0x000;
	vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  // Cleanup when leaving this gamestate
  systemUse();

	bitmapDestroy(s_pTiles);
	vehicleDestroy();
	windowDeinit();
	bobNewManagerDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}
