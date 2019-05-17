/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _AMINER_BASE_TILE_H_
#define _AMINER_BASE_TILE_H_

#include <ace/managers/viewport/tilebuffer.h>

void baseTileCreate(tTileBufferManager *pManager);

void baseTileDestroy(void);

void baseTileProcess(void);

#endif // _AMINER_BASE_TILE_H_
