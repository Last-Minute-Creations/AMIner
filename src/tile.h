/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TILE_H_
#define _TILE_H_

#include <ace/types.h>
#include "vehicle.h"

#define TILE_ROW_GRASS 4
// #define TILE_ROW_CHALLENGE_CHECKPOINT_1 15
// #define TILE_ROW_CHALLENGE_CHECKPOINT_2 30
// #define TILE_ROW_CHALLENGE_CHECKPOINT_3 45
// #define TILE_ROW_CHALLENGE_FINISH 60
#define TILE_ROW_CHALLENGE_CHECKPOINT_1 10
#define TILE_ROW_CHALLENGE_FINISH 12
#define TILE_ROW_CHALLENGE_CHECKPOINT_2 14
#define TILE_ROW_CHALLENGE_CHECKPOINT_3 16

void tileRefreshGrass(UWORD uwX);

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY);

void tileInit(UBYTE isCoalOnly, UBYTE isChallenge);

void tileExcavate(tVehicle *pVehicle, UWORD uwX, UWORD uwY);

#endif // _TILE_H_
