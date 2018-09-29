/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#include <ace/types.h>
#include <fixmath/fix16.h>
#include "bob_new.h"

#define VEHICLE_WIDTH 32
#define VEHICLE_HEIGHT 23

typedef enum _tDrillDir {
	DRILL_DIR_NONE = 0,
	DRILL_DIR_H,
	DRILL_DIR_V
} tDrillDir;

typedef struct _tVehicle {
	tBCoordYX sSteer;
	tBobNew sBobBody;
	tBobNew sBobTrack;
	tBobNew sBobJet;
	tBobNew sBobTool;
	fix16_t fX;
	fix16_t fY;
	fix16_t fDx;
	fix16_t fDy;
	fix16_t fDestX;
	fix16_t fDestY;
	UBYTE ubTrackFrame;
	UBYTE ubTrackAnimCnt;
	UBYTE ubBodyShakeCnt;
	UBYTE ubDrillDir;
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
