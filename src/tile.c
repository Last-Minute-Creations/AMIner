/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/rand.h>
#include "game.h"
#include "hud.h"

const tTileDef const g_pTileDefs[TILE_COUNT] = {
	{.szMsg = 0, .ubReward = 0, .ubSlots = 0},
	[TILE_SILVER_1] = {.szMsg = "Silver x1", .ubReward = 5, .ubSlots = 1, .ubColor = 15},
	[TILE_SILVER_2] = {.szMsg = "Silver x2", .ubReward = 5, .ubSlots = 2, .ubColor = 15},
	[TILE_SILVER_3] = {.szMsg = "Silver x3", .ubReward = 5, .ubSlots = 3, .ubColor = 15},
	[TILE_GOLD_1] = {.szMsg = "Gold x1", .ubReward = 10, .ubSlots = 1, .ubColor = 14},
	[TILE_GOLD_2] = {.szMsg = "Gold x2", .ubReward = 10, .ubSlots = 2, .ubColor = 14},
	[TILE_GOLD_3] = {.szMsg = "Gold x3", .ubReward = 10, .ubSlots = 3, .ubColor = 14},
	[TILE_EMERALD_1] = {.szMsg = "Emerald x1", .ubReward = 15, .ubSlots = 1, .ubColor = 12},
	[TILE_EMERALD_2] = {.szMsg = "Emerald x2", .ubReward = 15, .ubSlots = 2, .ubColor = 12},
	[TILE_EMERALD_3] = {.szMsg = "Emerald x3", .ubReward = 15, .ubSlots = 3, .ubColor = 12},
	[TILE_RUBY_1] = {.szMsg = "Ruby x1", .ubReward = 20, .ubSlots = 1, .ubColor = 9},
	[TILE_RUBY_2] = {.szMsg = "Ruby x2", .ubReward = 20, .ubSlots = 2, .ubColor = 9},
	[TILE_RUBY_3] = {.szMsg = "Ruby x3", .ubReward = 20, .ubSlots = 3, .ubColor = 9},
	[TILE_MOONSTONE_1] = {.szMsg = "Moonstone x1", .ubReward = 25, .ubSlots = 1, .ubColor = 10},
	[TILE_MOONSTONE_2] = {.szMsg = "Moonstone x2", .ubReward = 25, .ubSlots = 2, .ubColor = 10},
	[TILE_MOONSTONE_3] = {.szMsg = "Moonstone x3", .ubReward = 25, .ubSlots = 3, .ubColor = 10},
	[TILE_COAL_1] = {.szMsg = "Coal x1", .ubReward = 5, .ubSlots = 1, .ubColor = 10},
	[TILE_COAL_2] = {.szMsg = "Coal x2", .ubReward = 5, .ubSlots = 2, .ubColor = 10},
	[TILE_COAL_3] = {.szMsg = "Coal x3", .ubReward = 5, .ubSlots = 3, .ubColor = 10},
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

void tileInit(UBYTE isCoalOnly, UBYTE isChallenge) {
	UWORD uwEndX = g_pMainBuffer->uTileBounds.sUwCoord.uwX;
	UWORD uwEndY = g_pMainBuffer->uTileBounds.sUwCoord.uwY;
	if(isChallenge) {
		uwEndY = TILE_ROW_CHALLENGE_FINISH + 1; // without +1 it's broken
	}
	for(UWORD x = 1; x < uwEndX; ++x) {
		for(UWORD y = 0; y < TILE_ROW_GRASS; ++y) {
			g_pMainBuffer->pTileData[x][y] = TILE_NONE;
		}
		g_pMainBuffer->pTileData[x][TILE_ROW_GRASS] = TILE_GRASS_1 + (x & 1);
		for(UWORD y = TILE_ROW_GRASS + 1; y < uwEndY; ++y) {
			// 2000 is max
			UWORD uwWhat = (uwRand() * 1000) / 65535;
			UWORD uwChanceAir = 50;
			UWORD uwChanceRock, uwChanceSilver, uwChanceGold, uwChanceEmerald, uwChanceRuby, uwChanceMoonstone;
			if(g_isChallenge) {
				uwChanceRock = 75;
				uwChanceSilver = chanceTrapezoid(
					y, TILE_ROW_GRASS, TILE_ROW_GRASS+5,
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
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_STONE_1, TILE_STONE_2);
			}
			else if(
				uwWhat < (uwChance += uwChanceAir) &&
				tileIsSolid(x - 1, y) && tileIsSolid(x, y - 1)
			) {
				g_pMainBuffer->pTileData[x][y] = TILE_CAVE_BG+15;
			}
			else if(uwWhat < (uwChance += uwChanceSilver)) {
				g_pMainBuffer->pTileData[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_SILVER_1, TILE_SILVER_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceGold)) {
				g_pMainBuffer->pTileData[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_GOLD_1, TILE_GOLD_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceEmerald)) {
				g_pMainBuffer->pTileData[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_EMERALD_1, TILE_EMERALD_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceRuby)) {
				g_pMainBuffer->pTileData[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_RUBY_1, TILE_RUBY_3)
				);
			}
			else if(uwWhat < (uwChance += uwChanceMoonstone)) {
				g_pMainBuffer->pTileData[x][y] = (
					isCoalOnly
						? ubRandMinMax(TILE_COAL_1, TILE_COAL_2)
						: ubRandMinMax(TILE_MOONSTONE_1, TILE_MOONSTONE_3)
				);
			}
			else {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_ROCK_1, TILE_ROCK_2);
			}
		}
	}
	for(UWORD y = 0; y < uwEndY; ++y) {
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

	if(isChallenge) {
		for(UWORD x = 0; x < uwEndX; ++x) {
			g_pMainBuffer->pTileData[x][TILE_ROW_CHALLENGE_CHECKPOINT_1] = TILE_CHECKPOINT;
			g_pMainBuffer->pTileData[x][TILE_ROW_CHALLENGE_CHECKPOINT_2] = TILE_CHECKPOINT;
			g_pMainBuffer->pTileData[x][TILE_ROW_CHALLENGE_CHECKPOINT_3] = TILE_CHECKPOINT;
			g_pMainBuffer->pTileData[x][TILE_ROW_CHALLENGE_FINISH] = TILE_FINISH;
			g_pMainBuffer->pTileData[x][TILE_ROW_CHALLENGE_FINISH+1] = TILE_STONE_1 + (x & 1);
		}
	}
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
