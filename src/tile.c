/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/rand.h>
#include "game.h"
#include "core.h"
#include "hud.h"
#include "mineral.h"
#include "comm.h"
#include "defs.h"

typedef enum _tMsgLoading {
	MSG_LOADING_GEN_TERRAIN,
	MSG_LOADING_GEN_BASES,
	MSG_LOADING_FINISHING,
} tMsgLoading;

tStringArray g_sLoadMsgs;

const tTileDef const g_pTileDefs[TILE_COUNT] = {
	{.szMsg = 0, .ubSlots = 0, .ubMineral = MINERAL_TYPE_COUNT},
	[TILE_SILVER_1] = {.szMsg = "Silver x1", .ubSlots = 1, .ubMineral = MINERAL_TYPE_SILVER},
	[TILE_SILVER_2] = {.szMsg = "Silver x2", .ubSlots = 2, .ubMineral = MINERAL_TYPE_SILVER},
	[TILE_SILVER_3] = {.szMsg = "Silver x3", .ubSlots = 3, .ubMineral = MINERAL_TYPE_SILVER},
	[TILE_GOLD_1] = {.szMsg = "Gold x1", .ubSlots = 1, .ubMineral = MINERAL_TYPE_GOLD},
	[TILE_GOLD_2] = {.szMsg = "Gold x2", .ubSlots = 2, .ubMineral = MINERAL_TYPE_GOLD},
	[TILE_GOLD_3] = {.szMsg = "Gold x3", .ubSlots = 3, .ubMineral = MINERAL_TYPE_GOLD},
	[TILE_EMERALD_1] = {.szMsg = "Emerald x1", .ubSlots = 1, .ubMineral = MINERAL_TYPE_EMERALD},
	[TILE_EMERALD_2] = {.szMsg = "Emerald x2", .ubSlots = 2, .ubMineral = MINERAL_TYPE_EMERALD},
	[TILE_EMERALD_3] = {.szMsg = "Emerald x3", .ubSlots = 3, .ubMineral = MINERAL_TYPE_EMERALD},
	[TILE_RUBY_1] = {.szMsg = "Ruby x1", .ubSlots = 1, .ubMineral = MINERAL_TYPE_RUBY},
	[TILE_RUBY_2] = {.szMsg = "Ruby x2", .ubSlots = 2, .ubMineral = MINERAL_TYPE_RUBY},
	[TILE_RUBY_3] = {.szMsg = "Ruby x3", .ubSlots = 3, .ubMineral = MINERAL_TYPE_RUBY},
	[TILE_MOONSTONE_1] = {.szMsg = "Moonstone x1", .ubSlots = 1, .ubMineral = MINERAL_TYPE_MOONSTONE},
	[TILE_MOONSTONE_2] = {.szMsg = "Moonstone x2", .ubSlots = 2, .ubMineral = MINERAL_TYPE_MOONSTONE},
	[TILE_MOONSTONE_3] = {.szMsg = "Moonstone x3", .ubSlots = 3, .ubMineral = MINERAL_TYPE_MOONSTONE},
	[TILE_COAL_1] = {.szMsg = "Coal x1", .ubSlots = 1, .ubMineral = MINERAL_TYPE_COAL},
	[TILE_COAL_2] = {.szMsg = "Coal x2", .ubSlots = 2, .ubMineral = MINERAL_TYPE_COAL},
	[TILE_COAL_3] = {.szMsg = "Coal x3", .ubSlots = 3, .ubMineral = MINERAL_TYPE_COAL}
};

UBYTE tileIsSolid(UWORD uwX, UWORD uwY) {
	UBYTE ubTile = g_pMainBuffer->pTileData[uwX][uwY];
	return (
		(TILE_BASE_GROUND_1 <= ubTile && ubTile <= TILE_BASE_GROUND_9) ||
		ubTile >= TILE_STONE_1
	);
}

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_ROCK_1;
}

UBYTE tileIsHardToDrill(UWORD uwX, UWORD uwY) {
	return (
		g_pMainBuffer->pTileData[uwX][uwY] >= TILE_EMERALD_1 &&
		TILE_MOONSTONE_3 <= g_pMainBuffer->pTileData[uwX][uwY]
	);
}

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

typedef struct _tBase {
	UBYTE pPattern[10 * 10];
	UWORD uwLevel;
} tBase;

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

const uint8_t s_ubBaseCount = sizeof(s_pBases) / sizeof(s_pBases[0]);

