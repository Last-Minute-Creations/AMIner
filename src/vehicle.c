/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include <ace/managers/rand.h>
#include "hud.h"
#include "game.h"
#include "tile.h"
#include "mineral.h"
#include "plan.h"
#include "color.h"

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

tBitMap *s_pBodyFrames[2], *s_pBodyMask;
tBitMap *s_pTrackFrames, *s_pTrackMask;
tBitMap *s_pJetFrames, *s_pJetMask;
tBitMap *s_pToolFrames[2], *s_pToolMask;

UBYTE s_pJetAnimOffsets[VEHICLE_TRACK_HEIGHT * 2 + 1] = {0,1,2,3,4,5,4,3,2,1,0};

void vehicleBitmapsCreate(void) {
	// Load gfx
	s_pBodyFrames[0] = bitmapCreateFromFile("data/drill.bm");
	s_pBodyFrames[1] = bitmapCreateFromFile("data/drill_2.bm");
	s_pBodyMask = bitmapCreateFromFile("data/drill_mask.bm");
	s_pTrackFrames = bitmapCreateFromFile("data/track.bm");
	s_pTrackMask = bitmapCreateFromFile("data/track_mask.bm");
	s_pJetFrames = bitmapCreateFromFile("data/jet.bm");
	s_pJetMask = bitmapCreateFromFile("data/jet_mask.bm");
	s_pToolFrames[0] = bitmapCreateFromFile("data/tool.bm");
	s_pToolFrames[1] = bitmapCreateFromFile("data/tool_2.bm");
	s_pToolMask = bitmapCreateFromFile("data/tool_mask.bm");
}

void vehicleBitmapsDestroy(void) {
	bitmapDestroy(s_pBodyFrames[0]);
	bitmapDestroy(s_pBodyFrames[1]);
	bitmapDestroy(s_pBodyMask);
	bitmapDestroy(s_pTrackFrames);
	bitmapDestroy(s_pTrackMask);
	bitmapDestroy(s_pJetFrames);
	bitmapDestroy(s_pJetMask);
	bitmapDestroy(s_pToolFrames[0]);
	bitmapDestroy(s_pToolFrames[1]);
	bitmapDestroy(s_pToolMask);
}

void vehicleReset(tVehicle *pVehicle) {
	// Initial values
	pVehicle->ubCargoCurr = 0;
	pVehicle->ubCargoMax = 50;
	pVehicle->uwCargoScore = 0;
	pVehicle->ulCash = 0;
	pVehicle->uwFuelMax = 1000;
	pVehicle->uwFuelCurr = 1000;

	pVehicle->ubDrillDir = 0;

	pVehicle->ubTrackAnimCnt = 0;
	pVehicle->ubTrackFrame = 0;
	pVehicle->ubBodyShakeCnt = 0;
	pVehicle->ubJetShowFrame = 0;
	pVehicle->ubJetAnimFrame = 0;
	pVehicle->ubJetAnimCnt = 0;
	pVehicle->ubToolAnimCnt = 0;
	pVehicle->ubDrillVAnimCnt = 0;

	pVehicle->sBobBody.sPos.ulYX = 0;

	pVehicle->fY = fix16_from_int(32);
	pVehicle->fDx = 0;
	pVehicle->fDy = 0;
	if(pVehicle->ubPlayerIdx == PLAYER_1) {
		pVehicle->fX = fix16_from_int(64);
		vehicleMove(pVehicle, 1, 0);
	}
	else {
		pVehicle->fX = fix16_from_int(320-32);
		vehicleMove(pVehicle, -1, 0);
	}
}

void vehicleCreate(tVehicle *pVehicle, UBYTE ubIdx) {
	logBlockBegin("vehicleCreate(pVehicle: %p, ubIdx: %hhu)", pVehicle, ubIdx);

	// Setup bobs
  bobNewInit(
		&pVehicle->sBobBody, VEHICLE_WIDTH, VEHICLE_BODY_HEIGHT, 1,
		s_pBodyFrames[ubIdx], s_pBodyMask, 0, 0
	);
	bobNewInit(
		&pVehicle->sBobTrack, VEHICLE_WIDTH, VEHICLE_TRACK_HEIGHT, 1,
		s_pTrackFrames, s_pTrackMask, 0, 0
	);
	bobNewInit(
		&pVehicle->sBobJet, VEHICLE_WIDTH, VEHICLE_FLAME_HEIGHT, 1,
		s_pJetFrames, s_pJetMask, 0, 0
	);
	bobNewInit(
		&pVehicle->sBobTool, VEHICLE_TOOL_WIDTH, VEHICLE_TOOL_HEIGHT, 1,
		s_pToolFrames[ubIdx], s_pToolMask, 0, 0
	);
	pVehicle->ubPlayerIdx = ubIdx;

	vehicleReset(pVehicle);

	textBobCreate(&pVehicle->sTextBob, g_pFont, "Checkpoint! +1000");
	logBlockEnd("vehicleCreate()");
}

