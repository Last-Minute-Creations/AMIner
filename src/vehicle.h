#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#include <ace/types.h>
#include "bob_new.h"

#define VEHICLE_WIDTH 32
#define VEHICLE_HEIGHT 23

typedef struct _tVehicle {
	tBCoordYX sSteer;
	tBobNew sBob;
	tBitMap *pFrames;
	tBitMap *pMask;
} tVehicle;

void vehicleCreate(void);

void vehicleDestroy(void);

void vehicleMove(BYTE bDirX, BYTE bDirY);

void vehicleProcess(void);

tVehicle g_sVehicle;

#endif // _VEHICLE_H_
