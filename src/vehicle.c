/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include <ace/managers/rand.h>
#include "hud.h"
#include "game.h"
#include "tile.h"
#include "warehouse.h"
#include "color.h"

#define VEHICLE_BODY_HEIGHT 20
#define VEHICLE_DESTRUCTION_FRAMES 4
#define VEHICLE_TOOL_ANIM_FRAMES 2
#define VEHICLE_TRACK_HEIGHT 7
#define VEHICLE_TRACK_DRILL_HEIGHT 7
#define VEHICLE_TRACK_JET_HEIGHT 5
#define VEHICLE_FLAME_HEIGHT 21
#define VEHICLE_TOOL_WIDTH 16
#define VEHICLE_TOOL_HEIGHT 16
#define VEHICLE_WRECK_WIDTH 48
#define VEHICLE_WRECK_HEIGHT 21
#define VEHICLE_SMOKE_WIDTH 48
#define VEHICLE_SMOKE_FRAME_HEIGHT 29
#define VEHICLE_SMOKE_ANIM_HEIGHT 42
#define VEHICLE_SMOKE_FRAMES 12

#define TRACK_OFFSET_TRACK 0
#define TRACK_OFFSET_JET 28
#define TRACK_OFFSET_DRILL 35
#define DRILL_V_ANIM_LEN (VEHICLE_TRACK_HEIGHT + VEHICLE_TRACK_DRILL_HEIGHT - 1)

tBitMap *s_pBodyFrames[2], *s_pBodyMask;
tBitMap *s_pTrackFrames, *s_pTrackMask;
tBitMap *s_pJetFrames, *s_pJetMask;
tBitMap *s_pToolFrames[2], *s_pToolMask;
tBitMap *s_pWreckFrames[2], *s_pWreckMask;
tBitMap *s_pSmokeFrames, *s_pSmokeMask;

UBYTE s_pJetAnimOffsets[VEHICLE_TRACK_HEIGHT * 2 + 1] = {0,1,2,3,4,5,4,3,2,1,0};

void vehicleBitmapsCreate(void) {
	// Load gfx
	s_pBodyFrames[0] = bitmapCreateFromFile("data/drill.bm", 0);
	s_pBodyFrames[1] = bitmapCreateFromFile("data/drill_2.bm", 0);
	s_pBodyMask = bitmapCreateFromFile("data/drill_mask.bm", 0);

	s_pTrackFrames = bitmapCreateFromFile("data/track.bm", 0);
	s_pTrackMask = bitmapCreateFromFile("data/track_mask.bm", 0);

	s_pJetFrames = bitmapCreateFromFile("data/jet.bm", 0);
	s_pJetMask = bitmapCreateFromFile("data/jet_mask.bm", 0);

	s_pToolFrames[0] = bitmapCreateFromFile("data/tool.bm", 0);
	s_pToolFrames[1] = bitmapCreateFromFile("data/tool_2.bm", 0);
	s_pToolMask = bitmapCreateFromFile("data/tool_mask.bm", 0);

	s_pWreckFrames[0] = bitmapCreateFromFile("data/wreck.bm", 0);
	s_pWreckFrames[1] = bitmapCreateFromFile("data/wreck_2.bm", 0);
	s_pWreckMask = bitmapCreateFromFile("data/wreck_mask.bm", 0);

	s_pSmokeFrames = bitmapCreateFromFile("data/smoke.bm", 0);
	s_pSmokeMask = bitmapCreateFromFile("data/smoke_mask.bm", 0);
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

	bitmapDestroy(s_pWreckFrames[0]);
	bitmapDestroy(s_pWreckFrames[1]);
	bitmapDestroy(s_pWreckMask);

	bitmapDestroy(s_pSmokeFrames);
	bitmapDestroy(s_pSmokeMask);
}

