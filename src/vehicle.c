/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include <ace/managers/rand.h>
#include "hud.h"
#include "game.h"
#include "tile.h"

#define VEHICLE_BODY_HEIGHT 18
#define VEHICLE_TRACK_HEIGHT 7
#define VEHICLE_TRACK_DRILL_HEIGHT 7
#define VEHICLE_TRACK_JET_HEIGHT 5
#define VEHICLE_FLAME_HEIGHT 7
#define VEHICLE_TOOL_WIDTH 16
#define VEHICLE_TOOL_HEIGHT 17

#define TRACK_OFFSET_TRACK 0
#define TRACK_OFFSET_JET 14
#define TRACK_OFFSET_DRILL 21
#define DRILL_V_ANIM_LEN (VEHICLE_TRACK_HEIGHT + VEHICLE_TRACK_DRILL_HEIGHT - 1)

tBitMap *s_pBodyFrames, *s_pBodyMask;
tBitMap *s_pTrackFrames, *s_pTrackMask;
tBitMap *s_pJetFrames, *s_pJetMask;
tBitMap *s_pToolFrames, *s_pToolMask;

UBYTE s_pJetAnimOffsets[VEHICLE_TRACK_HEIGHT * 2 + 1] = {0,1,2,3,4,5,4,3,2,1,0};

void vehicleCreate(void) {
	logBlockBegin("vehicleCreate()");
	// Load gfx
	s_pBodyFrames = bitmapCreateFromFile("data/drill.bm");
	s_pBodyMask = bitmapCreateFromFile("data/drill_mask.bm");
	s_pTrackFrames = bitmapCreateFromFile("data/track.bm");
	s_pTrackMask = bitmapCreateFromFile("data/track_mask.bm");
	s_pJetFrames = bitmapCreateFromFile("data/jet.bm");
	s_pJetMask = bitmapCreateFromFile("data/jet_mask.bm");
	s_pToolFrames = bitmapCreateFromFile("data/tool.bm");
	s_pToolMask = bitmapCreateFromFile("data/tool_mask.bm");

	// Setup bobs
	bobNewInit(
		&g_sVehicle.sBobBody, VEHICLE_WIDTH, VEHICLE_BODY_HEIGHT, 1,
		s_pBodyFrames, s_pBodyMask, 0, 0
	);
	bobNewInit(
		&g_sVehicle.sBobTrack, VEHICLE_WIDTH, VEHICLE_TRACK_HEIGHT, 1,
		s_pTrackFrames, s_pTrackMask, 0, 0
	);
	bobNewInit(
		&g_sVehicle.sBobJet, VEHICLE_WIDTH, VEHICLE_FLAME_HEIGHT, 1,
		s_pJetFrames, s_pJetMask, 0, 0
	);
	bobNewInit(
		&g_sVehicle.sBobTool, VEHICLE_TOOL_WIDTH, VEHICLE_TOOL_HEIGHT, 1,
		s_pToolFrames, s_pToolMask, 0, 0
	);

	// Initial values
	g_sVehicle.ubPayloadCurr = 0;
	g_sVehicle.ubPayloadMax = 50;
	g_sVehicle.uwPayloadScore = 0;
	g_sVehicle.ulScore = 0;
	g_sVehicle.fX = 0;
	g_sVehicle.fY = 0;
	g_sVehicle.fDx = 0;
	g_sVehicle.fDy = 0;
	g_sVehicle.ubDrillDir = 0;

	g_sVehicle.ubTrackAnimCnt = 0;
	g_sVehicle.ubTrackFrame = 0;
	g_sVehicle.ubBodyShakeCnt = 0;
	g_sVehicle.ubJetShowFrame = 0;
	g_sVehicle.ubJetAnimFrame = 0;
	g_sVehicle.ubJetAnimCnt = 0;
	g_sVehicle.ubToolAnimCnt = 0;
	g_sVehicle.ubDrillVAnimCnt = 0;
	logBlockEnd("vehicleCreate()");
}

void vehicleDestroy(void) {
	bitmapDestroy(s_pBodyFrames);
	bitmapDestroy(s_pBodyMask);
	bitmapDestroy(s_pTrackFrames);
	bitmapDestroy(s_pTrackMask);
	bitmapDestroy(s_pJetFrames);
	bitmapDestroy(s_pJetMask);
	bitmapDestroy(s_pToolFrames);
	bitmapDestroy(s_pToolMask);
}

