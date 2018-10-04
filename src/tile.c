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
	TILE_SHOP_1,
	TILE_SHOP_2,
	TILE_SHOP_3,
	TILE_SHOP_4,
	TILE_GRASS_1,
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
	TILE_GOLD_4
} tTile;
// TODO sapphire, emerald, topaz

void tileRefreshGrass(UWORD uwX) {
	UBYTE ubCurrTile = TILE_GRASS_NONE;
	if(g_pMainBuffer->pTileData[uwX-1][TILE_ROW_GRASS] <= TILE_GRASS_2) {
		ubCurrTile += 1; // Promote to TILE_GRASS_LEFT
	}
	else {
		// left neighbor is _RIGHT or _BOTH - decrease to _NONE or _LEFT
		tileBufferSetTile(
			g_pMainBuffer, uwX-1, 2, g_pMainBuffer->pTileData[uwX-1][TILE_ROW_GRASS] - 2
		);
	}
	if(uwX >= 10 || g_pMainBuffer->pTileData[uwX+1][TILE_ROW_GRASS] <= TILE_GRASS_2) {
		ubCurrTile += 2; // Promote to TILE_GRASS_RIGHT or _BOTH
	}
	else if(uwX < 10) {
		// right neighbor is _LEFT or _BOTH - decrease to _NONE or _RIGHT
		tileBufferSetTile(
			g_pMainBuffer, uwX+1, 2, g_pMainBuffer->pTileData[uwX+1][TILE_ROW_GRASS] - 1
		);
	}

	tileBufferSetTile(g_pMainBuffer, uwX, 2, ubCurrTile);
}

UBYTE tileIsSolid(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_STONE_1;
}

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_ROCK_1;
}

void tileInit(void) {
	for(UWORD x = 1; x < g_pMainBuffer->uTileBounds.sUwCoord.uwX; ++x) {
		for(UWORD y = 0; y < TILE_ROW_GRASS; ++y) {
			g_pMainBuffer->pTileData[x][y] = TILE_NONE;
		}
		g_pMainBuffer->pTileData[x][TILE_ROW_GRASS] = TILE_GRASS_1 + (x & 1);
		for(UWORD y = TILE_ROW_GRASS + 1; y < g_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
			UBYTE ubChance = (ubRand() * 100) / 255;
			UBYTE ubChanceRock = CLAMP(y * 50 / 2000, 0, 50);
			if(ubChance < 20) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_GOLD_1, TILE_GOLD_4);
			}
			else if(ubChance < 20 + ubChanceRock) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_STONE_1, TILE_STONE_2);
			}
			else if(
				ubChance < 20 + ubChanceRock + 5 &&
				tileIsSolid(x - 1, y) && tileIsSolid(x, y - 1)
			) {
				g_pMainBuffer->pTileData[x][y] = TILE_CAVE_BG+15;
			}
			else {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_ROCK_1, TILE_ROCK_2);
			}
		}
	}
	for(UWORD y = 0; y < g_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
		g_pMainBuffer->pTileData[0][y] = TILE_ROCK_1;
	}
	// Shop
	g_pMainBuffer->pTileData[7][1] = TILE_SHOP_1;
	g_pMainBuffer->pTileData[8][1] = TILE_SHOP_2;
	g_pMainBuffer->pTileData[7][2] = TILE_SHOP_3;
	g_pMainBuffer->pTileData[8][2] = TILE_SHOP_4;
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
	static const char * const pMessages[] = {"GOLD x1", "GOLD x2", "GOLD x3", "GOLD x4", "GOLD x5"};
	static const char * const szMessageFull = "Cargo full!";
	UBYTE ubTile = g_pMainBuffer->pTileData[uwX][uwY];
	UBYTE ubScorePerSlot = 0;
	UBYTE ubSlots = 0;
	if(TILE_GOLD_1 <= ubTile && ubTile <= TILE_GOLD_4) {
		ubScorePerSlot = 5;
		ubSlots = (ubTile == TILE_GOLD_4 ? 5 : ubTile - TILE_GOLD_1 + 1);
	}
	if(ubSlots) {
		ubSlots = MIN(ubSlots, pVehicle->ubCargoMax - pVehicle->ubCargoCurr);
		pVehicle->uwCargoScore += ubScorePerSlot * ubSlots;
		pVehicle->ubCargoCurr += ubSlots;
		hudSetCargo(pVehicle->ubCargoCurr);
		const char *szMessage;
		UBYTE ubColor;
		if(pVehicle->ubCargoCurr == pVehicle->ubCargoMax) {
			szMessage = szMessageFull;
			ubColor = 6;
		}
		else {
			szMessage = pMessages[ubSlots-1];
			ubColor = 14;
		}
		textBobSet(
			&g_sVehicle.sTextBob, szMessage, ubColor,
			g_sVehicle.sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2 - 64/2,
			g_sVehicle.sBobBody.sPos.sUwCoord.uwY,
			g_sVehicle.sBobBody.sPos.sUwCoord.uwY - 32
		);

	}

	tileBufferSetTile(g_pMainBuffer, uwX, uwY, ubBg);
}
