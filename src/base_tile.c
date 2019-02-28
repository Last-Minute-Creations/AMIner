/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base_tile.h"

static UBYTE s_ubBaseCurrent;
static tBitMap *s_pBaseTiles[2];
tTileBufferManager *s_pManager;

static void baseTileLoad(UBYTE ubBaseIdx) {
	const tBitMap *pBase = s_pBaseTiles[ubBaseIdx];
	tBitMap *pTiles = s_pManager->pTileSet;
	CopyMemQuick(
		pBase->Planes[0], pTiles->Planes[0],
		pBase->Rows * pBase->BytesPerRow
	);
	s_ubBaseCurrent = ubBaseIdx;
}

void baseTileCreate(tTileBufferManager *pManager) {
	s_pManager = pManager;
	s_pBaseTiles[0] = bitmapCreateFromFile("data/base0.bm", 1);
	s_pBaseTiles[1] = bitmapCreateFromFile("data/base1.bm", 1);
	baseTileLoad(0);
}

void baseTileDestroy(void) {
	bitmapDestroy(s_pBaseTiles[0]);
	bitmapDestroy(s_pBaseTiles[1]);
}

void baseTileProcess(void) {
	UWORD uwCamY = s_pManager->pCamera->uPos.sUwCoord.uwY;
	if(uwCamY < 512 && s_ubBaseCurrent != 0) {
		baseTileLoad(0);
	}
	else if(uwCamY > 1024 && s_ubBaseCurrent != 1) {
		baseTileLoad(1);
	}
}
