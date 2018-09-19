/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include "game.h"

void vehicleCreate(void) {
	logBlockBegin("vehicleCreate()");
	g_sVehicle.pFrames = bitmapCreateFromFile("data/drill/drill.bm");
	g_sVehicle.pMask = bitmapCreateFromFile("data/drill/drill_mask.bm");
	bobNewInit(
		&g_sVehicle.sBob, VEHICLE_WIDTH, VEHICLE_HEIGHT, 1,
		g_sVehicle.pFrames, g_sVehicle.pMask, 0, 0
	);
	logBlockEnd("vehicleCreate()");
}

void vehicleDestroy(void) {
	bitmapDestroy(g_sVehicle.pFrames);
	bitmapDestroy(g_sVehicle.pMask);
}

void vehicleMove(BYTE bDirX, BYTE bDirY) {
	g_sVehicle.sSteer.bX = bDirX;
	g_sVehicle.sSteer.bY = bDirY;

	if(bDirX > 0) {
		bobNewSetBitMapOffset(&g_sVehicle.sBob, 0);
	}
	else if(bDirX < 0) {
		bobNewSetBitMapOffset(&g_sVehicle.sBob, VEHICLE_HEIGHT);
	}
}

void vehicleProcess(void) {
	g_sVehicle.sBob.sPos.sUwCoord.uwX = CLAMP(
		g_sVehicle.sBob.sPos.sUwCoord.uwX + g_sVehicle.sSteer.bX * 2,
		0, 320 - g_sVehicle.sBob.uwWidth
	);

	UWORD uwTileBottom = (g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sBob.uwHeight) >> 5;
	UWORD uwTileCenter = (g_sVehicle.sBob.sPos.sUwCoord.uwX + g_sVehicle.sBob.uwWidth/2) >> 5;
	if(g_sVehicle.sSteer.bY < 0) {
		UWORD uwTileTop = (g_sVehicle.sBob.sPos.sUwCoord.uwY - 1) >> 5;
		// Flying
		// TODO collision with ceiling
		g_sVehicle.sBob.sPos.sUwCoord.uwY = MAX(
			0, g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sSteer.bY * 2
		);
	}
	else if(g_sVehicle.sSteer.bY > 0) {
		// Drilling
		if(g_pMainBuffer->pTileData[uwTileCenter][uwTileBottom]) {
			// Move to center of closer tile
			g_sVehicle.sBob.sPos.sUwCoord.uwX = uwTileCenter << 5;
			g_pMainBuffer->pTileData[uwTileCenter][uwTileBottom] = 0;
			tileBufferInvalidateTile(g_pMainBuffer, uwTileCenter, uwTileBottom);
			g_sVehicle.sBob.sPos.sUwCoord.uwY += 16;
		}
	}
	else {
		// Gravity
		if(
			!g_pMainBuffer->pTileData[uwTileCenter][uwTileBottom]
		) {
			g_sVehicle.sBob.sPos.sUwCoord.uwY += 2;
		}
		else {
			g_sVehicle.sBob.sPos.sUwCoord.uwY = (uwTileBottom << 5) - g_sVehicle.sBob.uwHeight;
		}
	}
	bobNewPush(&g_sVehicle.sBob);
}
