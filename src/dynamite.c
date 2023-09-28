#include "dynamite.h"
#include "explosion.h"
#include "tile.h"
#include "vehicle.h"
#include <ace/managers/log.h>

void onExplosionPeak(ULONG ulData) {
	tDynamite *pDynamite = (tDynamite*)ulData;
	UWORD uwX = pDynamite->pCoords[pDynamite->ubCurrent].uwX;
	UWORD uwY = pDynamite->pCoords[pDynamite->ubCurrent].uwY;
	// TODO Hurt player if is on explosion tile?

	// Excavate tile under explosion
	if(tileIsDrillable(uwX, uwY)) {
		vehicleExcavateTile(&g_pVehicles[pDynamite->ubPlayer], uwX, uwY);
	}

	// Trigger next explosion
	if(++pDynamite->ubCurrent < pDynamite->ubCount) {
		explosionAdd(
			pDynamite->pCoords[pDynamite->ubCurrent].uwX << 5,
			pDynamite->pCoords[pDynamite->ubCurrent].uwY << 5,
			onExplosionPeak, ulData, 1, 0
		);
	}
}

static void dynamitePushXY(tDynamite *pDynamite, UWORD uwX, UWORD uwY) {
	pDynamite->pCoords[pDynamite->ubCount].uwX = uwX;
	pDynamite->pCoords[pDynamite->ubCount].uwY = uwY;
	++pDynamite->ubCount;
}

UBYTE dynamiteTrigger(
	tDynamite *pDynamite, UWORD uwTileX, UWORD uwTileY, UBYTE ubNewCount,
	tDirection eDir
) {
	if(dynamiteIsActive(pDynamite) || ubNewCount == 0) {
		return 0;
	}
	BYTE bDeltaX = 0, bDeltaY = 0;
	switch(eDir) {
		case DIRECTION_LEFT:
			bDeltaX = -1;
			break;
		case DIRECTION_RIGHT:
			bDeltaX = 1;
			break;
		case DIRECTION_UP:
			bDeltaY = -1;
			break;
		case DIRECTION_DOWN:
			bDeltaY = 1;
			break;
		case DIRECTION_COUNT:
		default:
			return 0;
	}
	pDynamite->ubCount = 0;
	pDynamite->ubCurrent = 0;
	UBYTE ubTntUsed;
	for(ubTntUsed = 0; ubTntUsed < ubNewCount; ++ubTntUsed) {
		uwTileX += bDeltaX;
		uwTileY += bDeltaY;
		if(1 <= uwTileX && uwTileX <= 10) {
			dynamitePushXY(pDynamite, uwTileX, uwTileY);
		}
		else {
			break;
		}
	}
	const tUwCoordYX *pFirst = &pDynamite->pCoords[0];
	explosionAdd(
		pFirst->uwX << 5, pFirst->uwY << 5, onExplosionPeak, (ULONG)pDynamite, 1, 0
	);
	return ubTntUsed;
}

UBYTE dynamiteIsActive(const tDynamite *pDynamite) {
	UBYTE isActive = pDynamite->ubCurrent < pDynamite->ubCount;
	return isActive;
}