void vehicleMove(BYTE bDirX, BYTE bDirY) {
	// Always register steer requests so that vehicle can continuously drill down
	g_sVehicle.sSteer.bX = bDirX;
	g_sVehicle.sSteer.bY = bDirY;

	// No vehicle rotating when drilling
	if(g_sVehicle.ubDrillDir != DRILL_DIR_NONE) {
		return;
	}
	if(bDirX > 0) {
		bobNewSetBitMapOffset(&g_sVehicle.sBobBody, 0);
	}
	else if(bDirX < 0) {
		bobNewSetBitMapOffset(&g_sVehicle.sBobBody, VEHICLE_BODY_HEIGHT);
	}
}

static inline void vehicleSetTool(tToolState eToolState, UBYTE ubFrame) {
	if(eToolState == TOOL_STATE_IDLE) {
		if(!g_sVehicle.sBobBody.uwOffsetY) {
			// Facing right
			g_sVehicle.sBobTool.sPos.sUwCoord.uwX += 24;
		}
		// Vertical drill anim
		bobNewSetBitMapOffset(&g_sVehicle.sBobTool, 0);
	}
	else { // if(eToolState == TOOL_STATE_DRILL)
		g_sVehicle.sBobTool.sPos.sUwCoord.uwY += 3;
		if(!g_sVehicle.sBobBody.uwOffsetY) {
			// Facing right
			g_sVehicle.sBobTool.sPos.sUwCoord.uwX += 24+3;
			bobNewSetBitMapOffset(
				&g_sVehicle.sBobTool, (1 + ubFrame) * VEHICLE_TOOL_HEIGHT
			);
		}
		else {
			// Facing left
			g_sVehicle.sBobTool.sPos.sUwCoord.uwX += -13+3;
			bobNewSetBitMapOffset(
				&g_sVehicle.sBobTool, (3 + ubFrame) * VEHICLE_TOOL_HEIGHT
			);
		}
	}
}

