/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/rand.h>
#include <comm/base.h>
#include "game.h"
#include "core.h"
#include "hud.h"
#include "mineral.h"
#include "defs.h"
#include "save.h"

typedef struct _tBase {
	UBYTE pPattern[10 * 10];
	UWORD uwLevel;
} tBase;

//----------------------------------------------------------------- PRIVATE VARS

static const tBase s_pBases[] = {
	{
		.uwLevel = 0,
		.pPattern = {
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
		}
	},
	{
		.uwLevel = 100,
		.pPattern = {
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
		}
	}
};

static const UBYTE s_ubBaseCount = sizeof(s_pBases) / sizeof(s_pBases[0]);

//------------------------------------------------------------------ PUBLIC VARS

const tTileDef g_pTileDefs[TILE_COUNT] = {
	{.ubSlots = 0, .ubMineral = MINERAL_TYPE_COUNT},
	[TILE_SILVER_1] = {.ubSlots = 1, .ubMineral = MINERAL_TYPE_SILVER},
	[TILE_SILVER_2] = {.ubSlots = 2, .ubMineral = MINERAL_TYPE_SILVER},
	[TILE_SILVER_3] = {.ubSlots = 3, .ubMineral = MINERAL_TYPE_SILVER},
	[TILE_GOLD_1] = {.ubSlots = 1, .ubMineral = MINERAL_TYPE_GOLD},
	[TILE_GOLD_2] = {.ubSlots = 2, .ubMineral = MINERAL_TYPE_GOLD},
	[TILE_GOLD_3] = {.ubSlots = 3, .ubMineral = MINERAL_TYPE_GOLD},
	[TILE_EMERALD_1] = {.ubSlots = 1, .ubMineral = MINERAL_TYPE_EMERALD},
	[TILE_EMERALD_2] = {.ubSlots = 2, .ubMineral = MINERAL_TYPE_EMERALD},
	[TILE_EMERALD_3] = {.ubSlots = 3, .ubMineral = MINERAL_TYPE_EMERALD},
	[TILE_RUBY_1] = {.ubSlots = 1, .ubMineral = MINERAL_TYPE_RUBY},
	[TILE_RUBY_2] = {.ubSlots = 2, .ubMineral = MINERAL_TYPE_RUBY},
	[TILE_RUBY_3] = {.ubSlots = 3, .ubMineral = MINERAL_TYPE_RUBY},
	[TILE_MOONSTONE_1] = {.ubSlots = 1, .ubMineral = MINERAL_TYPE_MOONSTONE},
	[TILE_MOONSTONE_2] = {.ubSlots = 2, .ubMineral = MINERAL_TYPE_MOONSTONE},
	[TILE_MOONSTONE_3] = {.ubSlots = 3, .ubMineral = MINERAL_TYPE_MOONSTONE},
	[TILE_COAL_1] = {.ubSlots = 1, .ubMineral = MINERAL_TYPE_COAL},
	[TILE_COAL_2] = {.ubSlots = 2, .ubMineral = MINERAL_TYPE_COAL},
	[TILE_COAL_3] = {.ubSlots = 3, .ubMineral = MINERAL_TYPE_COAL}
};

//------------------------------------------------------------------ PRIVATE FNS

static UWORD chanceTrapezoid(
	UWORD uwCurr, UWORD uwStart, UWORD uwPeakStart, UWORD uwPeakEnd, UWORD uwEnd,
	UWORD uwMin, UWORD uwMax
) {
	if(uwStart < uwCurr && uwCurr < uwPeakStart) {
		// Ascending ramp
		return uwMin + ((uwMax - uwMin) * (uwCurr - uwStart)) / (uwPeakStart - uwStart);
	}
	if(uwPeakStart <= uwCurr && uwCurr <= uwPeakEnd) {
		// Peak
		return uwMax;
	}
	if(uwPeakEnd < uwCurr && uwCurr < uwEnd) {
		// Descending ramp
		return uwMax - ((uwMax - uwMin) * (uwCurr - uwPeakEnd)) / (uwEnd - uwPeakEnd);
	}
	// Out of range
	return uwMin;
}

UBYTE tileIsSolid(UWORD uwX, UWORD uwY) {
	UBYTE ubTile = g_pMainBuffer->pTileData[uwX][uwY];
	return (
		(TILE_BASE_GROUND_1 <= ubTile && ubTile <= TILE_BASE_GROUND_9) ||
		ubTile >= TILE_DIRT_1
	);
}

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_DIRT_1;
}

UBYTE tileIsHardToDrill(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_STONE_1;
}

