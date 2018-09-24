/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
	UBYTE ubPayloadMax;
	UBYTE ubPayloadCurr;
	UWORD uwPayloadScore;
	ULONG ulScore;
} tVehicle;

void vehicleCreate(void);

void vehicleDestroy(void);

void vehicleMove(BYTE bDirX, BYTE bDirY);

void vehicleProcess(void);

tVehicle g_sVehicle;

#endif // _VEHICLE_H_
