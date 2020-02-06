#ifndef _DYNAMITE_H_
#define _DYNAMITE_H_

#include <ace/types.h>

#define VEHICLE_DYNAMITE_MAX 25

typedef enum _tBombDir {
	BOMB_DIR_NONE,
	BOMB_DIR_LEFT,
	BOMB_DIR_RIGHT,
	BOMB_DIR_UP,
	BOMB_DIR_DOWN,
} tBombDir;

typedef struct _tDynamite {
	tUwCoordYX pCoords[VEHICLE_DYNAMITE_MAX];
	UBYTE ubCount;
	UBYTE ubCurrent;
	UBYTE ubPlayer;
} tDynamite;

UBYTE dynamiteTrigger(
	tDynamite *pDynamite, UWORD uwTileX, UWORD uwTileY, UBYTE ubCount,
	tBombDir eDir
);

UBYTE dynamiteIsActive(const tDynamite *pDynamite);

#endif // _DYNAMITE_H_
