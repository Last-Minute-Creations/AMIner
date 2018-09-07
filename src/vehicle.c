#include "vehicle.h"

void vehicleCreate(void) {
	g_sVehicle.pFrames = bitmapCreateFromFile("data/drill/drill.bm");
	g_sVehicle.pMask = bitmapCreateFromFile("data/drill/drill_mask.bm");
	bobNewInit(
		&g_sVehicle.sBob, VEHICLE_WIDTH, VEHICLE_HEIGHT, 1,
		g_sVehicle.pFrames, g_sVehicle.pMask, 0, 0
	);
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
	g_sVehicle.sBob.sPos.sUwCoord.uwX += g_sVehicle.sSteer.bX * 2;
	g_sVehicle.sBob.sPos.sUwCoord.uwY += g_sVehicle.sSteer.bY * 2;
	bobNewPush(&g_sVehicle.sBob);
}
