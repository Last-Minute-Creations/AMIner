/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base_tile.h"
#include "game.h"
#include "tile.h"

static UBYTE s_ubBaseCurrent;
static UBYTE s_isFinishLoaded;
static tBitMap *s_pBaseTiles[3];
static tBitMap *s_pCheckpointTiles;
tTileBufferManager *s_pManager;

static void baseTileLoad(UBYTE ubBaseIdx) {
	const tBitMap *pBase = s_pBaseTiles[ubBaseIdx];
	tBitMap *pTiles = s_pManager->pTileSet;
	memcpy(
		pTiles->Planes[0], pBase->Planes[0],
		pBase->Rows * pBase->BytesPerRow
	);
	s_ubBaseCurrent = ubBaseIdx;
}

void baseTileCreate(tTileBufferManager *pManager) {
	s_pManager = pManager;
	s_pBaseTiles[0] = bitmapCreateFromFile("data/base0.bm", 1);
	s_pBaseTiles[1] = bitmapCreateFromFile("data/base1.bm", 1);
	s_pBaseTiles[2] = bitmapCreateFromFile("data/base2.bm", 1);
	s_pCheckpointTiles = bitmapCreateFromFile("data/checkpoint.bm", 1);
	baseTileLoad(0);
	s_isFinishLoaded = 0;
}

void baseTileDestroy(void) {
	bitmapDestroy(s_pBaseTiles[0]);
	bitmapDestroy(s_pBaseTiles[1]);
	bitmapDestroy(s_pBaseTiles[2]);
	bitmapDestroy(s_pCheckpointTiles);
}

#define TILE_BYTE_COUNT (32 * 4 * 5)

void baseTileProcess(void) {
	UWORD uwCamY = s_pManager->pCamera->uPos.uwY;
	if(g_isChallenge) {
		tBitMap *pTiles = s_pManager->pTileSet;
		if(uwCamY >= TILE_ROW_CHALLENGE_CHECKPOINT_3 * 32) {
			if(!s_isFinishLoaded) {
				memcpy(
					&pTiles->Planes[0][TILE_CHECKPOINT_1 * TILE_BYTE_COUNT],
					&s_pCheckpointTiles->Planes[0][10 * TILE_BYTE_COUNT],
					10 * TILE_BYTE_COUNT
				);
				s_isFinishLoaded = 1;
			}
		}
		else if(s_isFinishLoaded) {
			memcpy(
				&pTiles->Planes[0][TILE_CHECKPOINT_1 * TILE_BYTE_COUNT],
				s_pCheckpointTiles->Planes[0],
				10 * TILE_BYTE_COUNT
			);
			s_isFinishLoaded = 0;
		}
	}
	else {
		if(uwCamY > 190*32) {
			if(s_ubBaseCurrent != 2) {
				baseTileLoad(2);
			}
		}
		else if(uwCamY > 90*32) {
			if(s_ubBaseCurrent != 1) {
				baseTileLoad(1);
			}
		}
		else if(uwCamY < 20*32) {
			if(s_ubBaseCurrent != 0) {
				baseTileLoad(0);
			}
		}
	}
}
