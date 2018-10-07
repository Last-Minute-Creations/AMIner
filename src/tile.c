/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/rand.h>
#include "game.h"
#include "hud.h"

typedef enum _tTile {
	TILE_NONE = 0,
	TILE_SHOP_CHIMNEY,
	TILE_SHOP_A1, TILE_SHOP_A2, TILE_SHOP_A3,
	TILE_SHOP_B1, TILE_SHOP_B2, TILE_SHOP_B3,
	TILE_SHOP_C1, TILE_SHOP_C2, TILE_SHOP_C3,
	TILE_SHOP_D1, TILE_SHOP_D2, TILE_SHOP_D3,
	TILE_SHOP_E1, TILE_SHOP_E2, TILE_SHOP_E3,
	TILE_GRASS_1 = 17,
	TILE_GRASS_2,
	TILE_GRASS_NONE,
	TILE_GRASS_LEFT,
	TILE_GRASS_RIGHT,
	TILE_GRASS_BOTH,
	TILE_CAVE_BG,
	TILE_STONE_1 = TILE_CAVE_BG + 16,
	TILE_STONE_2,
	TILE_ROCK_1,
	TILE_ROCK_2,
	TILE_GOLD_1,
	TILE_GOLD_2,
	TILE_GOLD_3,
	TILE_SILVER_1,
	TILE_SILVER_2,
	TILE_SILVER_3,
	TILE_URANIUM_1,
	TILE_URANIUM_2,
	TILE_URANIUM_3,
	TILE_COAL_1,
	TILE_COAL_2,
	TILE_COAL_3
} tTile;
// TODO sapphire, emerald, topaz

typedef struct _tTileDef {
	const char *szMsg;
	UBYTE ubReward;
	UBYTE ubSlots;
	UBYTE ubColor;
} tTileDef;

static const tTileDef const s_pTileDefs[] = {
	{.szMsg = 0, .ubReward = 0, .ubSlots = 0},
	[TILE_SILVER_1] = {.szMsg = "Silver x1", .ubReward = 5, .ubSlots = 1, .ubColor = 15},
	[TILE_SILVER_2] = {.szMsg = "Silver x2", .ubReward = 5, .ubSlots = 2, .ubColor = 15},
	[TILE_SILVER_3] = {.szMsg = "Silver x3", .ubReward = 5, .ubSlots = 3, .ubColor = 15},
	[TILE_GOLD_1] = {.szMsg = "Gold x1", .ubReward = 10, .ubSlots = 1, .ubColor = 14},
	[TILE_GOLD_2] = {.szMsg = "Gold x2", .ubReward = 10, .ubSlots = 2, .ubColor = 14},
	[TILE_GOLD_3] = {.szMsg = "Gold x3", .ubReward = 10, .ubSlots = 3, .ubColor = 14},
	[TILE_URANIUM_1] = {.szMsg = "Uranium x1", .ubReward = 20, .ubSlots = 1, .ubColor = 12},
	[TILE_URANIUM_2] = {.szMsg = "Uranium x2", .ubReward = 20, .ubSlots = 2, .ubColor = 12},
	[TILE_URANIUM_3] = {.szMsg = "Uranium x3", .ubReward = 20, .ubSlots = 3, .ubColor = 12}
};

void tileRefreshGrass(UWORD uwX) {
	UBYTE ubCurrTile = TILE_GRASS_NONE;
	if(g_pMainBuffer->pTileData[uwX-1][TILE_ROW_GRASS] <= TILE_GRASS_2) {
		ubCurrTile += 1; // Promote to TILE_GRASS_LEFT
	}
	else {
		// left neighbor is _RIGHT or _BOTH - decrease to _NONE or _LEFT
		tileBufferSetTile(
			g_pMainBuffer, uwX-1, TILE_ROW_GRASS,
			g_pMainBuffer->pTileData[uwX-1][TILE_ROW_GRASS] - 2
		);
	}
	if(uwX >= 10 || g_pMainBuffer->pTileData[uwX+1][TILE_ROW_GRASS] <= TILE_GRASS_2) {
		ubCurrTile += 2; // Promote to TILE_GRASS_RIGHT or _BOTH
	}
	else if(uwX < 10) {
		// right neighbor is _LEFT or _BOTH - decrease to _NONE or _RIGHT
		tileBufferSetTile(
			g_pMainBuffer, uwX+1, TILE_ROW_GRASS,
			g_pMainBuffer->pTileData[uwX+1][TILE_ROW_GRASS] - 1
		);
	}

	tileBufferSetTile(g_pMainBuffer, uwX, TILE_ROW_GRASS, ubCurrTile);
}

UBYTE tileIsSolid(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_STONE_1;
}

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_ROCK_1;
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