void vehicleResetPos(tVehicle *pVehicle) {
	pVehicle->ubDrillDir = 0;

	pVehicle->ubTrackAnimCnt = 0;
	pVehicle->ubTrackFrame = 0;
	pVehicle->ubBodyShakeCnt = 0;
	pVehicle->ubJetShowFrame = 0;
	pVehicle->ubJetAnimFrame = 0;
	pVehicle->ubJetAnimCnt = 0;
	pVehicle->ubToolAnimCnt = 0;
	pVehicle->ubDrillVAnimCnt = 0;
	pVehicle->ubSmokeAnimFrame = 0;
	pVehicle->ubSmokeAnimCnt = 0;

	pVehicle->sBobBody.sPos.ulYX = 0;

	pVehicle->fY = fix16_from_int((TILE_ROW_BASE_DIRT - 2) * 32);
	pVehicle->fDx = 0;
	pVehicle->fDy = 0;
	if(pVehicle->ubPlayerIdx == PLAYER_1) {
		pVehicle->fX = fix16_from_int(g_isChallenge ? 0 : 96);
		vehicleMove(pVehicle, 1, 0);
	}
	else {
		pVehicle->fX = fix16_from_int(g_isChallenge ? 96 : 320-64);
		vehicleMove(pVehicle, -1, 0);
	}
}

void vehicleUpdateBodyBob(tVehicle *pVehicle) {
	UBYTE ubFrameOffs = VEHICLE_BODY_HEIGHT * pVehicle->ubDestructionState;
	if(!pVehicle->isFacingRight) {
		ubFrameOffs += VEHICLE_BODY_HEIGHT * VEHICLE_DESTRUCTION_FRAMES;
	}
	bobNewSetBitMapOffset(&pVehicle->sBobBody, ubFrameOffs);
}

static UBYTE s_ubBebCountdown = 0;

static void vehicleCrash(tVehicle *pVehicle) {
	// Calculate pos for bobs
	pVehicle->sBobWreck.sPos.uwX = fix16_to_int(pVehicle->fX);
	pVehicle->sBobWreck.sPos.uwY = (
		fix16_to_int(pVehicle->fY) + VEHICLE_BODY_HEIGHT +
		VEHICLE_TRACK_HEIGHT - VEHICLE_WRECK_HEIGHT
	);
	pVehicle->sBobSmoke.sPos.uwX = fix16_to_int(pVehicle->fX);
	pVehicle->sBobSmoke.sPos.uwY = (
		fix16_to_int(pVehicle->fY) + VEHICLE_BODY_HEIGHT +
		VEHICLE_TRACK_HEIGHT - VEHICLE_SMOKE_ANIM_HEIGHT
	);

	s_ubBebCountdown = 200;
}

static void vehicleHullDamage(tVehicle *pVehicle, UWORD uwDmg) {
	pVehicle->wHullCurr = MAX(0, pVehicle->wHullCurr - uwDmg);
	if(pVehicle->wHullCurr == 0) {
		vehicleCrash(pVehicle);
	}
	else {
		pVehicle->ubDestructionState = (
			(((UWORD)pVehicle->wHullCurr - 1) * VEHICLE_DESTRUCTION_FRAMES) /
			(UWORD)pVehicle->wHullMax
		);
		vehicleUpdateBodyBob(pVehicle);
	}
	hudSetHull(pVehicle->ubPlayerIdx, pVehicle->wHullCurr, pVehicle->wHullMax);
}

static void vehicleHullRepair(tVehicle *pVehicle) {
	pVehicle->wHullCurr = pVehicle->wHullMax;
	pVehicle->ubDestructionState = VEHICLE_DESTRUCTION_FRAMES - 1;
	hudSetHull(pVehicle->ubPlayerIdx, pVehicle->wHullCurr, pVehicle->wHullMax);
}

void vehicleRespawn(tVehicle *pVehicle) {
	pVehicle->ubCargoCurr = 0;
	pVehicle->uwCargoScore = 0;
	pVehicle->uwDrillCurr = pVehicle->uwDrillMax;
	vehicleHullRepair(pVehicle);
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		pVehicle->pStock[i] = 0;
	}
	vehicleResetPos(pVehicle);
}

void vehicleReset(tVehicle *pVehicle) {
	// Initial values
	pVehicle->ubCargoMax = 50;
	pVehicle->lCash = 0;
	pVehicle->uwDrillMax = 1000;
	pVehicle->wHullMax = 100;
	vehicleRespawn(pVehicle);
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
	bobNewInit(
		&pVehicle->sBobWreck, VEHICLE_WRECK_WIDTH, VEHICLE_WRECK_HEIGHT, 1,
		s_pWreckFrames[ubIdx], s_pWreckMask, 0, 0
	);
	bobNewInit(
		&pVehicle->sBobSmoke, VEHICLE_SMOKE_WIDTH, VEHICLE_SMOKE_FRAME_HEIGHT, 1,
		s_pSmokeFrames, s_pSmokeMask, 0, 0
	);
	pVehicle->ubPlayerIdx = ubIdx;

	vehicleReset(pVehicle);

	textBobCreate(&pVehicle->sTextBob, g_pFont, "Checkpoint! +1000\x1F");
	logBlockEnd("vehicleCreate()");
}

