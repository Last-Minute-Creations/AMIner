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

void dynamiteTrigger(
	tDynamite *pDynamite, UWORD uwTileX, UWORD uwTileY, UBYTE ubDynamiteType
) {
	if(pDynamite->ubCount != 0) {
		return;
	}
	UBYTE isLeft = (uwTileX > 1);
	UBYTE isRight = (uwTileX < 10);
	switch(ubDynamiteType) {
		case DYNAMITE_TYPE_5x5:
		case DYNAMITE_TYPE_3X3:
			if(isLeft) {
				dynamitePushXY(pDynamite, uwTileX - 1, uwTileY - 1);
				dynamitePushXY(pDynamite, uwTileX - 1, uwTileY + 0);
				dynamitePushXY(pDynamite, uwTileX - 1, uwTileY + 1);
			}
			dynamitePushXY(pDynamite, uwTileX, uwTileY + 1);
			if(isRight) {
				dynamitePushXY(pDynamite, uwTileX + 1, uwTileY + 1);
				dynamitePushXY(pDynamite, uwTileX + 1, uwTileY + 0);
				dynamitePushXY(pDynamite, uwTileX + 1, uwTileY - 1);
			}
			dynamitePushXY(pDynamite, uwTileX, uwTileY - 1);
			dynamitePushXY(pDynamite, uwTileX, uwTileY);
			break;
			break;
		case DYNAMITE_TYPE_VERT:
			dynamitePushXY(pDynamite, uwTileX, uwTileY - 2);
			dynamitePushXY(pDynamite, uwTileX, uwTileY + 2);
			dynamitePushXY(pDynamite, uwTileX, uwTileY - 1);
			dynamitePushXY(pDynamite, uwTileX, uwTileY + 1);
			dynamitePushXY(pDynamite, uwTileX, uwTileY);
			break;
		case DYNAMITE_TYPE_HORZ:
			if(isLeft) {
				dynamitePushXY(pDynamite, uwTileX - 1, uwTileY);
			}
			if(isRight) {
				dynamitePushXY(pDynamite, uwTileX + 1, uwTileY);
			}
			dynamitePushXY(pDynamite, uwTileX, uwTileY);
			break;
	}
	const tUwCoordYX *pFirst = &pDynamite->pCoords[pDynamite->ubCount - 1];
	explosionAdd(
		pFirst->uwX << 5, pFirst->uwY << 5, onExplosionPeak, (ULONG)pDynamite, 1, 0
	);
}
