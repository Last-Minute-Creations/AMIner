/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include "game.h"
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/rand.h>

typedef enum _tTile {
	TILE_NONE = 0,
	TILE_GRASS_1,
	TILE_GRASS_2,
	TILE_GRASS_NONE,
	TILE_GRASS_LEFT,
	TILE_GRASS_RIGHT,
	TILE_GRASS_BOTH,
	TILE_CAVE_BG,
	TILE_ROCK_1 = TILE_CAVE_BG + 16,
	TILE_ROCK_2,
	TILE_STONE_1,
	TILE_STONE_2,
	TILE_GOLD_1,
	TILE_GOLD_2,
	TILE_GOLD_3,
	TILE_GOLD_4
} tTile;
// TODO sapphire, emerald, topaz

void tileRefreshGrass(UWORD uwX) {
	UBYTE ubCurrTile = TILE_GRASS_NONE;
	if(!uwX || g_pMainBuffer->pTileData[uwX-1][2] <= TILE_GRASS_2) {
		ubCurrTile += 1; // Promote to TILE_GRASS_LEFT
	}
	else if(uwX){
		// left neighbor is _RIGHT or _BOTH - decrease to _NONE or _LEFT
		tileBufferSetTile(
			g_pMainBuffer, uwX-1, 2, g_pMainBuffer->pTileData[uwX-1][2] - 2
		);
	}
	if(uwX >= 9 || g_pMainBuffer->pTileData[uwX+1][2] <= TILE_GRASS_2) {
		ubCurrTile += 2; // Promote to TILE_GRASS_RIGHT or _BOTH
	}
	else if(uwX < 9) {
		// right neighbor is _LEFT or _BOTH - decrease to _NONE or _RIGHT
		tileBufferSetTile(
			g_pMainBuffer, uwX+1, 2, g_pMainBuffer->pTileData[uwX+1][2] - 1
		);
	}

	tileBufferSetTile(g_pMainBuffer, uwX, 2, ubCurrTile);
}

UBYTE tileIsSolid(UWORD uwX, UWORD uwY) {
	return g_pMainBuffer->pTileData[uwX][uwY] >= TILE_ROCK_1;
}

void tileInit(void) {
	for(UWORD x = 0; x < g_pMainBuffer->uTileBounds.sUwCoord.uwX; ++x) {
		g_pMainBuffer->pTileData[x][0] = TILE_NONE;
		g_pMainBuffer->pTileData[x][1] = TILE_NONE;
		g_pMainBuffer->pTileData[x][2] = TILE_GRASS_1 + (x & 1);
		for(UWORD y = 3; y < g_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
			UBYTE ubChance = (ubRand() * 100) / 255;
			UBYTE ubChanceRock = CLAMP(y * 50 / 2000, 0, 50);
			if(ubChance < 20) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_GOLD_1, TILE_GOLD_4);
			}
			else if(ubChance < 20 + ubChanceRock) {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_STONE_1, TILE_STONE_2);
			}
			else if(ubChance < 20 + ubChanceRock + 5 && (!x || tileIsSolid(x-1,y)) && tileIsSolid(x,y-1)) {
				g_pMainBuffer->pTileData[x][y] = TILE_CAVE_BG+15;
			}
			else {
				g_pMainBuffer->pTileData[x][y] = ubRandMinMax(TILE_ROCK_1, TILE_ROCK_2);
			}
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
	if(uwX >= 9 || tileIsSolid(uwX+1, uwY)) {
		ubBg += 4;
	}
	else if(uwX < 9) {
		g_pMainBuffer->pTileData[uwX+1][uwY] -= 8;
		tileBufferInvalidateTile(g_pMainBuffer, uwX+1, uwY);
	}

	// left
	if(!uwX || tileIsSolid(uwX-1, uwY)) {
		ubBg += 8;
	}
	else if(uwX) {
		g_pMainBuffer->pTileData[uwX-1][uwY] -= 4;
		tileBufferInvalidateTile(g_pMainBuffer, uwX-1, uwY);
	}
	tileBufferSetTile(g_pMainBuffer, uwX, uwY, ubBg);
}
