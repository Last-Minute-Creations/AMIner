/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include <ace/managers/rand.h>
#include "hud.h"
#include "core.h"
#include "game.h"
#include "tile.h"
#include "warehouse.h"
#include "color.h"
#include "explosion.h"
#include "dino.h"
#include "ground_layer.h"
#include "defs.h"
#include "inventory.h"

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

static const fix16_t s_fAccGrav = F16(1) / 8;
static const fix16_t s_fMaxGravDy = 4 * F16(1);

tBitMap *s_pBodyFrames[2], *s_pBodyMask;
tBitMap *s_pTrackFrames, *s_pTrackMask;
tBitMap *s_pJetFrames, *s_pJetMask;
tBitMap *s_pToolFrames[2], *s_pToolMask;
tBitMap *s_pWreckFrames[2], *s_pWreckMask;
tBitMap *s_pSmokeFrames, *s_pSmokeMask;

const UBYTE s_pJetAnimOffsets[VEHICLE_TRACK_HEIGHT * 2 + 1] = {
	0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0
};

static UBYTE s_ubBebCountdown = 0;

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

void vehicleSetPos(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
	pVehicle->ubDrillDir = DRILL_DIR_NONE;
	pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;

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

	pVehicle->fX = fix16_from_int(uwX);
	pVehicle->fY = fix16_from_int(uwY);
	pVehicle->fDx = 0;
	pVehicle->fDy = 0;
	if(pVehicle->ubPlayerIdx == PLAYER_1) {
		vehicleMove(pVehicle, 1, 0);
	}
	else {
		vehicleMove(pVehicle, -1, 0);
	}
}

void vehicleResetPos(tVehicle *pVehicle) {
	UWORD uwX;
	if(pVehicle->ubPlayerIdx == PLAYER_1) {
		uwX = g_isChallenge ? 0 : 96;
	}
	else {
		uwX = g_isChallenge ? 96 : 320-64;
	}
	UWORD uwY = (TILE_ROW_BASE_DIRT - 2) * 32;
	vehicleSetPos(pVehicle, uwX, uwY);
}

void vehicleUpdateBodyBob(tVehicle *pVehicle) {
	UBYTE ubFrameOffs = VEHICLE_BODY_HEIGHT * pVehicle->ubDestructionState;
	if(!pVehicle->isFacingRight) {
		ubFrameOffs += VEHICLE_BODY_HEIGHT * VEHICLE_DESTRUCTION_FRAMES;
	}
	bobNewSetBitMapOffset(&pVehicle->sBobBody, ubFrameOffs);
}

static void vehicleOnExplodePeak(ULONG ulData) {
	tVehicle *pVehicle = &g_pVehicles[ulData];
	pVehicle->ubVehicleState = VEHICLE_STATE_SMOKING;
}

static void vehicleCrash(tVehicle *pVehicle) {
	// Calculate pos for bobs
	pVehicle->sBobWreck.sPos.uwX = fix16_to_int(pVehicle->fX);
	pVehicle->sBobWreck.sPos.uwY = (
		fix16_to_int(pVehicle->fY) + VEHICLE_BODY_HEIGHT +
		VEHICLE_TRACK_HEIGHT - VEHICLE_WRECK_HEIGHT
	);
	pVehicle->sBobSmoke.sPos.uwX = fix16_to_int(pVehicle->fX);

	explosionAdd(
		pVehicle->sBobBody.sPos.uwX, pVehicle->sBobBody.sPos.uwY,
		vehicleOnExplodePeak, pVehicle->ubPlayerIdx, 0, 0
	);
	pVehicle->ubVehicleState = VEHICLE_STATE_EXPLODING;

	s_ubBebCountdown = 200;
}

