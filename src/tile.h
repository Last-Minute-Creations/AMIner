/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TILE_H_
#define _TILE_H_

#include <ace/types.h>
#include "vehicle.h"

#define TILE_ROW_GRASS 5

void tileRefreshGrass(UWORD uwX);

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY);

void tileInit(void);

void tileExcavate(tVehicle *pVehicle, UWORD uwX, UWORD uwY);

#endif // _TILE_H_
