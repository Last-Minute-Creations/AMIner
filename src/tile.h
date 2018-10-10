/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TILE_H_
#define _TILE_H_

#include <ace/types.h>
#include "vehicle.h"

#define TILE_ROW_GRASS 4
#define TILE_ROW_CHALLENGE_CHECKPOINT_1 (TILE_ROW_GRASS + 15)
#define TILE_ROW_CHALLENGE_CHECKPOINT_2 (TILE_ROW_CHALLENGE_CHECKPOINT_1 + 15)
#define TILE_ROW_CHALLENGE_CHECKPOINT_3 (TILE_ROW_CHALLENGE_CHECKPOINT_2 + 20)
#define TILE_ROW_CHALLENGE_FINISH (TILE_ROW_CHALLENGE_CHECKPOINT_3 + 15)

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
	TILE_ROCK_1, TILE_ROCK_2,
	TILE_GOLD_1, TILE_GOLD_2, TILE_GOLD_3,
	TILE_SILVER_1, TILE_SILVER_2, TILE_SILVER_3,
	TILE_EMERALD_1, TILE_EMERALD_2, TILE_EMERALD_3,
	TILE_RUBY_1, TILE_RUBY_2, TILE_RUBY_3,
	TILE_MOONSTONE_1, TILE_MOONSTONE_2, TILE_MOONSTONE_3,
	TILE_COAL_1, TILE_COAL_2, TILE_COAL_3,
	TILE_FINISH,
	TILE_CHECKPOINT,
	TILE_BOMB_1, TILE_BOMB_2,
	TILE_COUNT
} tTile;
// TODO sapphire, topaz

typedef struct _tTileDef {
	const char *szMsg;
	UBYTE ubReward;
	UBYTE ubSlots;
	UBYTE ubColor;
} tTileDef;

void tileRefreshGrass(UWORD uwX);

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY);

void tileInit(UBYTE isCoalOnly, UBYTE isChallenge);

void tileExcavate(UWORD uwX, UWORD uwY);

extern const tTileDef const g_pTileDefs[TILE_COUNT];

#endif // _TILE_H_