static void vehicleHullDamage(tVehicle *pVehicle, UWORD uwDmg) {
	UWORD uwHullMax = inventoryGetPartDef(INVENTORY_PART_HULL)->uwMax;
	pVehicle->wHullCurr = MAX(0, pVehicle->wHullCurr - uwDmg);
	if(pVehicle->wHullCurr == 0) {
		vehicleCrash(pVehicle);
	}
	else {
		pVehicle->ubDestructionState = (
			(((UWORD)pVehicle->wHullCurr - 1) * VEHICLE_DESTRUCTION_FRAMES) /
			uwHullMax
		);
		vehicleUpdateBodyBob(pVehicle);
	}
	hudSetHull(pVehicle->ubPlayerIdx, pVehicle->wHullCurr, uwHullMax);
}

static void vehicleHullRepair(tVehicle *pVehicle) {
	UWORD uwHullMax = inventoryGetPartDef(INVENTORY_PART_HULL)->uwMax;
	pVehicle->wHullCurr = uwHullMax;
	pVehicle->ubDestructionState = VEHICLE_DESTRUCTION_FRAMES - 1;
	hudSetHull(pVehicle->ubPlayerIdx, pVehicle->wHullCurr, uwHullMax);
}

void vehicleRespawn(tVehicle *pVehicle) {
	pVehicle->uwCargoCurr = 0;
	pVehicle->uwCargoScore = 0;
	pVehicle->uwDrillCurr = inventoryGetPartDef(INVENTORY_PART_DRILL)->uwMax;
	vehicleHullRepair(pVehicle);
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		pVehicle->pStock[i] = 0;
	}
	vehicleResetPos(pVehicle);
}