void vehicleDestroy(tVehicle *pVehicle) {
	textBobDestroy(&pVehicle->sTextBob);
}

UBYTE vehicleIsNearShop(const tVehicle *pVehicle) {
	UWORD uwCenterX = pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2;
	UWORD uwY = pVehicle->sBobBody.sPos.uwY;
	UBYTE isNearInBase0 = (
		7*32 <= uwCenterX && uwCenterX <= 9*32 &&
		(TILE_ROW_BASE_DIRT - 2) * 32 <= uwY && uwY <= (TILE_ROW_BASE_DIRT + 1) * 32
	);

	UBYTE isNearInBase1 = (
		1*32 <= uwCenterX && uwCenterX <= 3*32 &&
		(100 + 6) * 32 <= uwY && uwY <= (100 + 8) * 32
	);

	return isNearInBase0 || isNearInBase1;
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
		pVehicle->isFacingRight = 1;
	}
	else if(bDirX < 0) {
		pVehicle->isFacingRight = 0;
	}
	vehicleUpdateBodyBob(pVehicle);
}

static inline void vehicleSetTool(
	tVehicle *pVehicle, tToolState eToolState, UBYTE ubFrame
) {
	pVehicle->sBobTool.sPos.uwY -= 3;
	UBYTE ubFrameOffs;
	if(eToolState == TOOL_STATE_IDLE) {
		ubFrameOffs = 0;
	}
	else { // if(eToolState == TOOL_STATE_DRILL)
		ubFrameOffs = ubFrame ? VEHICLE_TOOL_HEIGHT : 0;
	}

	if(pVehicle->isFacingRight) {
		pVehicle->sBobTool.sPos.uwX += 24+3;
	}
	else {
		pVehicle->sBobTool.sPos.uwX += -13+3;
		ubFrameOffs += (
			VEHICLE_TOOL_ANIM_FRAMES * VEHICLE_DESTRUCTION_FRAMES *
			VEHICLE_TOOL_HEIGHT
		);
	}

	// Apply destruction
	ubFrameOffs += (
		VEHICLE_TOOL_HEIGHT * VEHICLE_TOOL_ANIM_FRAMES *
		pVehicle->ubDestructionState
	);

	// Vertical drill anim
	bobNewSetBitMapOffset(&pVehicle->sBobTool, ubFrameOffs);
}

