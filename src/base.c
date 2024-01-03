/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base.h"
#include "game.h"
#include "tile.h"
#include "hud.h"
#include "tile_variant.h"

#define BASE_TILE_DEPTH_GROUND 0
#define BASE_TILE_DEPTH_DINO 100
#define BASE_TILE_DEPTH_GATE 209
#define BASE_TILE_LOADING_MARGIN 10

//----------------------------------------------------------------- PRIVATE VARS

static tBaseId s_eBaseCurrent;
static tBitMap *s_pBaseTiles[BASE_ID_COUNT_UNIQUE];
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
			54, 48, 54, 55, 54, 55, 54, 55, 54, 55,
		},
		.sRectRestock = {
			.uwX1 = 7 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_GROUND + 7) * TILE_SIZE,
			.uwX2 = 9 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_GROUND + 8) * TILE_SIZE
		},
		.cbProcess = 0,
	},
	[BASE_ID_DINO] = {
		.uwTileDepth = 100,
		.pTilePattern = {
			43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
			44, 43, 43, 43, 43, 45, 43, 44, 43, 43,
			43, 45, 43, 43, 43, 43, 43, 43, 43, 49,
			43, 43, 43, 43, 43, 48, 45, 43, 43, 45,
			43, 43, 43, 43, 48, 43, 43, 43, 44, 43,
			 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			20, 21, 22, 23, 24, 10, 11, 12, 13, 14,
			25, 26, 27, 28, 29, 15, 16, 17, 18, 19,
			34, 35, 36, 37, 38, 33, 39, 40, 41, 42,
			54, 55, 54, 55, 54, 48, 54, 55, 54, 55,
		},
		.sRectRestock = {
			.uwX1 = 1 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_DINO + 7) * TILE_SIZE,
			.uwX2 = 3 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_DINO + 8) * TILE_SIZE
		},
		.cbProcess = 0,
	},
	[BASE_ID_GATE] = {
		.uwTileDepth = 209,
		.pTilePattern = {
			43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
			44, 43, 43, 43, 43, 45, 43, 44, 43, 43,
			43, 45, 43, 43, 43, 43, 43, 43, 43, 49,
			43, 43, 43, 43, 43, 48, 45, 43, 43, 45,
			 0,  1, 43, 43,  2,  1, 43, 43, 44, 43,
			 3,  4, 45, 43,  5,  6, 49,  7, 43, 43,
			12, 13, 43, 43, 14, 15, 16, 17, 18, 19,
			20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			34, 35, 36, 37, 38, 33, 39, 40, 41, 42,
			54, 55, 54, 55, 54, 48, 54, 55, 54, 55,
		},
		.sRectRestock = {
			.uwX1 = 8 * TILE_SIZE,
			.uwY1 = (BASE_TILE_DEPTH_GATE + 7) * TILE_SIZE,
			.uwX2 = 10 * TILE_SIZE,
			.uwY2 = (BASE_TILE_DEPTH_GATE + 8) * TILE_SIZE
		},
		.cbProcess = gameProcessBaseGate,
	},
	[BASE_ID_DINO_POPULATED] = {
		.uwTileDepth = BASE_TILE_DEPTH_VARIANT,
		.pTilePattern = {
			43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
			44, 43, 43, 43, 43, 45, 43, 44, 43, 43,
			43, 45, 43, 43, 43, 43, 43, 43, 43, 49,
			43, 43, 43, 43, 43, 48, 45, 43, 43, 45,
			 4,  5, 43, 43,  6,  5, 43, 43, 44, 43,
			 7,  8, 45, 43,  9, 10, 49, 11, 43, 43,
			12, 13, 43, 43, 14, 15, 16, 17, 18, 19,
			20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			34, 35, 36, 37, 38, 33, 39, 40, 41, 42,
			54, 55, 54, 55, 54, 48, 54, 55, 54, 55,
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
	baseTileLoad(BASE_ID_GROUND);
}

void baseDestroy(void) {
	bitmapDestroy(s_pBaseTiles[0]);
	bitmapDestroy(s_pBaseTiles[1]);
	bitmapDestroy(s_pBaseTiles[2]);
}

void baseProcess(void) {
	UWORD uwCamY = s_pManager->pCamera->uPos.uwY;
	if(g_isChallenge) {
		if(uwCamY >= TILE_ROW_CHALLENGE_CHECKPOINT_3 * TILE_SIZE) {
			tileVariantChangeTo(TILE_VARIANT_FINISH);
		}
		else {
			tileVariantChangeTo(TILE_VARIANT_CHECKPOINT);
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

	const tBase *pBase = baseGetCurrent();
	if(pBase->cbProcess) {
		pBase->cbProcess();
	}
}

const tBase *baseGetById(tBaseId eId) {
	return &s_pBases[eId];
}

const tBase *baseGetCurrent(void) {
	return &s_pBases[s_eBaseCurrent];
}

void baseUpdateDinoTileset(UBYTE isPopulated) {
	if(isPopulated) {
		bitmapLoadFromFile(s_pBaseTiles[BASE_ID_DINO], "data/base1_populated.bm", 0, 20 * TILE_SIZE);
	}
	else {
		// Sub-optimal speed, but whatever
		bitmapLoadFromFile(s_pBaseTiles[BASE_ID_DINO], "data/base1.bm", 0, 0);
	}
}