void vehicleReset(tVehicle *pVehicle) {
	// Initial values
	pVehicle->lCash = g_lInitialCash;
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
	pVehicle->sDynamite.ubPlayer = ubIdx;

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

	if(pVehicle->ubVehicleState != VEHICLE_STATE_MOVING) {
		// No vehicle rotating when other state than moving
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
	pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
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

	// Check if other player drills the same tile
	const tVehicle * const pOther = &g_pVehicles[!pVehicle->ubPlayerIdx];
	if(
		pOther->ubDrillDir &&
		pOther->sDrillTile.uwY == uwTileY && pOther->sDrillTile.uwX == uwTileX
	) {
		return 0;
	}

	// Calculate layer difficulty
	BYTE bDifficulty;
	if(tileIsHardToDrill(uwTileX, uwTileY)) {
		bDifficulty = 10;
	}
	else {
		bDifficulty = groundLayerGetDifficultyAtDepth(uwTileY << 5);
	}
	BYTE bDrillLevel = inventoryGetPartDef(INVENTORY_PART_DRILL)->ubLevel;
	BYTE bDrillDuration = MAX(1, bDifficulty - bDrillLevel);
	UBYTE ubDrillCost = g_ubDrillingCost * bDrillDuration;

	// Check vehicle's drill depletion
	if(!g_isChallenge) {
		if(pVehicle->uwDrillCurr < ubDrillCost) {
			if(!ubCooldown) {
				textBobSet(
					&pVehicle->sTextBob, g_pMsgs[MSG_MISC_DRILL_DEPLETED], 6,
					pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
					pVehicle->sBobBody.sPos.uwY,
					pVehicle->sBobBody.sPos.uwY - 32, 1
				);
				// audioPlay(
				// 	AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
				// 	g_pSamplePenalty, AUDIO_VOLUME_MAX, 1
				// );
				ubCooldown = 25;
			}
			else {
				--ubCooldown;
			}
			return 0;
		}
		pVehicle->uwDrillCurr -= ubDrillCost;
		warehouseElapseTime(ubDrillCost);
	}

	pVehicle->ubDrillState = (
		ubDrillDir == DRILL_DIR_V ? DRILL_STATE_VERT_ANIM_IN : DRILL_STATE_DRILLING
	);
	pVehicle->ubVehicleState = VEHICLE_STATE_DRILLING;
	pVehicle->ubDrillDir = ubDrillDir;

	pVehicle->sDrillTile.uwX = uwTileX;
	pVehicle->sDrillTile.uwY = uwTileY;
	pVehicle->fDrillDestX = fix16_from_int(uwTileX << 5);
	pVehicle->fDrillDestY = fix16_from_int(((uwTileY + 1) << 5) - VEHICLE_HEIGHT - 4);
	pVehicle->fDrillDelta = (fix16_one * 3)/(bDrillDuration * 2);

	pVehicle->fDx = 0;
	pVehicle->fDy = 0;
	// audioPlay(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx, g_pSampleDrill, AUDIO_VOLUME_MAX, -1);

	return 1;
}

static WORD vehicleRestock(tVehicle *pVehicle, UBYTE ubUseCashP1) {
	LONG *pCash = ubUseCashP1 ? &g_pVehicles[0].lCash : &pVehicle->lCash;

	pVehicle->uwCargoCurr = 0;
	pVehicle->uwCargoScore = 0;
	UWORD uwCargoMax = inventoryGetPartDef(INVENTORY_PART_CARGO)->uwMax;
	hudSetCargo(pVehicle->ubPlayerIdx, 0, uwCargoMax);
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		warehouseSetStock(i, warehouseGetStock(i) + pVehicle->pStock[i]);
		pVehicle->pStock[i] = 0;
	}

	// Buy as much fuel as needed
	// Start refueling if half a liter is spent
	UWORD uwDrillMax = inventoryGetPartDef(INVENTORY_PART_DRILL)->uwMax;
	UWORD uwRefuelLiters = (
		uwDrillMax - pVehicle->uwDrillCurr + g_ubFuelInLiter / 2
	) / g_ubFuelInLiter;

	// It's possible to buy more fuel than needed (last liter) - fill up to max
	pVehicle->uwDrillCurr = MIN(
		pVehicle->uwDrillCurr + uwRefuelLiters * g_ubFuelInLiter, uwDrillMax
	);

	// Buy as much hull as needed
	UWORD uwHullMax = inventoryGetPartDef(INVENTORY_PART_HULL)->uwMax;
	UWORD uwRehullCost = g_ubHullPrice * (uwHullMax - pVehicle->wHullCurr);
	vehicleHullRepair(pVehicle);

	// Pay for your fuel & hull!
	WORD wRestockValue = uwRefuelLiters * g_ubLiterPrice + uwRehullCost;
	*pCash -= wRestockValue;
	return -wRestockValue;
}

