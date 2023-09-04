/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base.h"
#include "game.h"
#include "tile.h"

#define BASE_TILE_DEPTH_GROUND 0
#define BASE_TILE_DEPTH_DINO 100
#define BASE_TILE_DEPTH_GATE 209
#define BASE_TILE_LOADING_MARGIN 10

//----------------------------------------------------------------- PRIVATE VARS

static tBaseId s_eBaseCurrent;
static UBYTE s_isFinishLineLoaded;
static tBitMap *s_pBaseTiles[BASE_ID_COUNT_UNIQUE];
static tBitMap *s_pCheckpointTiles;
static tTileBufferManager *s_pManager;

static const tBase s_pBases[BASE_ID_COUNT] = {
	[BASE_ID_GROUND] = {
		.uwTileDepth = 0,
		.pTilePattern = {
			 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
			 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
			 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
			 4,  4,  4,  4,  4,  4,  4,  4,  4,  5,
			 6,  6,  7,  6,  6,  8,  9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
			23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			34, 33, 35, 36, 37, 38, 39, 40, 41, 42,
			63, 57, 63, 64, 63, 64, 63, 64, 63, 64,
		},
		.sRectRestock = {
			.uwX1 = 7 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_GROUND + 7) * TILE_SIZE,
			.uwX2 = 9 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_GROUND + 8) * TILE_SIZE
		}
	},
	[BASE_ID_DINO] = {
		.uwTileDepth = 100,
		.pTilePattern = {
			44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
			 0, 43, 43, 43, 43,  1, 43,  0, 43, 43,
			43,  1, 43, 43, 43, 43, 43, 43, 43,  2,
			43, 43, 43, 43, 43,  3,  1, 43, 43,  1,
			43, 43, 43, 43,  3, 43, 43, 43,  0, 43,
			43,  1, 43, 43, 43, 43, 43, 43, 43,  2,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
			23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			34, 35, 36, 37, 38, 33, 39, 40, 41, 42,
			63, 64, 63, 64, 63, 57, 63, 64, 63, 64
		},
		.sRectRestock = {
			.uwX1 = 1 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_DINO + 7) * TILE_SIZE,
			.uwX2 = 3 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_DINO + 8) * TILE_SIZE
		}
	},
	[BASE_ID_GATE] = {
		.uwTileDepth = 209,
		.pTilePattern = {
			44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
			 0, 43, 43, 43, 43,  1, 43,  0, 43, 43,
			43,  1, 43, 43, 43, 43, 43, 43, 43,  2,
			43, 43, 43, 43, 43,  3,  1, 43, 43,  1,
			 4,  5, 43, 43,  6,  5, 43, 43,  0, 43,
			 7,  8,  1, 43,  9, 10,  2, 11, 43, 43,
			12, 13, 43, 43, 14, 15, 16, 17, 18, 19,
			20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			34, 35, 36, 37, 38, 33, 39, 40, 41, 42,
			63, 64, 63, 64, 63, 57, 63, 64, 63, 64,
		},
		.sRectRestock = {
			.uwX1 = 8 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_GATE + 7) * TILE_SIZE,
			.uwX2 = 10 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_GATE + 8) * TILE_SIZE
		}
	},
	[BASE_ID_DINO_POPULATED] = {
		.uwTileDepth = BASE_TILE_DEPTH_VARIANT,
		.pTilePattern = {
			44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
			 0, 43, 43, 43, 43,  1, 43,  0, 43, 43,
			43,  1, 43, 43, 43, 43, 43, 43, 43,  2,
			43, 43, 43, 43, 43,  3,  1, 43, 43,  1,
			43, 43, 43, 43,  4, 43, 43, 43,  0, 43,
			43,  3,  5,  6,  7,  8,  9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
			23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			34, 35, 36, 37, 38, 33, 39, 40, 41, 42,
			63, 64, 63, 64, 63, 57, 63, 64, 63, 64
		},
		.sRectRestock = {
			.uwX1 = 1 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_DINO + 7) * TILE_SIZE,
			.uwX2 = 3 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_DINO + 8) * TILE_SIZE
		}
	},
};

//------------------------------------------------------------------ PUBLIC VARS

//------------------------------------------------------------------ PRIVATE FNS

static void baseTileLoad(tBaseId eBaseId) {
	logWrite("Loading tiles for base %d", eBaseId);
	const tBitMap *pBase = s_pBaseTiles[eBaseId];
	tBitMap *pTiles = s_pManager->pTileSet;
	memcpy(
		pTiles->Planes[0], pBase->Planes[0],
		pBase->Rows * pBase->BytesPerRow
	);
	s_eBaseCurrent = eBaseId;
}

//------------------------------------------------------------------- PUBLIC FNS

void baseCreate(tTileBufferManager *pManager) {
	s_pManager = pManager;
	s_pBaseTiles[BASE_ID_GROUND] = bitmapCreateFromFile("data/base0.bm", 1);
	s_pBaseTiles[BASE_ID_DINO] = bitmapCreateFromFile("data/base1.bm", 1);
	s_pBaseTiles[BASE_ID_GATE] = bitmapCreateFromFile("data/base2.bm", 1);
	s_pCheckpointTiles = bitmapCreateFromFile("data/checkpoint.bm", 1);
	baseTileLoad(BASE_ID_GROUND);
	s_isFinishLineLoaded = 0;
}

void baseDestroy(void) {
	bitmapDestroy(s_pBaseTiles[0]);
	bitmapDestroy(s_pBaseTiles[1]);
	bitmapDestroy(s_pBaseTiles[2]);
	bitmapDestroy(s_pCheckpointTiles);
}

#define TILE_BYTE_COUNT (TILE_SIZE * (TILE_SIZE / 8) * 5)

void baseProcess(void) {
	UWORD uwCamY = s_pManager->pCamera->uPos.uwY;
	if(g_isChallenge) {
		tBitMap *pTiles = s_pManager->pTileSet;
		if(uwCamY >= TILE_ROW_CHALLENGE_CHECKPOINT_3 * TILE_SIZE) {
			if(!s_isFinishLineLoaded) {
				memcpy(
					&pTiles->Planes[0][TILE_CHECKPOINT_1 * TILE_BYTE_COUNT],
					&s_pCheckpointTiles->Planes[0][10 * TILE_BYTE_COUNT],
					10 * TILE_BYTE_COUNT
				);
				s_isFinishLineLoaded = 1;
			}
		}
		else if(s_isFinishLineLoaded) {
			memcpy(
				&pTiles->Planes[0][TILE_CHECKPOINT_1 * TILE_BYTE_COUNT],
				s_pCheckpointTiles->Planes[0],
				10 * TILE_BYTE_COUNT
			);
			s_isFinishLineLoaded = 0;
		}
	}
	else {
		for(tBaseId eBaseId = 0; eBaseId < BASE_ID_COUNT_UNIQUE; ++eBaseId) {
			UWORD uwLoadingBottom = (
				s_pBases[eBaseId].uwTileDepth + BASE_TILE_LOADING_MARGIN
			) * TILE_SIZE;
			if(uwCamY < uwLoadingBottom) {
				if(s_eBaseCurrent != eBaseId) {
					baseTileLoad(eBaseId);
				}
				break;
			}
		}
	}
}

const tBase *baseGetById(tBaseId eId) {
	return &s_pBases[eId];
}

const tBase *baseGetCurrent(void) {
	return &s_pBases[s_eBaseCurrent];
}