static void vehicleProcessMovement(void) {
	UBYTE isOnGround = 0;
	const fix16_t fMaxDx = 4 * fix16_one;
	const fix16_t fAccX = fix16_one / 8;
	const fix16_t fFrictX = fix16_one / 12;

	if(g_sVehicle.sSteer.bX) {
		if(g_sVehicle.sSteer.bX > 0) {
			g_sVehicle.fDx = MIN(g_sVehicle.fDx + fAccX, fMaxDx);
		}
		else {
			g_sVehicle.fDx = MAX(g_sVehicle.fDx - fAccX, -fMaxDx);
		}
	}
	else {
		if(g_sVehicle.fDx > 0) {
			g_sVehicle.fDx = MAX(0, g_sVehicle.fDx - fFrictX);
		}
		else {
			g_sVehicle.fDx = MIN(0, g_sVehicle.fDx + fFrictX);
		}
	}

	// Limit X movement
	const fix16_t fMaxPosX = fix16_one * (320 - VEHICLE_WIDTH);
	g_sVehicle.fX = CLAMP(g_sVehicle.fX + g_sVehicle.fDx, 0, fMaxPosX);
	g_sVehicle.sBobBody.sPos.sUwCoord.uwX = fix16_to_int(g_sVehicle.fX);
	UBYTE ubAdd = (g_sVehicle.sBobBody.sPos.sUwCoord.uwY > (1 + TILE_ROW_GRASS) * 32) ? 4 : 8;
	UBYTE ubHalfWidth = 12;

	UWORD uwCenterX = g_sVehicle.sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH / 2;
	UWORD uwTileBottom = (g_sVehicle.sBobBody.sPos.sUwCoord.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
	UWORD uwTileMid = (g_sVehicle.sBobBody.sPos.sUwCoord.uwY + VEHICLE_HEIGHT / 2) >> 5;
	UWORD uwTileCenter = uwCenterX >> 5;
	UBYTE isTouchingLeft = 0, isTouchingRight = 0;
	UWORD uwTileLeft = (uwCenterX - ubHalfWidth) >> 5;
	UWORD uwTileRight = (uwCenterX + ubHalfWidth) >> 5;

	if(tileIsSolid(uwTileLeft, uwTileMid)) {
		g_sVehicle.fX = fix16_from_int(((uwTileLeft+1) << 5) - VEHICLE_WIDTH / 2 + ubHalfWidth);
		g_sVehicle.fDx = 0;
		isTouchingLeft = 1;
	}
	else if(tileIsSolid(uwTileRight, uwTileMid)) {
		g_sVehicle.fX = fix16_from_int((uwTileRight << 5) - VEHICLE_WIDTH / 2 - ubHalfWidth);
		g_sVehicle.fDx = 0;
		isTouchingRight = 1;
	}

	const fix16_t fMaxFlightDy = 3 * fix16_one;
	const fix16_t fAccFlight = fix16_one / 12;
	const fix16_t fMaxGravDy = 4 * fix16_one;
	const fix16_t fAccGrav = fix16_one / 8;
	if(g_sVehicle.sSteer.bY < 0) {
		if(g_sVehicle.ubJetShowFrame == 10) {
			g_sVehicle.fDy = MAX(-fMaxFlightDy, g_sVehicle.fDy - fAccFlight);
		}
		else {
			++g_sVehicle.ubJetShowFrame;
		}
	}
	else {
		if(g_sVehicle.ubJetShowFrame) {
			--g_sVehicle.ubJetShowFrame;
		}
		g_sVehicle.fDy = MIN(fMaxGravDy, g_sVehicle.fDy + fAccGrav);
	}

	if(g_sVehicle.fDy < 0) {
		UWORD uwTileTop = (fix16_to_int(g_sVehicle.fY) - 1) >> 5;
		// Flying
		g_sVehicle.fY = MAX(0, g_sVehicle.fY + g_sVehicle.fDy);
		if(tileIsSolid(uwTileCenter, uwTileTop)) {
			g_sVehicle.fY = fix16_from_int((uwTileTop+1) << 5);
			g_sVehicle.fDy = 0;
		}
	}
	else {
		if(!tileIsSolid(uwTileCenter, uwTileBottom)) {
			// Gravity
			g_sVehicle.fY += g_sVehicle.fDy;
		}
		else {
			// Collision with ground
			isOnGround = 1;
			g_sVehicle.fY = fix16_from_int((uwTileBottom << 5) - VEHICLE_HEIGHT - ubAdd);
			g_sVehicle.fDy = 0;
		}
	}

	// Update track bob
	g_sVehicle.sBobBody.sPos.sUwCoord.uwY = fix16_to_int(g_sVehicle.fY);
	g_sVehicle.sBobTrack.sPos.ulYX = g_sVehicle.sBobBody.sPos.ulYX;
	g_sVehicle.sBobTrack.sPos.sUwCoord.uwY += VEHICLE_BODY_HEIGHT;
	if(g_sVehicle.ubJetShowFrame == 0) {
		// Jet hidden
		if(g_sVehicle.fDx) {
			++g_sVehicle.ubTrackAnimCnt;
			if(g_sVehicle.ubTrackAnimCnt >= (5 - fix16_to_int(fix16_abs(g_sVehicle.fDx)))) {
				g_sVehicle.ubTrackFrame = !g_sVehicle.ubTrackFrame;
				bobNewSetBitMapOffset(&g_sVehicle.sBobTrack, g_sVehicle.ubTrackFrame * VEHICLE_TRACK_HEIGHT);
				g_sVehicle.ubTrackAnimCnt = 0;
			}
		}

		++g_sVehicle.ubBodyShakeCnt;
		UBYTE ubShakeSpeed = (g_sVehicle.fDx ? 5 : 10);
		if(g_sVehicle.ubBodyShakeCnt >= 2 * ubShakeSpeed) {
			g_sVehicle.ubBodyShakeCnt = 0;
		}
		if(g_sVehicle.ubBodyShakeCnt >= ubShakeSpeed) {
			g_sVehicle.sBobBody.sPos.sUwCoord.uwY += 1;
		}
	}
	else {
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY += s_pJetAnimOffsets[g_sVehicle.ubJetShowFrame];
		if(g_sVehicle.ubJetShowFrame == 5) {
			bobNewSetBitMapOffset(&g_sVehicle.sBobTrack, g_sVehicle.sSteer.bY ? 2*VEHICLE_TRACK_HEIGHT : 0);
		}
		else if(g_sVehicle.ubJetShowFrame == 10) {
			// Update jet pos
			g_sVehicle.ubJetAnimCnt = (g_sVehicle.ubJetAnimCnt + 1) & 15;
			bobNewSetBitMapOffset(
				&g_sVehicle.sBobJet, VEHICLE_FLAME_HEIGHT * (g_sVehicle.ubJetAnimCnt / 4)
			);
			g_sVehicle.sBobJet.sPos.ulYX = g_sVehicle.sBobTrack.sPos.ulYX;
			g_sVehicle.sBobJet.sPos.sUwCoord.uwY += VEHICLE_TRACK_JET_HEIGHT;
			bobNewPush(&g_sVehicle.sBobJet);
		}
	}
	bobNewPush(&g_sVehicle.sBobTrack);

	// Drilling
	if(isOnGround) {
		if(g_sVehicle.sSteer.bX > 0 && isTouchingRight) {
			g_sVehicle.ubDrillDir = DRILL_DIR_H;
			g_sVehicle.fDestX = fix16_from_int(uwTileRight << 5);
			g_sVehicle.fDestY = g_sVehicle.fY;
			g_sVehicle.ubDrillState = DRILL_STATE_DRILLING;
			g_sVehicle.fDx = 0;
			g_sVehicle.fDy = 0;
		}
		else if(g_sVehicle.sSteer.bX < 0 && isTouchingLeft) {
			g_sVehicle.ubDrillDir = DRILL_DIR_H;
			g_sVehicle.fDestX = fix16_from_int(uwTileLeft << 5);
			g_sVehicle.fDestY = g_sVehicle.fY;
			g_sVehicle.ubDrillState = DRILL_STATE_DRILLING;
			g_sVehicle.fDx = 0;
			g_sVehicle.fDy = 0;
		}
		else if(
			g_sVehicle.sSteer.bY > 0 && tileIsSolid(uwTileCenter, uwTileBottom)
		) {
			g_sVehicle.ubDrillDir = DRILL_DIR_V;
			g_sVehicle.fDestX = fix16_from_int(uwTileCenter << 5);
			g_sVehicle.fDestY = fix16_from_int(((uwTileBottom + 1) << 5) - VEHICLE_HEIGHT - 4);
			g_sVehicle.ubDrillState = DRILL_STATE_ANIM_IN;
			g_sVehicle.fDx = 0;
			g_sVehicle.fDy = 0;
		}
	}
	bobNewPush(&g_sVehicle.sBobBody);

	// Tool
	g_sVehicle.sBobTool.sPos.ulYX = g_sVehicle.sBobBody.sPos.ulYX;
	vehicleSetTool(TOOL_STATE_IDLE, 0);
	bobNewPush(&g_sVehicle.sBobTool);

	if(
		7*32 <= g_sVehicle.sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2 &&
		g_sVehicle.sBobBody.sPos.sUwCoord.uwX <= 9*32 + VEHICLE_HEIGHT/2 &&
		1*32 <= g_sVehicle.sBobBody.sPos.sUwCoord.uwY &&
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY <= 3*32
	) {
		g_sVehicle.ubPayloadCurr = 0;
		g_sVehicle.ulScore += g_sVehicle.uwPayloadScore;
		g_sVehicle.uwPayloadScore = 0;
		hudSetCargo(0);
	}
}

static void vehicleProcessDrilling(void) {
	const UBYTE pTrackAnimOffs[DRILL_V_ANIM_LEN] = {
		0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0
	};
	if(g_sVehicle.ubDrillState == DRILL_STATE_ANIM_IN) {
		++g_sVehicle.ubDrillVAnimCnt;
		if(g_sVehicle.ubDrillVAnimCnt >= DRILL_V_ANIM_LEN - 1) {
			g_sVehicle.ubDrillState = DRILL_STATE_DRILLING;
		}
		else if(g_sVehicle.ubDrillVAnimCnt == 5) {
			bobNewSetBitMapOffset(&g_sVehicle.sBobTrack, TRACK_OFFSET_DRILL);
		}

		g_sVehicle.sBobBody.sPos.sUwCoord.uwX = fix16_to_int(g_sVehicle.fX);
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY = fix16_to_int(g_sVehicle.fY);
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY += pTrackAnimOffs[g_sVehicle.ubDrillVAnimCnt];
	}
	else if(g_sVehicle.ubDrillState == DRILL_STATE_DRILLING) {
		// Movement
		const fix16_t fDelta = (fix16_one*3)/4;
		UBYTE isDoneX = 0, isDoneY = 0;
		if(fix16_abs(g_sVehicle.fX - g_sVehicle.fDestX) <= fDelta) {
			g_sVehicle.fX = g_sVehicle.fDestX;
			isDoneX = 1;
		}
		else if (g_sVehicle.fX < g_sVehicle.fDestX) {
			g_sVehicle.fX += fDelta;
		}
		else {
			g_sVehicle.fX -= fDelta;
		}

		if(fix16_abs(g_sVehicle.fY - g_sVehicle.fDestY) <= fDelta) {
			g_sVehicle.fY = g_sVehicle.fDestY;
			isDoneY = 1;
		}
		else if (g_sVehicle.fY < g_sVehicle.fDestY) {
			g_sVehicle.fY += fDelta;
		}
		else {
			g_sVehicle.fY -= fDelta;
		}
		g_sVehicle.sBobBody.sPos.sUwCoord.uwX = fix16_to_int(g_sVehicle.fX);
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY = fix16_to_int(g_sVehicle.fY);
		// Pos for tool & track
		g_sVehicle.sBobTrack.sPos.ulYX = g_sVehicle.sBobBody.sPos.ulYX;
		g_sVehicle.sBobTrack.sPos.sUwCoord.uwY += VEHICLE_BODY_HEIGHT;

		if(isDoneX && isDoneY) {
			if(g_sVehicle.ubDrillDir == DRILL_DIR_H) {
				g_sVehicle.ubDrillDir = DRILL_DIR_NONE;
			}
			else {
				const UBYTE ubAdd = 4; // No grass past this point
				UWORD uwTileBottom = (g_sVehicle.sBobBody.sPos.sUwCoord.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
				UWORD uwCenterX = g_sVehicle.sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH / 2;
				UWORD uwTileCenter = uwCenterX >> 5;
				if(
					g_sVehicle.sSteer.bY > 0 && tileIsSolid(uwTileCenter, uwTileBottom)
				) {
					g_sVehicle.ubDrillDir = DRILL_DIR_V;
					g_sVehicle.fDestX = fix16_from_int(uwTileCenter << 5);
					g_sVehicle.fDestY = fix16_from_int(((uwTileBottom + 1) << 5) - VEHICLE_HEIGHT - 4);
					g_sVehicle.ubDrillState = DRILL_STATE_DRILLING;
				}
				else {
					g_sVehicle.ubDrillState = DRILL_STATE_ANIM_OUT;
				}
			}
			// Center is on tile to excavate
			UWORD uwTileX = (fix16_to_int(g_sVehicle.fX) + VEHICLE_WIDTH / 2) >> 5;
			UWORD uwTileY = (fix16_to_int(g_sVehicle.fY) + VEHICLE_HEIGHT / 2) >> 5;

			tileExcavate(&g_sVehicle, uwTileX, uwTileY);
			if(uwTileY == TILE_ROW_GRASS + 1) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileX);
			}
		}
		else {
			g_sVehicle.sBobTool.sPos.ulYX = g_sVehicle.sBobBody.sPos.ulYX;
			// Body shake
			WORD wX = g_sVehicle.sBobBody.sPos.sUwCoord.uwX + (ubRand() & 1);
			g_sVehicle.sBobBody.sPos.sUwCoord.uwX = CLAMP(
				wX, 0, 320 - VEHICLE_WIDTH
			);
			g_sVehicle.sBobBody.sPos.sUwCoord.uwY += ubRand() & 1;

			// Anim counter for Tool / track drill
			UBYTE ubAnim = 0;
			++g_sVehicle.ubToolAnimCnt;
			if(g_sVehicle.ubToolAnimCnt >= 4) {
				g_sVehicle.ubToolAnimCnt = 0;
			}
			else if(g_sVehicle.ubToolAnimCnt >= 2) {
				ubAnim = 1;
			}

			// Anim for Tool / track drill
			if(g_sVehicle.ubDrillDir == DRILL_DIR_H) {
				vehicleSetTool(TOOL_STATE_DRILL, ubAnim);
			}
			else {
				vehicleSetTool(TOOL_STATE_IDLE, 0);
				bobNewSetBitMapOffset(
					&g_sVehicle.sBobTrack,
					TRACK_OFFSET_DRILL + (ubAnim ?  VEHICLE_TRACK_HEIGHT : 0)
				);
			}
		}
	}
	else if(g_sVehicle.ubDrillState == DRILL_STATE_ANIM_OUT) {
		--g_sVehicle.ubDrillVAnimCnt;
		if(!g_sVehicle.ubDrillVAnimCnt) {
			g_sVehicle.ubDrillDir = DRILL_DIR_NONE;
		}
		else if(g_sVehicle.ubDrillVAnimCnt == 5) {
			bobNewSetBitMapOffset(&g_sVehicle.sBobTrack, TRACK_OFFSET_TRACK);
		}

		g_sVehicle.sBobBody.sPos.sUwCoord.uwX = fix16_to_int(g_sVehicle.fX);
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY = fix16_to_int(g_sVehicle.fY);
		g_sVehicle.sBobBody.sPos.sUwCoord.uwY += pTrackAnimOffs[g_sVehicle.ubDrillVAnimCnt];
	}

	bobNewPush(&g_sVehicle.sBobTrack);
	bobNewPush(&g_sVehicle.sBobBody);
	bobNewPush(&g_sVehicle.sBobTool);
}

void vehicleProcess(void) {
	if(g_sVehicle.ubDrillDir) {
		vehicleProcessDrilling();
	}
	else {
		vehicleProcessMovement();
	}
}