void tileInit(UBYTE isCoalOnly, UBYTE isChallenge) {
	logBlockBegin(
		"tileInit(isCoalOnly: %hhu, isChallenge: %hhu)", isCoalOnly, isChallenge
	);
	UWORD uwEndX = g_pMainBuffer->uTileBounds.uwX;
	UWORD uwEndY = g_pMainBuffer->uTileBounds.uwY;
	if(isChallenge) {
		uwEndY = TILE_ROW_CHALLENGE_FINISH + 1; // without +1 it's broken
	}

	UBYTE **pTiles = g_pMainBuffer->pTileData;

	// Draw terrain
	UBYTE ubPercentTiles = (100 - 10 * s_ubBaseCount);
	for(UWORD x = 1; x < uwEndX; ++x) {
		UBYTE ubPercent = (x * ubPercentTiles) / uwEndX;
		commProgress(ubPercent, g_sLoadMsgs.pStrings[MSG_LOADING_GEN_TERRAIN]);
		for(UWORD y = TILE_ROW_BASE_DIRT + 2; y < uwEndY; ++y) {
			// 2000 is max
			UWORD uwWhat = (uwRand() * 1000) / 65535;
			UWORD uwChanceAir = 50;
			UWORD uwChanceRock, uwChanceSilver, uwChanceGold, uwChanceEmerald, uwChanceRuby, uwChanceMoonstone;
			if(g_isChallenge) {
				uwChanceRock = 75;
				uwChanceSilver = chanceTrapezoid(
					y, TILE_ROW_BASE_DIRT, TILE_ROW_BASE_DIRT+5,
					TILE_ROW_CHALLENGE_CHECKPOINT_1, TILE_ROW_CHALLENGE_CHECKPOINT_1 + 5,
					5, 200
				);
				uwChanceGold = chanceTrapezoid(
					y, TILE_ROW_CHALLENGE_CHECKPOINT_1 - 5, TILE_ROW_CHALLENGE_CHECKPOINT_1 + 5,
					TILE_ROW_CHALLENGE_CHECKPOINT_2 - 5, TILE_ROW_CHALLENGE_CHECKPOINT_2 + 5,
					2, 200
				);
				uwChanceEmerald = chanceTrapezoid(
					y, TILE_ROW_CHALLENGE_CHECKPOINT_2 - 5, TILE_ROW_CHALLENGE_CHECKPOINT_2 + 5,
					TILE_ROW_CHALLENGE_CHECKPOINT_3 - 5, TILE_ROW_CHALLENGE_CHECKPOINT_3 + 5,
					1, 200
				);
				uwChanceRuby = chanceTrapezoid(
					y, TILE_ROW_CHALLENGE_CHECKPOINT_3 - 5, TILE_ROW_CHALLENGE_CHECKPOINT_3 + 5,
					TILE_ROW_CHALLENGE_FINISH - 5, TILE_ROW_CHALLENGE_FINISH + 5,
					1, 200
				);
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
				pTiles[x][y] = ubRandMinMax(TILE_STONE_1, TILE_STONE_4);
			}
			else if(
				uwWhat < (uwChance += uwChanceAir) &&
				tileIsSolid(x - 1, y) && tileIsSolid(x, y - 1)
			) {
				pTiles[x][y] = TILE_CAVE_BG+15;
			}
			else if(uwWhat < (uwChance += uwChanceSilver)) {
				pTiles[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_SILVER_1, TILE_SILVER_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceGold)) {
				pTiles[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_GOLD_1, TILE_GOLD_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceEmerald)) {
				pTiles[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_EMERALD_1, TILE_EMERALD_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceRuby)) {
				pTiles[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_RUBY_1, TILE_RUBY_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceMoonstone)) {
				pTiles[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_MOONSTONE_1, TILE_MOONSTONE_3)
				);
			}
			else {
				pTiles[x][y] = TILE_ROCK_1 + ((x & 1) ^ (y & 1));
			}
			// For quick tests
			// g_pMainBuffer->pTileData[2][y] = TILE_CAVE_BG + 12;
		}
	}

	// Draw bases
	for(UBYTE ubBase = 0; ubBase < s_ubBaseCount; ++ubBase) {
		UBYTE ubPercent = ((100 - ubPercentTiles) * ubBase / s_ubBaseCount);
		commProgress(ubPercentTiles + ubPercent, g_sLoadMsgs.pStrings[MSG_LOADING_GEN_BASES]);
		const tBase *pBase = &s_pBases[ubBase];
		for(UWORD y = 0; y <= TILE_ROW_BASE_DIRT+1; ++y) {
			for(UWORD x = 1; x < 1 + 10; ++x) {
				pTiles[x][pBase->uwLevel + y] = pBase->pPattern[y * 10 + x - 1];
			}
		}
	}

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

	// Fill left invisible col with rocks
	commProgress(100, g_sLoadMsgs.pStrings[MSG_LOADING_FINISHING]);
	for(UWORD y = 0; y < uwEndY; ++y) {
		pTiles[0][y] = TILE_ROCK_1;
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
	logBlockEnd("tileInit()");
}

void tileExcavate(UWORD uwX, UWORD uwY) {
	UBYTE ubBg = TILE_CAVE_BG;

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