void tileInit(void) {
	for(UWORD x = 1; x < g_pMainBuffer->uTileBounds.sUwCoord.uwX; ++x) {
		for(UWORD y = 0; y < TILE_ROW_GRASS; ++y) {
			g_pMainBuffer->pTileData[x][y] = TILE_NONE;
		}
		g_pMainBuffer->pTileData[x][TILE_ROW_GRASS] = TILE_GRASS_1 + (x & 1);
		for(UWORD y = TILE_ROW_GRASS + 1; y < g_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
			// 2000 is max
			UWORD uwWhat = (uwRand() * 1000) / 65535;
			UWORD uwChanceAir = 50;
			UWORD uwChanceRock = CLAMP(y * 500 / 2000, 0, 500);
			UWORD uwChanceSilver = chanceTrapezoid(y, 10, 30, 50, 100, 5, 200);
			UWORD uwChanceGold = chanceTrapezoid(y, 60, 120, 150, 250, 2, 200);
			UWORD uwChanceUranium = chanceTrapezoid(y, 175, 400, 450, 600, 1, 200);
			UWORD uwChance;
			if(uwWhat < (uwChance = uwChanceRock)) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_STONE_1, TILE_STONE_2);
			}
			else if(
				uwWhat < (uwChance += uwChanceAir) &&
				tileIsSolid(x - 1, y) && tileIsSolid(x, y - 1)
			) {
				g_pMainBuffer->pTileData[x][y] = TILE_CAVE_BG+15;
			}
			else if(uwWhat < (uwChance += uwChanceSilver)) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_SILVER_1, TILE_SILVER_3);
			}
			else if(uwWhat < (uwChance += uwChanceGold)) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_GOLD_1, TILE_GOLD_3);
			}
			else if(uwWhat < (uwChance += uwChanceUranium)) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_URANIUM_1, TILE_URANIUM_3);
			}
			else {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_ROCK_1, TILE_ROCK_2);
			}
		}
	}
	for(UWORD y = 0; y < g_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
		g_pMainBuffer->pTileData[0][y] = TILE_ROCK_1;
	}
	g_pMainBuffer->pTileData[0][TILE_ROW_GRASS] = TILE_GRASS_1;
	// Shop
	g_pMainBuffer->pTileData[7][TILE_ROW_GRASS-3] = TILE_SHOP_CHIMNEY;
	for(UWORD y = 0; y < 3; ++y) {
		g_pMainBuffer->pTileData[3][TILE_ROW_GRASS-2+y] = TILE_SHOP_A1+y;
		g_pMainBuffer->pTileData[4][TILE_ROW_GRASS-2+y] = TILE_SHOP_B1+y;
		g_pMainBuffer->pTileData[5][TILE_ROW_GRASS-2+y] = TILE_SHOP_C1+y;
		g_pMainBuffer->pTileData[6][TILE_ROW_GRASS-2+y] = TILE_SHOP_D1+y;
		g_pMainBuffer->pTileData[7][TILE_ROW_GRASS-2+y] = TILE_SHOP_E1+y;
	}
	for(UWORD x = 3; x <= 7; ++x) {
		g_pMainBuffer->pTileData[x][TILE_ROW_GRASS+1] = ubRandMinMax(
			TILE_STONE_1, TILE_STONE_2
		);
	}
}

void tileExcavate(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
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

	// Load mineral to vehicle
	static const char * const szMessageFull = "Cargo full!";
	UBYTE ubTile = g_pMainBuffer->pTileData[uwX][uwY];
	UBYTE ubSlots = 0;
	if(s_pTileDefs[ubTile].szMsg) {
		ubSlots = s_pTileDefs[ubTile].ubSlots;
		ubSlots = MIN(ubSlots, pVehicle->ubCargoMax - pVehicle->ubCargoCurr);
		pVehicle->uwCargoScore += s_pTileDefs[ubTile].ubReward * ubSlots;
		pVehicle->ubCargoCurr += ubSlots;
		hudSetCargo(pVehicle->ubPlayerIdx, pVehicle->ubCargoCurr);
		const char *szMessage;
		UBYTE ubColor;
		if(pVehicle->ubCargoCurr == pVehicle->ubCargoMax) {
			szMessage = szMessageFull;
			ubColor = 6;
		}
		else {
			szMessage = s_pTileDefs[ubTile].szMsg;
			ubColor = s_pTileDefs[ubTile].ubColor;
		}
		textBobSet(
			&pVehicle->sTextBob, szMessage, ubColor,
			pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2 - 64/2,
			pVehicle->sBobBody.sPos.sUwCoord.uwY,
			pVehicle->sBobBody.sPos.sUwCoord.uwY - 32
		);
	}

	tileBufferSetTile(g_pMainBuffer, uwX, uwY, ubBg);
}
