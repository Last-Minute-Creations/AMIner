#include "dynamite.h"
#include "explosion.h"
#include "tile.h"

void onExplosionPeak(ULONG ulData) {
	tDynamite *pDynamite = (tDynamite*)ulData;
	if(pDynamite->ubCount > 0) {
		UWORD uwX = pDynamite->pCoords[pDynamite->ubCount - 1].uwX;
		UWORD uwY = pDynamite->pCoords[pDynamite->ubCount - 1].uwY;
		// Hurt player if is on explosion tile
		// TODO

		// Remove last tile from list
		if(tileIsExplodable(uwX, uwY)) {
			tileExcavate(uwX, uwY);
		}
		// Decrease count and add explosion on last one
		--pDynamite->ubCount;
		if(pDynamite->ubCount > 0) {
			explosionAdd(
				pDynamite->pCoords[pDynamite->ubCount - 1].uwX << 5,
				pDynamite->pCoords[pDynamite->ubCount - 1].uwY << 5,
				onExplosionPeak, ulData, 1, 0
			);
		}
	}
}

static void dynamitePushXY(tDynamite *pDynamite, UWORD uwX, UWORD uwY) {
	pDynamite->pCoords[pDynamite->ubCount].uwX = uwX;
	pDynamite->pCoords[pDynamite->ubCount].uwY = uwY;
	++pDynamite->ubCount;
}

UBYTE dynamiteTrigger(
	tDynamite *pDynamite, UWORD uwTileX, UWORD uwTileY, UBYTE ubCount,
	tBombDir eDir
) {
	UBYTE ubTntUsed = 0;
	if(pDynamite->ubCount != 0 || ubCount == 0) {
		return 0;
	}
	BYTE bDeltaX = 0, bDeltaY = 0;
	switch(eDir) {
		case BOMB_DIR_LEFT:
			bDeltaX = -1;
			break;
		case BOMB_DIR_RIGHT:
			bDeltaX = 1;
			break;
		case BOMB_DIR_UP:
			bDeltaY = -1;
			break;
		case BOMB_DIR_DOWN:
			bDeltaY = 1;
			break;
		case BOMB_DIR_NONE:
		default:
			return 0;
	}
	for(UBYTE i = 0; i < ubCount; ++i) {
		uwTileX += bDeltaX;
		uwTileY += bDeltaY;
		if(1 <= uwTileX && uwTileX <= 10) {
			dynamitePushXY(pDynamite, uwTileX, uwTileY);
			++ubTntUsed;
		}
		else {
			break;
		}
	}
	const tUwCoordYX *pFirst = &pDynamite->pCoords[pDynamite->ubCount - 1];
	explosionAdd(
		pFirst->uwX << 5, pFirst->uwY << 5, onExplosionPeak, (ULONG)pDynamite, 1, 0
	);
	return ubTntUsed;
}
