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

static tView *s_pView;
static tVPort *s_pVpMain;
tTileBufferManager *g_pMainBuffer;

static tBitMap *s_pTiles;

#define TILE_NONE 0
#define TILE_ROCK_1 1
#define TILE_ROCK_2 2
#define TILE_STONE_1 3
#define TILE_STONE_2 4
#define TILE_GOLD_1 5
#define TILE_GOLD_2 6
#define TILE_GOLD_3 7
#define TILE_GOLD_4 8

static UBYTE s_isDebug = 0;

// TODO sapphire, emerald, topaz

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

	for(UWORD x = 0; x < g_pMainBuffer->uTileBounds.sUwCoord.uwX; ++x) {
		g_pMainBuffer->pTileData[x][0] = TILE_NONE;
		g_pMainBuffer->pTileData[x][1] = TILE_NONE;
		g_pMainBuffer->pTileData[x][2] = TILE_NONE;
		for(UWORD y = 3; y < g_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
			if(ubRandMax(100) < 10) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_GOLD_1, TILE_GOLD_4);
			}
			else if(ubRandMax(100) < 5) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_STONE_1, TILE_STONE_2);
			}
			else {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_ROCK_1, TILE_ROCK_2);
			}
		}
	}

	bobNewManagerCreate(
		1, VEHICLE_HEIGHT * (VEHICLE_WIDTH/16 + 1),
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
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

	if(s_isDebug) {
		g_pCustom->color[0] = 0x008;
	}

	bobNewBegin();
	tileBufferQueueProcess(g_pMainBuffer);
	gameProcessInput();
	vehicleProcess();
	hudSetDepth(g_sVehicle.sBob.sPos.sUwCoord.uwY + VEHICLE_HEIGHT);
	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	cameraCenterAt(
		g_pMainBuffer->pCamera,
		g_sVehicle.sBob.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2,
		g_sVehicle.sBob.sPos.sUwCoord.uwY + VEHICLE_WIDTH/2
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
	bobNewManagerDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}