void vehicleDestroy(tVehicle *pVehicle) {
	textBobDestroy(&pVehicle->sTextBob);
}

void vehicleMove(tVehicle *pVehicle, BYTE bDirX, BYTE bDirY) {
	// Always register steer requests so that vehicle can continuously drill down
	pVehicle->sSteer.bX = bDirX;
	pVehicle->sSteer.bY = bDirY;

	// No vehicle rotating when drilling
	if(pVehicle->ubDrillDir != DRILL_DIR_NONE) {
		return;
	}
	if(bDirX > 0) {
		bobNewSetBitMapOffset(&pVehicle->sBobBody, 0);
	}
	else if(bDirX < 0) {
		bobNewSetBitMapOffset(&pVehicle->sBobBody, VEHICLE_BODY_HEIGHT);
	}
}

static inline void vehicleSetTool(
	tVehicle *pVehicle, tToolState eToolState, UBYTE ubFrame
) {
	if(eToolState == TOOL_STATE_IDLE) {
		if(!pVehicle->sBobBody.uwOffsetY) {
			// Facing right
			pVehicle->sBobTool.sPos.sUwCoord.uwX += 24;
		}
		// Vertical drill anim
		bobNewSetBitMapOffset(&pVehicle->sBobTool, 0);
	}
	else { // if(eToolState == TOOL_STATE_DRILL)
		pVehicle->sBobTool.sPos.sUwCoord.uwY += 3;
		if(!pVehicle->sBobBody.uwOffsetY) {
			// Facing right
			pVehicle->sBobTool.sPos.sUwCoord.uwX += 24+3;
			bobNewSetBitMapOffset(
				&pVehicle->sBobTool, (1 + ubFrame) * VEHICLE_TOOL_HEIGHT
			);
		}
		else {
			// Facing left
			pVehicle->sBobTool.sPos.sUwCoord.uwX += -13+3;
			bobNewSetBitMapOffset(
				&pVehicle->sBobTool, (3 + ubFrame) * VEHICLE_TOOL_HEIGHT
			);
		}
	}
}

static inline UBYTE vehicleStartDrilling(
	tVehicle *pVehicle, UWORD uwTileX, UWORD uwTileY, UBYTE ubDrillDir
) {
	static UBYTE ubCooldown = 0;
	if(pVehicle->uwFuelCurr < 30) {
		if(!ubCooldown) {
			textBobSet(
				&pVehicle->sTextBob, "No fuel!", 6,
				pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2,
				pVehicle->sBobBody.sPos.sUwCoord.uwY,
				pVehicle->sBobBody.sPos.sUwCoord.uwY - 32, 1
			);
			audioPlay(
				AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
				g_pSampleTeleport, AUDIO_VOLUME_MAX, 1
			);
			ubCooldown = 25;
		}
		else {
			--ubCooldown;
		}
		return 0;
	}

	// Check if other player drills the same tile
	const tVehicle * const pOther = &g_pVehicles[!pVehicle->ubPlayerIdx];
	if(
		pOther->ubDrillDir &&
		pOther->uwDrillTileX == uwTileX && pOther->uwDrillTileY == uwTileY
	) {
		return 0;
	}
	pVehicle->uwDrillTileX = uwTileX;
	pVehicle->uwDrillTileY = uwTileY;

	if(ubDrillDir == DRILL_DIR_V) {
		pVehicle->ubDrillState = DRILL_STATE_ANIM_IN;
	}
	else {
		pVehicle->ubDrillState = DRILL_STATE_DRILLING;
	}
	pVehicle->ubDrillDir = ubDrillDir;
	pVehicle->fDestX = fix16_from_int(uwTileX << 5);
	pVehicle->fDestY = fix16_from_int(((uwTileY + 1) << 5) - VEHICLE_HEIGHT - 4);
	pVehicle->fDx = 0;
	pVehicle->fDy = 0;
	audioPlay(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx, g_pSampleDrill, AUDIO_VOLUME_MAX, -1);

	if(!g_isChallenge) {
		pVehicle->uwFuelCurr -= 30;
	}
	return 1;
}