static inline UBYTE vehicleStartDrilling(
	tVehicle *pVehicle, UWORD uwTileX, UWORD uwTileY, UBYTE ubDrillDir
) {
	static UBYTE ubCooldown = 0;
	if(pVehicle->uwDrillCurr < 30) {
		if(!ubCooldown) {
			textBobSet(
				&pVehicle->sTextBob, "Drill depleted!", 6,
				pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
				pVehicle->sBobBody.sPos.uwY,
				pVehicle->sBobBody.sPos.uwY - 32, 1
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
		UBYTE ubDrillCost = 30;
		pVehicle->uwDrillCurr -= ubDrillCost;
		warehouseElapseTime(ubDrillCost);
	}
	return 1;
}

static WORD vehicleRestock(tVehicle *pVehicle, UBYTE ubUseCashP1) {
	LONG *pCash = ubUseCashP1 ? &g_pVehicles[0].lCash : &pVehicle->lCash;

	pVehicle->ubCargoCurr = 0;
	pVehicle->uwCargoScore = 0;
	hudSetCargo(pVehicle->ubPlayerIdx, 0, pVehicle->ubCargoMax);
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		warehouseSetStock(i, warehouseGetStock(i) + pVehicle->pStock[i]);
		pVehicle->pStock[i] = 0;
	}

	// Fuel price per liter, units in liter
	const UBYTE ubLiterPrice = 5;
	const UBYTE ubFuelInLiter = 100;
	const UBYTE ubHullPrice = 2;

	// Buy as much fuel as needed
	// Start refueling if half a liter is spent
	UWORD uwRefuelLiters = (
		pVehicle->uwDrillMax - pVehicle->uwDrillCurr + ubFuelInLiter / 2
	) / ubFuelInLiter;

	// It's possible to buy more fuel than needed (last liter) - fill up to max
	pVehicle->uwDrillCurr = MIN(
		pVehicle->uwDrillCurr + uwRefuelLiters * ubFuelInLiter, pVehicle->uwDrillMax
	);

	// Buy as much hull as needed
	UWORD uwRehullCost = ubHullPrice * (pVehicle->wHullMax - pVehicle->wHullCurr);
	vehicleHullRepair(pVehicle);

	// Pay for your fuel & hull!
	WORD wRestockValue = uwRefuelLiters * ubLiterPrice + uwRehullCost;
	*pCash -= wRestockValue;
	return -wRestockValue;
}

static void vehicleExcavateTile(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
	// Load mineral to vehicle
	static const char * const szMessageFull = "Cargo full!";
	UBYTE ubTile = g_pMainBuffer->pTileData[uwX][uwY];
	if(ubTile == TILE_BONE_HEAD || ubTile == TILE_BONE_1) {
		char szMessage[50];
		if(g_ubDinoBonesFound < 9) {
			++g_ubDinoBonesFound;
		}
		sprintf(szMessage, "Found bone no. %hhu!", g_ubDinoBonesFound);
		textBobSet(
			&pVehicle->sTextBob, szMessage, COLOR_GREEN,
			pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
			pVehicle->sBobBody.sPos.uwY,
			pVehicle->sBobBody.sPos.uwY - 32, 1
		);
		audioPlay(
			AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
			g_pSampleOre, AUDIO_VOLUME_MAX, 1
		);
	}
	else if(g_pTileDefs[ubTile].szMsg) {
		UBYTE ubMineralType = g_pTileDefs[ubTile].ubMineral;
		const tMineralDef *pMineral = &g_pMinerals[ubMineralType];
		UBYTE ubSlots = g_pTileDefs[ubTile].ubSlots;
		ubSlots = MIN(ubSlots, pVehicle->ubCargoMax - pVehicle->ubCargoCurr);
		pVehicle->uwCargoScore += pMineral->ubReward * ubSlots;
		pVehicle->ubCargoCurr += ubSlots;
		pVehicle->pStock[ubMineralType] += ubSlots;
		warehousePlanUnlockMineral(ubMineralType);

		hudSetCargo(pVehicle->ubPlayerIdx, pVehicle->ubCargoCurr, pVehicle->ubCargoMax);
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
			szMessage = g_pTileDefs[ubTile].szMsg;
			ubColor = pMineral->ubTitleColor;
			audioPlay(
				AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
				g_pSampleOre, AUDIO_VOLUME_MAX, 1
			);
		}
		textBobSet(
			&pVehicle->sTextBob, szMessage, ubColor,
			pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
			pVehicle->sBobBody.sPos.uwY,
			pVehicle->sBobBody.sPos.uwY - 32, 1
		);
	}

	if(g_isChallenge) {
		if(TILE_CHECKPOINT_1 <= ubTile && ubTile <= TILE_CHECKPOINT_1 + 9) {
			if(uwY == TILE_ROW_CHALLENGE_FINISH) {
				pVehicle->lCash += pVehicle->uwCargoScore;
				vehicleRestock(pVehicle, 0);
				gameChallengeEnd();
			}
			else {
				textBobSetText(
					&pVehicle->sTextBob, "Checkpoint! %+hu\x1F", pVehicle->uwCargoScore
				);
				textBobSetColor(&pVehicle->sTextBob, COLOR_GREEN);
				textBobSetPos(
					&pVehicle->sTextBob,
					pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
					pVehicle->sBobBody.sPos.uwY,
					pVehicle->sBobBody.sPos.uwY - 32, 1
				);
				pVehicle->lCash += pVehicle->uwCargoScore;
				vehicleRestock(pVehicle, 0);
			}
			audioPlay(
				AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
				g_pSampleOre, AUDIO_VOLUME_MAX, 1
			);
		}
	}

	tileExcavate(uwX, uwY);
}

