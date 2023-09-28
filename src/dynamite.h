#ifndef _DYNAMITE_H_
#define _DYNAMITE_H_

#include <ace/types.h>
#include "direction.h"

#define VEHICLE_DYNAMITE_MAX 25

typedef struct _tDynamite {
	tUwCoordYX pCoords[VEHICLE_DYNAMITE_MAX];
	UBYTE ubCount;
	UBYTE ubCurrent;
	UBYTE ubPlayer;
} tDynamite;

UBYTE dynamiteTrigger(
	tDynamite *pDynamite, UWORD uwTileX, UWORD uwTileY, UBYTE ubCount,
	tDirection eDir
);

UBYTE dynamiteIsActive(const tDynamite *pDynamite);

#endif // _DYNAMITE_H_