static WORD vehicleRestock(tVehicle *pVehicle) {
	pVehicle->ubCargoCurr = 0;
	pVehicle->ulCash += pVehicle->uwCargoScore;
	WORD wScoreNow = pVehicle->uwCargoScore;
	pVehicle->uwCargoScore = 0;
	hudSetCargo(pVehicle->ubPlayerIdx, 0);
	const UBYTE ubFuelPrice = 5;
	const UBYTE ubFuelDiv = 100;
	UWORD uwRefuelUnits = MIN(
		(pVehicle->ulCash / ubFuelPrice),
		(UWORD)(pVehicle->uwFuelMax - pVehicle->uwFuelCurr + ubFuelDiv-1) / ubFuelDiv
	);
	pVehicle->uwFuelCurr = MIN(
		pVehicle->uwFuelCurr + uwRefuelUnits * ubFuelDiv, pVehicle->uwFuelMax
	);
	pVehicle->ulCash -= uwRefuelUnits * ubFuelPrice;
	wScoreNow -= uwRefuelUnits * ubFuelPrice;

	audioPlay(
		AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
		g_pSampleOre, AUDIO_VOLUME_MAX, 1
	);

	return wScoreNow;
}

static void vehicleExcavateTile(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
	// Load mineral to vehicle
	static UBYTE wasPlanFulfilled = 0; // FIXME this may work incorrectly when game restarts
	static const char * const szMessageFull = "Cargo full!";
	static const char * const szMessagePlanDone = "Plan done!";
	UBYTE ubTile = g_pMainBuffer->pTileData[uwX][uwY];
	if(g_pTileDefs[ubTile].szMsg) {
		UBYTE ubMineralType = g_pTileDefs[ubTile].ubMineral;
		const tMineralDef *pMineral = &g_pMinerals[ubMineralType];
		UBYTE ubSlots = g_pTileDefs[ubTile].ubSlots;
		ubSlots = MIN(ubSlots, pVehicle->ubCargoMax - pVehicle->ubCargoCurr);
		pVehicle->uwCargoScore += pMineral->ubReward * ubSlots;
		pVehicle->ubCargoCurr += ubSlots;
		planAddMinerals(ubMineralType, ubSlots);

		hudSetCargo(pVehicle->ubPlayerIdx, pVehicle->ubCargoCurr);
		const char *szMessage;
		UBYTE ubColor;
		if(pVehicle->ubCargoCurr == pVehicle->ubCargoMax) {
			szMessage = szMessageFull;
			ubColor = COLOR_REDEST;
			audioPlay(
				AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
				g_pSampleTeleport, AUDIO_VOLUME_MAX, 1
			);
		}
		else {
			UBYTE isPlanFulfilled = planIsFulfilled();
			if(isPlanFulfilled && !wasPlanFulfilled) {
				szMessage = szMessagePlanDone;
			}
			else {
				szMessage = g_pTileDefs[ubTile].szMsg;
			}
			wasPlanFulfilled = isPlanFulfilled;
			ubColor = pMineral->ubTitleColor;
			audioPlay(
				AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
				g_pSampleOre, AUDIO_VOLUME_MAX, 1
			);
		}
		textBobSet(
			&pVehicle->sTextBob, szMessage, ubColor,
			pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2,
			pVehicle->sBobBody.sPos.sUwCoord.uwY,
			pVehicle->sBobBody.sPos.sUwCoord.uwY - 32, 1
		);
	}

	if(g_isChallenge) {
		if(ubTile == TILE_CHECKPOINT) {
			textBobSetText(
				&pVehicle->sTextBob, "Checkpoint! %+hd", vehicleRestock(pVehicle)
			);
			textBobSetColor(&pVehicle->sTextBob, COLOR_GREEN);
			textBobSetPos(
				&pVehicle->sTextBob,
				pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2,
				pVehicle->sBobBody.sPos.sUwCoord.uwY,
				pVehicle->sBobBody.sPos.sUwCoord.uwY - 32, 1
			);
		}
		else if(ubTile == TILE_FINISH) {
			vehicleRestock(pVehicle);
			gameChallengeEnd();
		}
	}

	tileExcavate(uwX, uwY);
}