void vehicleExcavateTile(tVehicle *pVehicle, UWORD uwTileX, UWORD uwTileY) {
	// Load mineral to vehicle
	UBYTE ubTile = g_pMainBuffer->pTileData[uwTileX][uwTileY];
	if(ubTile == TILE_BONE_HEAD || ubTile == TILE_BONE_1) {
		char szMessage[50];
		if(dinoGetBoneCount() < 9) {
			dinoFoundBone();
		}
		sprintf(szMessage, g_pMsgs[MSG_MISC_FOUND_BONE], dinoGetBoneCount());
		textBobSet(
			&pVehicle->sTextBob, szMessage, COLOR_GREEN,
			pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH/2,
			pVehicle->sBobBody.sPos.uwY,
			pVehicle->sBobBody.sPos.uwY - 32, 1
		);
		// audioPlay(
		// 	AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
		// 	g_pSampleOre, AUDIO_VOLUME_MAX, 1
		// );
	}
	else if(g_pTileDefs[ubTile].ubSlots) {
		UWORD uwCargoMax = inventoryGetPartDef(INVENTORY_PART_CARGO)->uwMax;
		UBYTE ubMineralType = g_pTileDefs[ubTile].ubMineral;
		const tMineralDef *pMineral = &g_pMinerals[ubMineralType];
		UBYTE ubSlots = g_pTileDefs[ubTile].ubSlots;
		ubSlots = MIN(ubSlots, uwCargoMax - pVehicle->uwCargoCurr);
		pVehicle->uwCargoScore += pMineral->ubReward * ubSlots;
		pVehicle->uwCargoCurr += ubSlots;
		pVehicle->pStock[ubMineralType] += ubSlots;
		warehousePlanUnlockMineral(ubMineralType);

		hudSetCargo(pVehicle->ubPlayerIdx, pVehicle->uwCargoCurr, uwCargoMax);
		char szMsgBuffer[40];
		const char *szMessage;
		UBYTE ubColor;
		if(pVehicle->uwCargoCurr == uwCargoMax) {
			szMessage = g_pMsgs[MSG_MISC_CARGO_FULL];
			ubColor = COLOR_REDEST;
			// audioPlay(
			// 	AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
			// 	g_pSamplePenalty, AUDIO_VOLUME_MAX, 1
			// );
		}
		else {
			sprintf(
				szMsgBuffer, "%s x%hhu",
				g_pMineralNames[g_pTileDefs[ubTile].ubMineral],
				g_pTileDefs[ubTile].ubSlots
			);
			szMessage = szMsgBuffer;
			ubColor = pMineral->ubTitleColor;
			// audioPlay(
			// 	AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
			// 	g_pSampleOre, AUDIO_VOLUME_MAX, 1
			// );
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
			if(uwTileY == TILE_ROW_CHALLENGE_FINISH) {
				pVehicle->lCash += pVehicle->uwCargoScore;
				vehicleRestock(pVehicle, 0);
				gameChallengeEnd();
			}
			else {
				textBobSetText(
					&pVehicle->sTextBob, "%s %+hu\x1F",
					g_pMsgs[MSG_CHALLENGE_CHECKPOINT], pVehicle->uwCargoScore
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
			// audioPlay(
			// 	AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
			// 	g_pSampleOre, AUDIO_VOLUME_MAX, 1
			// );
		}
	}

	tileExcavate(uwTileX, uwTileY);
}

static void vehicleProcessMovement(tVehicle *pVehicle) {
	UBYTE isOnGround = 0;
	const fix16_t fMaxDx = 2 * fix16_one;
	const fix16_t fAccX = fix16_one / 16;
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
		// audioPlay(
		// 	AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx,
		// 	g_pSamplePenalty, AUDIO_VOLUME_MAX, 1
		// );
		UWORD uwTeleportPenalty = 50;
		textBobSetText(
			&pVehicle->sTextBob, "%s -%hu\x1F",
			g_pMsgs[MSG_CHALLENGE_TELEPORT], uwTeleportPenalty
		);
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
		pVehicle->fDy = MIN(s_fMaxGravDy, pVehicle->fDy + s_fAccGrav);
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
				// vehicleHullDamage(pVehicle, fix16_to_int(pVehicle->fDy - 4 * fix16_one));
				vehicleHullDamage(pVehicle, fix16_to_int(fix16_mul(
					pVehicle->fDy - 2 * fix16_one, fix16_one * 4
				)));
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
			pVehicle->ubJetAnimCnt = (pVehicle->ubJetAnimCnt + 1) & 63;
			bobNewSetBitMapOffset(
				&pVehicle->sBobJet, VEHICLE_FLAME_HEIGHT * (pVehicle->ubJetAnimCnt / 4)
			);
			pVehicle->sBobJet.sPos.ulYX = pVehicle->sBobTrack.sPos.ulYX;
			pVehicle->sBobJet.sPos.uwY += VEHICLE_TRACK_JET_HEIGHT;
			if(isVehicleVisible) {
				gameTryPushBob(&pVehicle->sBobJet);
			}
		}
	}
	if(isVehicleVisible) {
		gameTryPushBob(&pVehicle->sBobTrack);
	}

	// Drilling
	if(isOnGround && !dynamiteIsActive(&pVehicle->sDynamite)) {
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
		gameTryPushBob(&pVehicle->sBobBody);
	}

	// Tool
	vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
	if(isVehicleVisible) {
		gameTryPushBob(&pVehicle->sBobTool);
	}

	if(vehicleIsNearShop(pVehicle)) {
		// If restocked then play audio & display score
		WORD wDeltaScore = vehicleRestock(pVehicle, 1);
		if(wDeltaScore) {
			// audioPlay(
			// 	AUDIO_CHANNEL_2 + pVehicle->ubPlayerIdx,
			// 	g_pSampleOre, AUDIO_VOLUME_MAX, 1
			// );
			textBobSetText(
				&pVehicle->sTextBob, "%s %hd\x1F",
				g_pMsgs[MSG_MISC_RESTOCK], wDeltaScore
			);
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

static void vehicleProcessDeadGravity(tVehicle *pVehicle) {
	UWORD uwY = fix16_to_int(pVehicle->fY);
	UBYTE ubAdd = (uwY > (1 + TILE_ROW_BASE_DIRT) * 32) ? 4 : 2;
	UWORD uwTileBottom = (uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
	UWORD uwCenterX = pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH / 2;
	UWORD uwTileCenter = uwCenterX >> 5;
	if(!tileIsSolid(uwTileCenter, uwTileBottom)) {
		// Gravity
		pVehicle->fDy = MIN(s_fMaxGravDy, pVehicle->fDy + s_fAccGrav);
		pVehicle->fY += pVehicle->fDy;
	}
	else {
		// Collision with ground
		pVehicle->fY = fix16_from_int((uwTileBottom << 5) - VEHICLE_HEIGHT - ubAdd);
		pVehicle->fDy = 0;
	}
}

static void vehicleProcessSmoking(tVehicle *pVehicle) {
	// TODO: push if vehicle is visible
	vehicleProcessDeadGravity(pVehicle);
	pVehicle->sBobWreck.sPos.uwY = (
		fix16_to_int(pVehicle->fY) + VEHICLE_BODY_HEIGHT +
		VEHICLE_TRACK_HEIGHT - VEHICLE_WRECK_HEIGHT
	);
	gameTryPushBob(&pVehicle->sBobWreck);

	pVehicle->sBobSmoke.sPos.uwY = (
		fix16_to_int(pVehicle->fY) + VEHICLE_BODY_HEIGHT +
		VEHICLE_TRACK_HEIGHT - VEHICLE_SMOKE_ANIM_HEIGHT
	);
	if(pVehicle->ubSmokeAnimCnt == 10) {
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
	gameTryPushBob(&pVehicle->sBobSmoke);

	if(s_ubBebCountdown == 0) {
		vehicleRespawn(pVehicle);
	}
	else {
		--s_ubBebCountdown;
	}
}

UBYTE transitionVarToBy(fix16_t *pVar, fix16_t fDest, fix16_t fDelta) {
	// Var is close to dest - set to same
	if(fix16_abs(*pVar - fDest) <= fDelta) {
		*pVar = fDest;
		return 1;
	}

	// Increment by delta if diff is big enough
	if (*pVar < fDest) {
		*pVar += fDelta;
	}
	else {
		*pVar -= fDelta;
	}
	return 0;
}

static void vehicleProcessDrilling(tVehicle *pVehicle) {
	static const UBYTE pTrackAnimOffs[DRILL_V_ANIM_LEN] = {
		0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0
	};
	switch(pVehicle->ubDrillState) {
		case DRILL_STATE_VERT_ANIM_IN:
		case DRILL_STATE_VERT_ANIM_OUT:
			if(pVehicle->ubDrillState == DRILL_STATE_VERT_ANIM_IN) {
				++pVehicle->ubDrillVAnimCnt;
				if(pVehicle->ubDrillVAnimCnt >= DRILL_V_ANIM_LEN - 1) {
					pVehicle->ubDrillState = DRILL_STATE_DRILLING;
				}
				else if(pVehicle->ubDrillVAnimCnt == 5) {
					bobNewSetBitMapOffset(&pVehicle->sBobTrack, TRACK_OFFSET_DRILL);
				}
			}
			else {
				if(!--pVehicle->ubDrillVAnimCnt) {
					pVehicle->ubDrillDir = DRILL_DIR_NONE;
					pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;
				}
				else if(pVehicle->ubDrillVAnimCnt == 5) {
					bobNewSetBitMapOffset(&pVehicle->sBobTrack, TRACK_OFFSET_TRACK);
				}
			}

			pVehicle->sBobBody.sPos.uwX = fix16_to_int(pVehicle->fX);
			pVehicle->sBobBody.sPos.uwY = fix16_to_int(pVehicle->fY);
			pVehicle->sBobBody.sPos.uwY += pTrackAnimOffs[pVehicle->ubDrillVAnimCnt];
			vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
			break;
		case DRILL_STATE_DRILLING: {
			// Movement - process X and Y since vehicle may be not centered on tile
			// when drilling vertically and we must correct X pos
			fix16_t fDrillDelta = pVehicle->fDrillDelta;
			UBYTE isDoneX = transitionVarToBy(&pVehicle->fX, pVehicle->fDrillDestX, fDrillDelta);
			UBYTE isDoneY = transitionVarToBy(&pVehicle->fY, pVehicle->fDrillDestY, fDrillDelta);

			// Pos for tool & track
			pVehicle->sBobTrack.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
			pVehicle->sBobTrack.sPos.uwY += VEHICLE_BODY_HEIGHT - 1;
			pVehicle->sBobBody.sPos.uwX = fix16_to_int(pVehicle->fX);
			pVehicle->sBobBody.sPos.uwY = fix16_to_int(pVehicle->fY);

			if(isDoneX && isDoneY) {
				vehicleExcavateTile(pVehicle, pVehicle->sDrillTile.uwX, pVehicle->sDrillTile.uwY);
				if(pVehicle->ubDrillDir == DRILL_DIR_H) {
					pVehicle->ubDrillDir = DRILL_DIR_NONE;
					pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;
					// audioStop(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx);
				}
				else {
					const UBYTE ubAdd = 4; // No grass past this point
					UWORD uwTileBottom = (pVehicle->sBobBody.sPos.uwY + VEHICLE_HEIGHT + ubAdd) >> 5;
					UWORD uwTileX = pVehicle->sBobBody.sPos.uwX >> 5;
					if(
						pVehicle->sSteer.bY > 0 && tileIsDrillable(uwTileX, uwTileBottom) &&
						vehicleStartDrilling(pVehicle, uwTileX, uwTileBottom, DRILL_DIR_V)
					) {
						pVehicle->ubDrillState = DRILL_STATE_DRILLING;
					}
					else {
						pVehicle->ubDrillState = DRILL_STATE_VERT_ANIM_OUT;
						// audioStop(AUDIO_CHANNEL_0 + pVehicle->ubPlayerIdx);
					}
				}
			}
			else {
				// Body shake
				pVehicle->sBobBody.sPos.uwX += uwRand() & 1;
				pVehicle->sBobBody.sPos.uwY += uwRand() & 1;

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
		} break;
		default:
			break;
	}

	gameTryPushBob(&pVehicle->sBobTrack);
	gameTryPushBob(&pVehicle->sBobBody);
	gameTryPushBob(&pVehicle->sBobTool);
}

void vehicleProcessText(void) {
	// MUST BE BEFORE ANY BOB PUSH
	static UBYTE ubPlayer = 0;
	textBobUpdate(&g_pVehicles[ubPlayer].sTextBob);
	ubPlayer = !ubPlayer;
}

void vehicleProcessExploding(tVehicle *pVehicle) {
	// TODO: push if vehicle is visible
	vehicleProcessDeadGravity(pVehicle);
	pVehicle->sBobTrack.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
	pVehicle->sBobTrack.sPos.uwY += VEHICLE_BODY_HEIGHT - 1;
	gameTryPushBob(&pVehicle->sBobTrack);
	gameTryPushBob(&pVehicle->sBobBody);
	vehicleSetTool(pVehicle, TOOL_STATE_IDLE, 0);
	gameTryPushBob(&pVehicle->sBobTool);
}

void vehicleProcess(tVehicle *pVehicle) {
	switch(pVehicle->ubVehicleState) {
		case VEHICLE_STATE_DRILLING:
			vehicleProcessDrilling(pVehicle);
			break;
		case VEHICLE_STATE_MOVING:
			vehicleProcessMovement(pVehicle);
			break;
		case VEHICLE_STATE_EXPLODING:
		case VEHICLE_STATE_TELEPORTING_OUT:
			vehicleProcessExploding(pVehicle);
			break;
		case VEHICLE_STATE_SMOKING:
			vehicleProcessSmoking(pVehicle);
			break;
		case VEHICLE_STATE_TELEPORTING_IN:
			break;
	}
	UBYTE ubPlayerIdx = pVehicle->ubPlayerIdx;
	UWORD uwDrillMax = inventoryGetPartDef(INVENTORY_PART_DRILL)->uwMax;
	hudSetDrill(ubPlayerIdx, pVehicle->uwDrillCurr, uwDrillMax);
	textBobAnimate(&pVehicle->sTextBob);
	hudSetDepth(ubPlayerIdx, MAX(
		0, fix16_to_int(pVehicle->fY) + VEHICLE_HEIGHT - (TILE_ROW_BASE_DIRT)*32
	));
	hudSetCash(ubPlayerIdx, pVehicle->lCash);
}

uint8_t vehiclesAreClose(void) {
	const UWORD uwVpHeight = 256 - 31;
	WORD wDelta = ABS(
		g_pVehicles[0].sBobBody.sPos.uwY - g_pVehicles[1].sBobBody.sPos.uwY
	);
	if(wDelta <= uwVpHeight) {
		return 1;
	}
	return 0;
}

static void vehicleOnTeleportInPeak(ULONG ulData) {
	tVehicle *pVehicle = (tVehicle*)ulData;
	pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;
	UWORD uwMaxHealth = inventoryGetPartDef(INVENTORY_PART_HULL)->uwMax;
	if(uwRandMax(100) <= 5) {
		vehicleHullDamage(pVehicle, uwMaxHealth);
	}
	else if(uwRandMax(100) <= 20) {
		vehicleHullDamage(pVehicle, uwMaxHealth / 2);
	}
}

static void vehicleOnTeleportOutPeak(ULONG ulData) {
	tVehicle *pVehicle = (tVehicle*)ulData;
	pVehicle->ubTeleportAnimFrame = 0;
	pVehicle->ubTeleportAnimCnt = 0;
	vehicleSetPos(pVehicle, pVehicle->uwTeleportX, pVehicle->uwTeleportY);
	pVehicle->ubVehicleState = VEHICLE_STATE_TELEPORTING_IN;
	explosionAdd(
		pVehicle->uwTeleportX, pVehicle->uwTeleportY,
		vehicleOnTeleportInPeak, (ULONG)pVehicle, 0, 1
	);
}

void vehicleTeleport(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
	pVehicle->uwTeleportX = uwX;
	pVehicle->uwTeleportY = uwY;
	pVehicle->ubVehicleState = VEHICLE_STATE_TELEPORTING_OUT;
	explosionAdd(
		pVehicle->sBobBody.sPos.uwX, pVehicle->sBobBody.sPos.uwY,
		vehicleOnTeleportOutPeak, (ULONG)pVehicle, 0, 1
	);
}

tVehicle g_pVehicles[2];
