/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TILE_H_
#define _TILE_H_

#include <ace/types.h>

void tileRefreshGrass(UWORD uwX);

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

void tileInit(void);

#endif // _TILE_H_
