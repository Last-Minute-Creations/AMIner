#ifndef _DYNAMITE_H_
#define _DYNAMITE_H_

#include <ace/types.h>

#define VEHICLE_DYNAMITE_MAX 25

typedef enum _eDynamiteType {
	DYNAMITE_TYPE_3X3,
	DYNAMITE_TYPE_VERT,
	DYNAMITE_TYPE_HORZ,
	DYNAMITE_TYPE_5x5
} eDynamite;

typedef struct _tDynamite {
	tUwCoordYX pCoords[VEHICLE_DYNAMITE_MAX];
	UBYTE ubCount;
} tDynamite;

void dynamiteTrigger(
	tDynamite *pDynamite, UWORD uwTileX, UWORD uwTileY, UBYTE ubDynamiteType
);

#endif // _DYNAMITE_H_