static void vehicleProcessMovement(tVehicle *pVehicle) {
	UBYTE isOnGround = 0;
	const fix16_t fMaxDx = 5 * fix16_one / 2;
	const fix16_t fAccX = fix16_one / 8;
	const fix16_t fFrictX = fix16_one / 12;

	if(
		g_isChallenge &&
		fix16_to_int(pVehicle->fY) < g_pMainBuffer->pCamera->uPos.uwY
	) {
		UWORD uwTileY = (
			g_pMainBuffer->pCamera->uPos.uwY +
			g_pMainBuffer->pCamera->sCommon.pVPort->uwHeight / 2
		) / 32;
		UWORD uwTileX = fix16_to_int(pVehicle->fX)/32;
		if(tileIsSolid(uwTileX, uwTileY)) {
			tileExcavate(uwTileX, uwTileY);
		}
		pVehicle->fY = fix16_from_int(uwTileY*32);
		pVehicle->fDy = fix16_from_int(-1); // HACK HACK HACK
		pVehicle->sBobBody.sPos.uwY = fix16_to_int(pVehicle->fY);
		audioPlay(
			AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
			g_pSampleTeleport, AUDIO_VOLUME_MAX, 1
		);
		UWORD uwTeleportPenalty = 200;
		textBobSetText(&pVehicle->sTextBob, "Teleport: -%hu\x1F", uwTeleportPenalty);
		textBobSetColor(&pVehicle->sTextBob, COLOR_REDEST);
		textBobSetPos(
			&pVehicle->sTextBob,
			pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
			pVehicle->sBobBody.sPos.uwY,
			pVehicle->sBobBody.sPos.uwY - 48, 1
		);
		pVehicle->lCash -= uwTeleportPenalty;

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
	pVehicle->sBobBody.sPos.uwX = fix16_to_int(pVehicle->fX);
	UBYTE ubAdd = (pVehicle->sBobBody.sPos.uwY > (1 + TILE_ROW_BASE_DIRT) * 32) ? 4 : 2;
	UBYTE ubHalfWidth = 12;

	UWORD uwCenterX = pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH / 2;
	UWORD uwTileBottom = (pVehicle->sBobBody.sPos.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
	UWORD uwTileMid = (pVehicle->sBobBody.sPos.uwY + VEHICLE_HEIGHT / 2) >> 5;
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
			if(pVehicle->fDy > 2 * fix16_one) {
				vehicleHullDamage(pVehicle, fix16_to_int(fix16_div(
					pVehicle->fDy - 2 * fix16_one, (fix16_one / 2)
				) * 2));
			}
			pVehicle->fDy = 0;
		}
	}
	pVehicle->sBobBody.sPos.uwY = fix16_to_int(pVehicle->fY);

	// Vehicle has its destination position calculated - check if it's visible
	// TODO: Could check only row for a bit faster calculations
	UWORD uwTileTop = (pVehicle->sBobBody.sPos.uwY + ubAdd) / 32;
	UBYTE isVehicleVisible = (
		tileBufferIsTileOnBuffer(g_pMainBuffer, uwTileLeft, uwTileTop) &&
		tileBufferIsTileOnBuffer(g_pMainBuffer, uwTileLeft, uwTileBottom)
	);

	// Update track bob
	pVehicle->sBobTrack.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
	pVehicle->sBobTrack.sPos.uwY += VEHICLE_BODY_HEIGHT - 1;
	if(pVehicle->ubJetShowFrame == 0) {
		// Jet hidden
		if(pVehicle->fDx) {
			// Animate wheel rotation
			pVehicle->ubTrackAnimCnt += fix16_abs(pVehicle->fDx);
			if(fix16_to_int(pVehicle->ubTrackAnimCnt) >= 5) {
				if(pVehicle->fDx > 0) {
					if(pVehicle->ubTrackFrame >= 3) {
						pVehicle->ubTrackFrame = 0;
					}
					else {
						++pVehicle->ubTrackFrame;
					}
				}
				else {
					if(pVehicle->ubTrackFrame <= 0) {
						pVehicle->ubTrackFrame = 3;
					}
					else {
						--pVehicle->ubTrackFrame;
					}
				}
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
			pVehicle->sBobBody.sPos.uwY += 1;
		}
	}
	else {
		pVehicle->sBobBody.sPos.uwY += s_pJetAnimOffsets[pVehicle->ubJetShowFrame];
		if(pVehicle->ubJetShowFrame == 5) {
			bobNewSetBitMapOffset(
				&pVehicle->sBobTrack, pVehicle->sSteer.bY ? TRACK_OFFSET_JET : 0
			);
		}
		else if(pVehicle->ubJetShowFrame == 10) {
			// Update jet pos
			pVehicle->ubJetAnimCnt = (pVehicle->ubJetAnimCnt + 1) & 15;
			bobNewSetBitMapOffset(
				&pVehicle->sBobJet, VEHICLE_FLAME_HEIGHT * (pVehicle->ubJetAnimCnt / 8)
			);
			pVehicle->sBobJet.sPos.ulYX = pVehicle->sBobTrack.sPos.ulYX;
			pVehicle->sBobJet.sPos.uwY += VEHICLE_TRACK_JET_HEIGHT;
			if(isVehicleVisible) {
				bobNewPush(&pVehicle->sBobJet);
			}
		}
	}
	if(isVehicleVisible) {
		bobNewPush(&pVehicle->sBobTrack);
	}

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
	if(isVehicleVisible) {
		bobNewPush(&pVehicle->sBobBody);
	}

	// Tool
	pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
	vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
	if(isVehicleVisible) {
		bobNewPush(&pVehicle->sBobTool);
	}

	if(vehicleIsNearShop(pVehicle)) {
		// If restocked then play audio & display score
		WORD wDeltaScore = vehicleRestock(pVehicle, 1);
		if(wDeltaScore) {
			audioPlay(
				AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
				g_pSampleOre, AUDIO_VOLUME_MAX, 1
			);
			textBobSetText(&pVehicle->sTextBob, "Restock! %hd\x1F", wDeltaScore);
			textBobSetColor(&pVehicle->sTextBob, COLOR_GOLD);
			textBobSetPos(
				&pVehicle->sTextBob,
				pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
				pVehicle->sBobBody.sPos.uwY,
				pVehicle->sBobBody.sPos.uwY - 48, 1
			);
		}
	}
}

static void vehicleProcessDestroyed(tVehicle *pVehicle) {
	bobNewPush(&pVehicle->sBobWreck);

	if(pVehicle->ubSmokeAnimCnt == 5) {
		pVehicle->ubSmokeAnimCnt = 0;
		++pVehicle->ubSmokeAnimFrame;
		if(pVehicle->ubSmokeAnimFrame == VEHICLE_SMOKE_FRAMES) {
			pVehicle->ubSmokeAnimFrame = 0;
		}
		bobNewSetBitMapOffset(
			&pVehicle->sBobSmoke, VEHICLE_SMOKE_FRAME_HEIGHT * pVehicle->ubSmokeAnimFrame
		);
	}
	else {
		++pVehicle->ubSmokeAnimCnt;
	}
	bobNewPush(&pVehicle->sBobSmoke);

	if(s_ubBebCountdown == 0) {
		vehicleRespawn(pVehicle);
	}
	else {
		--s_ubBebCountdown;
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

		pVehicle->sBobBody.sPos.uwX = fix16_to_int(pVehicle->fX);
		pVehicle->sBobBody.sPos.uwY = fix16_to_int(pVehicle->fY);
		pVehicle->sBobBody.sPos.uwY += pTrackAnimOffs[pVehicle->ubDrillVAnimCnt];
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
		pVehicle->sBobBody.sPos.uwX = fix16_to_int(pVehicle->fX);
		pVehicle->sBobBody.sPos.uwY = fix16_to_int(pVehicle->fY);
		// Pos for tool & track
		pVehicle->sBobTrack.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
		pVehicle->sBobTrack.sPos.uwY += VEHICLE_BODY_HEIGHT - 1;

		if(isDoneX && isDoneY) {
			if(pVehicle->ubDrillDir == DRILL_DIR_H) {
				pVehicle->ubDrillDir = DRILL_DIR_NONE;
				audioStop(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx);
			}
			else {
				const UBYTE ubAdd = 4; // No grass past this point
				UWORD uwTileBottom = (pVehicle->sBobBody.sPos.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
				UWORD uwCenterX = pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH / 2;
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
		}
		else {
			pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
			// Body shake
			pVehicle->sBobBody.sPos.uwX += ubRand() & 1;
			pVehicle->sBobBody.sPos.uwY += ubRand() & 1;

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
	else if(pVehicle->wHullCurr) {
		vehicleProcessMovement(pVehicle);
	}
	else {
		vehicleProcessDestroyed(pVehicle);
	}
	UBYTE ubPlayerIdx = pVehicle->ubPlayerIdx;
	hudSetDrill(ubPlayerIdx, pVehicle->uwDrillCurr, pVehicle->uwDrillMax);
	textBobAnimate(&pVehicle->sTextBob);
	hudSetDepth(ubPlayerIdx, MAX(
		0, fix16_to_int(pVehicle->fY) + VEHICLE_HEIGHT - (TILE_ROW_BASE_DIRT)*32
	));
	hudSetCash(ubPlayerIdx, pVehicle->lCash);
}
