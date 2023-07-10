/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/rand.h>
#include <comm/comm.h>
#include "game.h"
#include "core.h"
#include "hud.h"
#include "mineral.h"
#include "defs.h"
#include "save.h"

//----------------------------------------------------------------- PRIVATE VARS

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

static void tileSetBaseTiles(const tBase *pBase, UWORD uwLevel, UBYTE isReplacing) {
	UBYTE **pTiles = g_pMainBuffer->pTileData;
	for(UWORD y = 0; y <= TILE_ROW_BASE_DIRT+1; ++y) {
		for(UWORD x = 1; x < 1 + 10; ++x) {
			if(!isReplacing || tileIsSolid(x, uwLevel + y)) {
				pTiles[x][uwLevel + y] = pBase->pTilePattern[y * 10 + x - 1];
			}
		}
	}
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
	UBYTE ubPercentTiles = (100 - 10 * BASE_ID_COUNT_UNIQUE);
	for(UWORD x = 1; x < uwEndX; ++x) {
		UBYTE ubPercent = (x * ubPercentTiles) / uwEndX;
		commProgress(ubPercent, g_pMsgs[MSG_LOADING_GEN_TERRAIN]);
		for(UWORD y = TILE_ROW_BASE_DIRT + 2; y < uwEndY; ++y) {
			// 2000 is max
			UWORD uwWhat = (randUw(&g_sRand) * 1000) / 65535;
			UWORD uwChanceAir = 50;
			UWORD uwChanceRock, uwChanceSilver, uwChanceGold, uwChanceEmerald, uwChanceRuby, uwChanceMoonstone, uwChanceMagma;
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
				uwChanceMagma = 10;
			}
			else {
				uwChanceRock = CLAMP(y * 500 / 2000, 0, 500);
				uwChanceSilver = 75;
				uwChanceGold = y > 60 ? 75 : 0;
				uwChanceEmerald = y > 200 ? 75 : 0;
				uwChanceRuby = y  > 400 ? 75 : 0;
				uwChanceMoonstone = y > 175 ? 75 : 0;
				uwChanceMagma = chanceTrapezoid(y, 50, 900, 1000, 1100, 0, 75);
			}
			UWORD uwChance;
			if(uwWhat < (uwChance = uwChanceRock)) {
				pTiles[x][y] = randUwMinMax(&g_sRand, TILE_STONE_1, TILE_STONE_4);
			}
			else if(uwWhat < (uwChance += uwChanceMagma)) {
				pTiles[x][y] = randUwMinMax(&g_sRand, TILE_MAGMA_1, TILE_MAGMA_2);
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
	for(tBaseId eBaseId = 0; eBaseId < BASE_ID_COUNT; ++eBaseId) {
		const tBase *pBase = baseGetById(eBaseId);
		if(pBase->uwTileDepth != BASE_TILE_DEPTH_VARIANT) {
			UBYTE ubPercent = ((100 - ubPercentTiles) * eBaseId / BASE_ID_COUNT_UNIQUE);
			commProgress(ubPercentTiles + ubPercent, g_pMsgs[MSG_LOADING_GEN_BASES]);
			tileSetBaseTiles(pBase, pBase->uwTileDepth, 0);
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
		// Rock bottom
		for(UWORD x = 1; x < uwEndX; ++x) {
			pTiles[x][uwEndY - 1] = TILE_STONE_1 + (x & 3);
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

		// Gate fragments
		pTiles[1][219] = TILE_GATE_1;
		pTiles[2][219] = TILE_GATE_2;
		pTiles[3][219] = TILE_GATE_1;
		pTiles[4][219] = TILE_GATE_2;
		pTiles[5][219] = TILE_GATE_1;
		pTiles[6][219] = TILE_GATE_2;
		pTiles[7][219] = TILE_GATE_1;
		pTiles[8][219] = TILE_GATE_2;
		pTiles[9][219] = TILE_GATE_1;
		pTiles[1][220] = TILE_GATE_2;
		pTiles[2][220] = TILE_GATE_1;
		pTiles[3][220] = TILE_GATE_2;
		pTiles[4][220] = TILE_GATE_1;
		pTiles[5][220] = TILE_GATE_2;
		pTiles[6][220] = TILE_GATE_1;
		pTiles[7][220] = TILE_GATE_2;
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

void tileReplaceBaseWithVariant(tBaseId eBase, tBaseId eNewVariant) {
	const tBase *pBase = baseGetById(eBase);
	const tBase *pBaseVariant = baseGetById(eNewVariant);
	if(pBaseVariant->uwTileDepth != BASE_TILE_DEPTH_VARIANT) {
		logWrite("ERR: eNewVariant %d is not a base variant\n", eNewVariant);
	}
	if(pBase->uwTileDepth == BASE_TILE_DEPTH_VARIANT) {
		logWrite("ERR: eBase %d is not a base non-variant\n", eNewVariant);
	}

	tileSetBaseTiles(pBaseVariant, pBase->uwTileDepth, 1);
}