static void vehicleProcessMovement(tVehicle *pVehicle) {
	UBYTE isOnGround = 0;
	const fix16_t fMaxDx = 4 * fix16_one;
	const fix16_t fAccX = fix16_one / 8;
	const fix16_t fFrictX = fix16_one / 12;

	if(
		g_isChallenge &&
		fix16_to_int(pVehicle->fY) < g_pMainBuffer->pCamera->uPos.sUwCoord.uwY
	) {
		UWORD uwTileY = (
			g_pMainBuffer->pCamera->uPos.sUwCoord.uwY +
			g_pMainBuffer->pCamera->sCommon.pVPort->uwHeight / 2
		) / 32;
		UWORD uwTileX = fix16_to_int(pVehicle->fX)/32;
		if(tileIsSolid(uwTileX, uwTileY)) {
			tileExcavate(uwTileX, uwTileY);
		}
		pVehicle->fY = fix16_from_int(uwTileY*32);
		pVehicle->fDy = fix16_from_int(-1); // HACK HACK HACK
		audioPlay(
			AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
			g_pSampleTeleport, AUDIO_VOLUME_MAX, 1
		);

		if(uwTileY >= TILE_ROW_CHALLENGE_FINISH) {
			gameChallengeEnd();
		}
	}
	if(pVehicle->sSteer.bX) {
		if(pVehicle->sSteer.bX > 0) {
			pVehicle->fDx = MIN(pVehicle->fDx + fAccX, fMaxDx);
		}
		else {
			pVehicle->fDx = MAX(pVehicle->fDx - fAccX, -fMaxDx);
		}
	}
	else {
		if(pVehicle->fDx > 0) {
			pVehicle->fDx = MAX(0, pVehicle->fDx - fFrictX);
		}
		else {
			pVehicle->fDx = MIN(0, pVehicle->fDx + fFrictX);
		}
	}

	// Limit X movement
	const fix16_t fMaxPosX = fix16_one * (11*32 - VEHICLE_WIDTH);
	pVehicle->fX = CLAMP(pVehicle->fX + pVehicle->fDx, fix16_from_int(32), fMaxPosX);
	pVehicle->sBobBody.sPos.sUwCoord.uwX = fix16_to_int(pVehicle->fX);
	UBYTE ubAdd = (pVehicle->sBobBody.sPos.sUwCoord.uwY > (1 + TILE_ROW_GRASS) * 32) ? 4 : 8;
	UBYTE ubHalfWidth = 12;

	UWORD uwCenterX = pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH / 2;
	UWORD uwTileBottom = (pVehicle->sBobBody.sPos.sUwCoord.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
	UWORD uwTileMid = (pVehicle->sBobBody.sPos.sUwCoord.uwY + VEHICLE_HEIGHT / 2) >> 5;
	UWORD uwTileCenter = uwCenterX >> 5;
	UWORD uwTileLeft = (uwCenterX - ubHalfWidth) >> 5;
	UWORD uwTileRight = (uwCenterX + ubHalfWidth) >> 5;

	if(tileIsSolid(uwTileLeft, uwTileMid)) {
		pVehicle->fX = fix16_from_int(((uwTileLeft+1) << 5) - VEHICLE_WIDTH / 2 + ubHalfWidth);
		pVehicle->fDx = 0;
	}
	else if(tileIsSolid(uwTileRight, uwTileMid)) {
		pVehicle->fX = fix16_from_int((uwTileRight << 5) - VEHICLE_WIDTH / 2 - ubHalfWidth);
		pVehicle->fDx = 0;
	}

	const fix16_t fMaxFlightDy = 3 * fix16_one;
	const fix16_t fAccFlight = fix16_one / 12;
	const fix16_t fMaxGravDy = 4 * fix16_one;
	const fix16_t fAccGrav = fix16_one / 8;
	if(pVehicle->sSteer.bY < 0) {
		if(pVehicle->ubJetShowFrame == 10) {
			pVehicle->fDy = MAX(-fMaxFlightDy, pVehicle->fDy - fAccFlight);
		}
		else {
			++pVehicle->ubJetShowFrame;
		}
	}
	else {
		if(pVehicle->ubJetShowFrame) {
			--pVehicle->ubJetShowFrame;
		}
		pVehicle->fDy = MIN(fMaxGravDy, pVehicle->fDy + fAccGrav);
	}

	if(pVehicle->fDy < 0) {
		UWORD uwTileTop = (fix16_to_int(pVehicle->fY) - 1) >> 5;
		// Flying
		pVehicle->fY = MAX(0, pVehicle->fY + pVehicle->fDy);
		if(tileIsSolid(uwTileCenter, uwTileTop)) {
			pVehicle->fY = fix16_from_int((uwTileTop+1) << 5);
			pVehicle->fDy = 0;
		}
	}
	else {
		if(!tileIsSolid(uwTileCenter, uwTileBottom)) {
			// Gravity
			pVehicle->fY += pVehicle->fDy;
		}
		else {
			// Collision with ground
			isOnGround = 1;
			pVehicle->fY = fix16_from_int((uwTileBottom << 5) - VEHICLE_HEIGHT - ubAdd);
			pVehicle->fDy = 0;
		}
	}

	// Update track bob
	pVehicle->sBobBody.sPos.sUwCoord.uwY = fix16_to_int(pVehicle->fY);
	pVehicle->sBobTrack.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
	pVehicle->sBobTrack.sPos.sUwCoord.uwY += VEHICLE_BODY_HEIGHT;
	if(pVehicle->ubJetShowFrame == 0) {
		// Jet hidden
		if(pVehicle->fDx) {
			++pVehicle->ubTrackAnimCnt;
			if(pVehicle->ubTrackAnimCnt >= (5 - fix16_to_int(fix16_abs(pVehicle->fDx)))) {
				pVehicle->ubTrackFrame = !pVehicle->ubTrackFrame;
				bobNewSetBitMapOffset(
					&pVehicle->sBobTrack, pVehicle->ubTrackFrame * VEHICLE_TRACK_HEIGHT
				);
				pVehicle->ubTrackAnimCnt = 0;
			}
		}

		++pVehicle->ubBodyShakeCnt;
		UBYTE ubShakeSpeed = (pVehicle->fDx ? 5 : 10);
		if(pVehicle->ubBodyShakeCnt >= 2 * ubShakeSpeed) {
			pVehicle->ubBodyShakeCnt = 0;
		}
		if(pVehicle->ubBodyShakeCnt >= ubShakeSpeed) {
			pVehicle->sBobBody.sPos.sUwCoord.uwY += 1;
		}
	}
	else {
		pVehicle->sBobBody.sPos.sUwCoord.uwY += s_pJetAnimOffsets[pVehicle->ubJetShowFrame];
		if(pVehicle->ubJetShowFrame == 5) {
			bobNewSetBitMapOffset(
				&pVehicle->sBobTrack, pVehicle->sSteer.bY ? 2*VEHICLE_TRACK_HEIGHT : 0
			);
		}
		else if(pVehicle->ubJetShowFrame == 10) {
			// Update jet pos
			pVehicle->ubJetAnimCnt = (pVehicle->ubJetAnimCnt + 1) & 15;
			bobNewSetBitMapOffset(
				&pVehicle->sBobJet, VEHICLE_FLAME_HEIGHT * (pVehicle->ubJetAnimCnt / 4)
			);
			pVehicle->sBobJet.sPos.ulYX = pVehicle->sBobTrack.sPos.ulYX;
			pVehicle->sBobJet.sPos.sUwCoord.uwY += VEHICLE_TRACK_JET_HEIGHT;
			bobNewPush(&pVehicle->sBobJet);
		}
	}
	bobNewPush(&pVehicle->sBobTrack);

	// Drilling
	if(isOnGround) {
		if(pVehicle->sSteer.bX > 0 && tileIsDrillable(uwTileRight, uwTileMid)) {
			vehicleStartDrilling(pVehicle, uwTileRight, uwTileMid, DRILL_DIR_H);
		}
		else if(pVehicle->sSteer.bX < 0 && tileIsDrillable(uwTileLeft, uwTileMid)) {
			vehicleStartDrilling(pVehicle, uwTileLeft, uwTileMid, DRILL_DIR_H);
		}
		else if(
			pVehicle->sSteer.bY > 0 && tileIsDrillable(uwTileCenter, uwTileBottom)
		) {
			vehicleStartDrilling(pVehicle, uwTileCenter, uwTileBottom, DRILL_DIR_V);
		}
	}
	bobNewPush(&pVehicle->sBobBody);

	// Tool
	pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
	vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
	bobNewPush(&pVehicle->sBobTool);

	if(
		4*32 <= pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2 &&
		pVehicle->sBobBody.sPos.sUwCoord.uwX <= 6*32 + VEHICLE_HEIGHT/2 &&
		(TILE_ROW_GRASS - 2) * 32 <= pVehicle->sBobBody.sPos.sUwCoord.uwY &&
		pVehicle->sBobBody.sPos.sUwCoord.uwY <= (TILE_ROW_GRASS+1) * 32 &&
		(pVehicle->ubCargoCurr  || (pVehicle->uwFuelMax - pVehicle->uwFuelCurr > 100))
	) {
		WORD wScoreNow = vehicleRestock(pVehicle);
		UBYTE ubColor = 12;
		if(wScoreNow < 0) {
			ubColor = 6;
		}
		textBobSetText(&pVehicle->sTextBob, "%+hd\x1F", wScoreNow);
		textBobSetColor(&pVehicle->sTextBob, ubColor);
		textBobSetPos(
			&pVehicle->sTextBob,
			pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2,
			pVehicle->sBobBody.sPos.sUwCoord.uwY,
			pVehicle->sBobBody.sPos.sUwCoord.uwY - 24, 0
		);
	}
}

static void vehicleProcessDrilling(tVehicle *pVehicle) {
	const UBYTE pTrackAnimOffs[DRILL_V_ANIM_LEN] = {
		0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0
	};
	if(
		pVehicle->ubDrillState == DRILL_STATE_ANIM_IN ||
		pVehicle->ubDrillState == DRILL_STATE_ANIM_OUT
	) {
		if(pVehicle->ubDrillState == DRILL_STATE_ANIM_IN) {
			++pVehicle->ubDrillVAnimCnt;
			if(pVehicle->ubDrillVAnimCnt >= DRILL_V_ANIM_LEN - 1) {
				pVehicle->ubDrillState = DRILL_STATE_DRILLING;
			}
			else if(pVehicle->ubDrillVAnimCnt == 5) {
				bobNewSetBitMapOffset(&pVehicle->sBobTrack, TRACK_OFFSET_DRILL);
			}
		}
		else {
			--pVehicle->ubDrillVAnimCnt;
			if(!pVehicle->ubDrillVAnimCnt) {
				pVehicle->ubDrillDir = DRILL_DIR_NONE;
			}
			else if(pVehicle->ubDrillVAnimCnt == 5) {
				bobNewSetBitMapOffset(&pVehicle->sBobTrack, TRACK_OFFSET_TRACK);
			}
		}

		pVehicle->sBobBody.sPos.sUwCoord.uwX = fix16_to_int(pVehicle->fX);
		pVehicle->sBobBody.sPos.sUwCoord.uwY = fix16_to_int(pVehicle->fY);
		pVehicle->sBobBody.sPos.sUwCoord.uwY += pTrackAnimOffs[pVehicle->ubDrillVAnimCnt];
		pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
		vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
	}
	else if(pVehicle->ubDrillState == DRILL_STATE_DRILLING) {
		// Movement
		const fix16_t fDelta = (fix16_one*3)/4;
		UBYTE isDoneX = 0, isDoneY = 0;
		if(fix16_abs(pVehicle->fX - pVehicle->fDestX) <= fDelta) {
			pVehicle->fX = pVehicle->fDestX;
			isDoneX = 1;
		}
		else if (pVehicle->fX < pVehicle->fDestX) {
			pVehicle->fX += fDelta;
		}
		else {
			pVehicle->fX -= fDelta;
		}

		if(fix16_abs(pVehicle->fY - pVehicle->fDestY) <= fDelta) {
			pVehicle->fY = pVehicle->fDestY;
			isDoneY = 1;
		}
		else if (pVehicle->fY < pVehicle->fDestY) {
			pVehicle->fY += fDelta;
		}
		else {
			pVehicle->fY -= fDelta;
		}
		pVehicle->sBobBody.sPos.sUwCoord.uwX = fix16_to_int(pVehicle->fX);
		pVehicle->sBobBody.sPos.sUwCoord.uwY = fix16_to_int(pVehicle->fY);
		// Pos for tool & track
		pVehicle->sBobTrack.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
		pVehicle->sBobTrack.sPos.sUwCoord.uwY += VEHICLE_BODY_HEIGHT;

		if(isDoneX && isDoneY) {
			if(pVehicle->ubDrillDir == DRILL_DIR_H) {
				pVehicle->ubDrillDir = DRILL_DIR_NONE;
				audioStop(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx);
			}
			else {
				const UBYTE ubAdd = 4; // No grass past this point
				UWORD uwTileBottom = (pVehicle->sBobBody.sPos.sUwCoord.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
				UWORD uwCenterX = pVehicle->sBobBody.sPos.sUwCoord.uwX + VEHICLE_WIDTH / 2;
				UWORD uwTileCenter = uwCenterX >> 5;
				if(
					pVehicle->sSteer.bY > 0 && tileIsDrillable(uwTileCenter, uwTileBottom) &&
					vehicleStartDrilling(pVehicle, uwTileCenter, uwTileBottom, DRILL_DIR_V)
				) {
					pVehicle->ubDrillState = DRILL_STATE_DRILLING;
				}
				else {
					pVehicle->ubDrillState = DRILL_STATE_ANIM_OUT;
					audioStop(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx);
				}
			}
			// Center is on tile to excavate
			UWORD uwTileX = (fix16_to_int(pVehicle->fX) + VEHICLE_WIDTH / 2) >> 5;
			UWORD uwTileY = (fix16_to_int(pVehicle->fY) + VEHICLE_HEIGHT / 2) >> 5;

			vehicleExcavateTile(pVehicle, uwTileX, uwTileY);
			if(uwTileY == TILE_ROW_GRASS + 1) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileX);
			}
		}
		else {
			pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
			// Body shake
			pVehicle->sBobBody.sPos.sUwCoord.uwX += ubRand() & 1;
			pVehicle->sBobBody.sPos.sUwCoord.uwY += ubRand() & 1;

			// Anim counter for Tool / track drill
			UBYTE ubAnim = 0;
			++pVehicle->ubToolAnimCnt;
			if(pVehicle->ubToolAnimCnt >= 4) {
				pVehicle->ubToolAnimCnt = 0;
			}
			else if(pVehicle->ubToolAnimCnt >= 2) {
				ubAnim = 1;
			}

			// Anim for Tool / track drill
			if(pVehicle->ubDrillDir == DRILL_DIR_H) {
				vehicleSetTool(pVehicle, TOOL_STATE_DRILL, ubAnim);
			}
			else {
				vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
				bobNewSetBitMapOffset(
					&pVehicle->sBobTrack,
					TRACK_OFFSET_DRILL + (ubAnim ?  VEHICLE_TRACK_HEIGHT : 0)
				);
			}
		}
	}

	bobNewPush(&pVehicle->sBobTrack);
	bobNewPush(&pVehicle->sBobBody);
	bobNewPush(&pVehicle->sBobTool);
}

void vehicleProcessText(void) {
	// MUST BE BEFORE ANY BOB PUSH
	static UBYTE ubPlayer = 0;
	textBobUpdate(&g_pVehicles[ubPlayer].sTextBob);
	ubPlayer = !ubPlayer;
}

void vehicleProcess(tVehicle *pVehicle) {
	if(pVehicle->ubDrillDir) {
		vehicleProcessDrilling(pVehicle);
	}
	else {
		vehicleProcessMovement(pVehicle);
	}
	hudSetFuel(pVehicle->ubPlayerIdx, pVehicle->uwFuelCurr);
	textBobAnimate(&pVehicle->sTextBob);
	hudSetDepth(pVehicle->ubPlayerIdx, MAX(
		0, fix16_to_int(pVehicle->fY) + VEHICLE_HEIGHT - (TILE_ROW_GRASS+1)*32
	));
	hudSetScore(pVehicle->ubPlayerIdx, pVehicle->ulCash);
}