void tileReset(UBYTE isCoalOnly, UBYTE isChallenge) {
	logBlockBegin(
		"tileReset(isCoalOnly: %hhu, isChallenge: %hhu)", isCoalOnly, isChallenge
	);
	UWORD uwEndX = g_pMainBuffer->uTileBounds.uwX;
	UWORD uwEndY = g_pMainBuffer->uTileBounds.uwY;
	if(isChallenge) {
		uwEndY = TILE_ROW_CHALLENGE_FINISH + 20; // generate a bit more to accomodate scroll
	}

	UBYTE **pTiles = g_pMainBuffer->pTileData;

	// Draw terrain
	UBYTE ubPercentTiles = (100 - 10 * s_ubBaseCount);
	for(UWORD x = 1; x < uwEndX; ++x) {
		UBYTE ubPercent = (x * ubPercentTiles) / uwEndX;
		commProgress(ubPercent, g_pMsgs[MSG_LOADING_GEN_TERRAIN]);
		for(UWORD y = TILE_ROW_BASE_DIRT + 2; y < uwEndY; ++y) {
			// 2000 is max
			UWORD uwWhat = (randUw(&g_sRand) * 1000) / 65535;
			UWORD uwChanceAir = 50;
			UWORD uwChanceRock, uwChanceSilver, uwChanceGold, uwChanceEmerald, uwChanceRuby, uwChanceMoonstone;
			if(g_isChallenge) {
				uwChanceRock = 75;
				uwChanceSilver = chanceTrapezoid(
					y, TILE_ROW_BASE_DIRT, TILE_ROW_BASE_DIRT+5,
					TILE_ROW_CHALLENGE_CHECKPOINT_2, TILE_ROW_CHALLENGE_CHECKPOINT_2 + 5,
					5, 200
				);
				uwChanceGold = chanceTrapezoid(
					y, TILE_ROW_CHALLENGE_CHECKPOINT_2 - 5, TILE_ROW_CHALLENGE_CHECKPOINT_2 + 5,
					TILE_ROW_CHALLENGE_FINISH - 5, TILE_ROW_CHALLENGE_FINISH + 5,
					2, 200
				);
				uwChanceEmerald = 0;
				uwChanceRuby = 0;
				uwChanceMoonstone = 10;
			}
			else {
				uwChanceRock = CLAMP(y * 500 / 2000, 0, 500);
				uwChanceSilver = chanceTrapezoid(y, 10, 30, 50, 100, 5, 200);
				uwChanceGold = chanceTrapezoid(y, 60, 120, 150, 250, 2, 200);
				uwChanceEmerald = chanceTrapezoid(y, 175, 400, 450, 600, 1, 200);
				uwChanceRuby = chanceTrapezoid(y, 500, 650, 700, 850, 1, 200);
				uwChanceMoonstone = chanceTrapezoid(y, 175, 1000, 1500, 2000, 1, 10);
			}
			UWORD uwChance;
			if(uwWhat < (uwChance = uwChanceRock)) {
				pTiles[x][y] = randUwMinMax(&g_sRand, TILE_STONE_1, TILE_STONE_4);
			}
			else if(
				uwWhat < (uwChance += uwChanceAir) &&
				tileIsSolid(x - 1, y) && tileIsSolid(x, y - 1)
			) {
				pTiles[x][y] = TILE_CAVE_BG_1+15;
			}
			else if(uwWhat < (uwChance += uwChanceSilver)) {
				pTiles[x][y] = (
					isCoalOnly
						? randUwMinMax(&g_sRand, TILE_COAL_1, TILE_COAL_2)
						: randUwMinMax(&g_sRand, TILE_SILVER_1, TILE_SILVER_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceGold)) {
				pTiles[x][y] = (
					isCoalOnly
						? randUwMinMax(&g_sRand, TILE_COAL_1, TILE_COAL_2)
						: randUwMinMax(&g_sRand, TILE_GOLD_1, TILE_GOLD_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceEmerald)) {
				pTiles[x][y] = (
					isCoalOnly
						? randUwMinMax(&g_sRand, TILE_COAL_1, TILE_COAL_2)
						: randUwMinMax(&g_sRand, TILE_EMERALD_1, TILE_EMERALD_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceRuby)) {
				pTiles[x][y] = (
					isCoalOnly
						? randUwMinMax(&g_sRand, TILE_COAL_1, TILE_COAL_2)
						: randUwMinMax(&g_sRand, TILE_RUBY_1, TILE_RUBY_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceMoonstone)) {
				pTiles[x][y] = (
					isCoalOnly
						? randUwMinMax(&g_sRand, TILE_COAL_1, TILE_COAL_2)
						: randUwMinMax(&g_sRand, TILE_MOONSTONE_1, TILE_MOONSTONE_3)
				);
			}
			else {
				pTiles[x][y] = TILE_DIRT_1 + ((x & 1) ^ (y & 1));
			}
			// For quick tests
			// g_pMainBuffer->pTileData[2][y] = TILE_CAVE_BG_1 + 12;
		}
	}

	// Draw bases
	for(UBYTE ubBase = 0; ubBase < s_ubBaseCount; ++ubBase) {
		UBYTE ubPercent = ((100 - ubPercentTiles) * ubBase / s_ubBaseCount);
		commProgress(ubPercentTiles + ubPercent, g_pMsgs[MSG_LOADING_GEN_BASES]);
		const tBase *pBase = &s_pBases[ubBase];
		for(UWORD y = 0; y <= TILE_ROW_BASE_DIRT+1; ++y) {
			for(UWORD x = 1; x < 1 + 10; ++x) {
				pTiles[x][pBase->uwLevel + y] = pBase->pPattern[y * 10 + x - 1];
			}
		}
	}

	// Fill left invisible col with rocks
	commProgress(100, g_pMsgs[MSG_LOADING_FINISHING]);
	for(UWORD y = 0; y < uwEndY; ++y) {
		pTiles[0][y] = TILE_DIRT_1;
	}

	if(isChallenge) {
		for(UWORD x = 1; x < uwEndX; ++x) {
			pTiles[x][TILE_ROW_CHALLENGE_CHECKPOINT_1] = TILE_CHECKPOINT_1 + x - 1;
			pTiles[x][TILE_ROW_CHALLENGE_CHECKPOINT_2] = TILE_CHECKPOINT_1 + x - 1;
			pTiles[x][TILE_ROW_CHALLENGE_CHECKPOINT_3] = TILE_CHECKPOINT_1 + x - 1;
			pTiles[x][TILE_ROW_CHALLENGE_FINISH] = TILE_CHECKPOINT_1 + x - 1;
			pTiles[x][TILE_ROW_CHALLENGE_FINISH+1] = TILE_STONE_1 + (x & 1);
		}
	}
	else {
		// Dino bones
		pTiles[5][g_pDinoDepths[0]] = TILE_BONE_HEAD;
		pTiles[3][g_pDinoDepths[1]] = TILE_BONE_1;
		pTiles[7][g_pDinoDepths[2]] = TILE_BONE_1;
		pTiles[1][g_pDinoDepths[3]] = TILE_BONE_1;
		pTiles[4][g_pDinoDepths[4]] = TILE_BONE_1;
		pTiles[6][g_pDinoDepths[5]] = TILE_BONE_1;
		pTiles[8][g_pDinoDepths[6]] = TILE_BONE_1;
		pTiles[2][g_pDinoDepths[7]] = TILE_BONE_1;
		pTiles[9][g_pDinoDepths[8]] = TILE_BONE_1;
	}

	logBlockEnd("tileReset()");
}

void tileSave(tFile *pFile) {
	saveWriteHeader(pFile, "TILE");
	UBYTE **pTiles = g_pMainBuffer->pTileData;
	UWORD uwSizeX = g_pMainBuffer->uTileBounds.uwX;
	UWORD uwSizeY = g_pMainBuffer->uTileBounds.uwY;

	fileWrite(pFile, &uwSizeX, sizeof(uwSizeX));
	fileWrite(pFile, &uwSizeY, sizeof(uwSizeY));
	for(UWORD uwX = 0; uwX < uwSizeX; ++uwX) {
		fileWrite(pFile, &pTiles[uwX][0], sizeof(pTiles[0][0]) * uwSizeY);
	}
}

UBYTE tileLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "TILE")) {
		return 0;
	}

	UBYTE **pTiles = g_pMainBuffer->pTileData;
	UWORD uwSizeX, uwSizeY;

	fileRead(pFile, &uwSizeX, sizeof(uwSizeX));
	fileRead(pFile, &uwSizeY, sizeof(uwSizeY));
	for(UWORD uwX = 0; uwX < uwSizeX; ++uwX) {
		fileRead(pFile, &pTiles[uwX][0], sizeof(pTiles[0][0]) * uwSizeY);
	}
	return 1;
}

void tileExcavate(UWORD uwX, UWORD uwY) {
	UBYTE ubBg = TILE_CAVE_BG_1;

	// up
	if(tileIsSolid(uwX, uwY-1)) {
		ubBg += 1;
	}
	else {
		g_pMainBuffer->pTileData[uwX][uwY-1] -= 2;
		tileBufferInvalidateTile(g_pMainBuffer, uwX, uwY-1);
	}

	// down
	if(tileIsSolid(uwX, uwY+1)) {
		ubBg += 2;
	}
	else {
		g_pMainBuffer->pTileData[uwX][uwY+1] -= 1;
		tileBufferInvalidateTile(g_pMainBuffer, uwX, uwY+1);
	}

	// right
	if(uwX >= 10 || tileIsSolid(uwX+1, uwY)) {
		ubBg += 4;
	}
	else if(uwX < 10) {
		g_pMainBuffer->pTileData[uwX+1][uwY] -= 8;
		tileBufferInvalidateTile(g_pMainBuffer, uwX+1, uwY);
	}

	// left
	if(tileIsSolid(uwX-1, uwY)) {
		ubBg += 8;
	}
	else {
		g_pMainBuffer->pTileData[uwX-1][uwY] -= 4;
		tileBufferInvalidateTile(g_pMainBuffer, uwX-1, uwY);
	}

	tileBufferSetTile(g_pMainBuffer, uwX, uwY, ubBg);
}
