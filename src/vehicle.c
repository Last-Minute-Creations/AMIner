/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include "game.h"
#include "tile.h"

void vehicleCreate(void) {
	logBlockBegin("vehicleCreate()");
	g_sVehicle.pFrames = bitmapCreateFromFile("data/drill.bm");
	g_sVehicle.pMask = bitmapCreateFromFile("data/drill_mask.bm");
	bobNewInit(
		&g_sVehicle.sBob, VEHICLE_WIDTH, VEHICLE_HEIGHT, 1,
		g_sVehicle.pFrames, g_sVehicle.pMask, 0, 0
	);
	g_sVehicle.ubPayloadCurr = 0;
	g_sVehicle.ubPayloadMax = 10;
	g_sVehicle.uwPayloadScore = 0;
	g_sVehicle.ulScore = 0;
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
	UBYTE isOnGround = 0;
	// Limit X movement
	g_sVehicle.sBob.sPos.sUwCoord.uwX = CLAMP(
		g_sVehicle.sBob.sPos.sUwCoord.uwX + g_sVehicle.sSteer.bX * 2,
		0, 320 - g_sVehicle.sBob.uwWidth
	);

	UWORD uwCenterX = g_sVehicle.sBob.sPos.sUwCoord.uwX + g_sVehicle.sBob.uwWidth / 2;
	UWORD uwTileBottom = (g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sBob.uwHeight) >> 5;
	UWORD uwTileMid = (g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sBob.uwHeight /2) >> 5;
	UWORD uwTileCenter = uwCenterX >> 5;
	UBYTE isTouchingLeft = 0, isTouchingRight = 0;
	UWORD uwTileLeft = (uwCenterX - 1) >> 5;
	UWORD uwTileRight = (uwCenterX + 2) >> 5;

	if(g_sVehicle.sSteer.bX) {
		if(tileIsSolid(uwTileLeft, uwTileMid)) {
			g_sVehicle.sBob.sPos.sUwCoord.uwX = uwCenterX - g_sVehicle.sBob.uwWidth / 2 + 2;
			isTouchingLeft = 1;
		}
		else if(tileIsSolid(uwTileRight, uwTileMid)) {
			g_sVehicle.sBob.sPos.sUwCoord.uwX = uwCenterX - g_sVehicle.sBob.uwWidth / 2 - 2;
			isTouchingRight = 1;
		}
	}

	if(g_sVehicle.sSteer.bY < 0) {
		UWORD uwTileTop = (g_sVehicle.sBob.sPos.sUwCoord.uwY - 1) >> 5;
		// Flying
		g_sVehicle.sBob.sPos.sUwCoord.uwY = MAX(
			0, g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sSteer.bY * 2
		);
		if(tileIsSolid(uwTileCenter, uwTileTop)) {
			g_sVehicle.sBob.sPos.sUwCoord.uwY = (uwTileTop+1) << 5;
		}
	}
	else {
		if(!tileIsSolid(uwTileCenter, uwTileBottom)) {
			// Gravity
			g_sVehicle.sBob.sPos.sUwCoord.uwY += 2;
		}
		else {
			// Collision with ground
			isOnGround = 1;
			g_sVehicle.sBob.sPos.sUwCoord.uwY = (uwTileBottom << 5) - g_sVehicle.sBob.uwHeight;
		}
	}

	if(isOnGround) {
		if(g_sVehicle.sSteer.bX > 0 && isTouchingRight) {
			// Drilling right
			tileExcavate(&g_sVehicle, uwTileRight, uwTileMid);
			if(uwTileMid == 3) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileRight);
			}
		}
		else if(g_sVehicle.sSteer.bX < 0 && isTouchingLeft) {
			// Drilling left
			tileExcavate(&g_sVehicle, uwTileLeft, uwTileMid);
			if(uwTileMid == 3) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileLeft);
			}
		}
		else if(
			g_sVehicle.sSteer.bY > 0 && tileIsSolid(uwTileCenter, uwTileBottom)
		) {
			// Drilling down
			// Move to center of tile
			g_sVehicle.sBob.sPos.sUwCoord.uwX = uwTileCenter << 5;
			tileExcavate(&g_sVehicle, uwTileCenter, uwTileBottom);
			g_sVehicle.sBob.sPos.sUwCoord.uwY += 16;
			if(uwTileBottom == 3) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileCenter);
			}
		}

	}
	bobNewPush(&g_sVehicle.sBob);
}
