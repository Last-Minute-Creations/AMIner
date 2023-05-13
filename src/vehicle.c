/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include <ace/managers/rand.h>
#include <ace/utils/string.h>
#include <ace/contrib/managers/audio_mixer.h>
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
#include "save.h"

#define VEHICLE_BODY_HEIGHT 20
#define VEHICLE_DESTRUCTION_FRAMES 4
#define VEHICLE_TOOL_ANIM_FRAMES 2
#define VEHICLE_TOOL_OFFSET_Y 3
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

#define SFX_CHANNEL_DRILL 0
#define SFX_CHANNEL_EFFECT 1

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
tBitMap *s_pWhiteBody, *s_pWhiteTool;

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

	s_pWhiteBody = bitmapCreate(VEHICLE_WIDTH, VEHICLE_BODY_HEIGHT, GAME_BPP, BMF_INTERLEAVED);
	blitRect(s_pWhiteBody, 0, 0, VEHICLE_WIDTH, VEHICLE_BODY_HEIGHT, COLOR_WHITE);
	s_pWhiteTool = bitmapCreate(VEHICLE_TOOL_WIDTH, VEHICLE_TOOL_HEIGHT, GAME_BPP, BMF_INTERLEAVED);
	blitRect(s_pWhiteTool, 0, 0, VEHICLE_TOOL_WIDTH, VEHICLE_TOOL_HEIGHT, COLOR_WHITE);
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

	bitmapDestroy(s_pWhiteBody);
	bitmapDestroy(s_pWhiteTool);
}

void vehicleSetPos(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
	pVehicle->ubDrillDir = DRILL_DIR_NONE;
	pVehicle->ubDrillState = DRILL_STATE_OFF;
	pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;

	pVehicle->fTrackAnimCnt = 0;
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
	UWORD uwY = (TILE_ROW_BASE_DIRT - 1) * 32;
	vehicleSetPos(pVehicle, uwX, uwY);
}

void vehicleUpdateBodyBob(tVehicle *pVehicle) {
	UBYTE ubFrameOffs = VEHICLE_BODY_HEIGHT * pVehicle->ubDestructionState;
	if(!pVehicle->isFacingRight) {
		ubFrameOffs += VEHICLE_BODY_HEIGHT * VEHICLE_DESTRUCTION_FRAMES;
	}

	UWORD uwByteOffset = s_pBodyMask->BytesPerRow * ubFrameOffs;
	UBYTE *pFrame;
	if(pVehicle->ubDamageFrames) {
		--pVehicle->ubDamageFrames;
		pFrame = s_pWhiteBody->Planes[0];
	}
	else {
		pFrame = &s_pBodyFrames[pVehicle->ubPlayerIdx]->Planes[0][uwByteOffset];
	}

	bobSetFrame(
		&pVehicle->sBobBody,
		pFrame,
		&s_pBodyMask->Planes[0][uwByteOffset]
	);
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
		vehicleOnExplodePeak, pVehicle->ubPlayerIdx, 0, EXPLOSION_KIND_BOOM
	);
	pVehicle->ubVehicleState = VEHICLE_STATE_EXPLODING;

	s_ubBebCountdown = 200;
}

static void vehicleHullDamage(tVehicle *pVehicle, UWORD uwDmg) {
	UWORD uwHullMax = inventoryGetPartDef(INVENTORY_PART_HULL)->uwMax;
	pVehicle->wHullCurr = MAX(0, pVehicle->wHullCurr - uwDmg);
	pVehicle->ubDamageFrames = 5;
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
	pVehicle->ubDamageFrames = 0;
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		pVehicle->pStock[i] = 0;
	}
	vehicleResetPos(pVehicle);
}

void vehicleReset(tVehicle *pVehicle) {
	// Initial values
	pVehicle->isChallengeEnded = 0;
	pVehicle->lCash = g_lInitialCash;
	vehicleRespawn(pVehicle);
}

void vehicleSave(tVehicle *pVehicle, tFile *pFile) {
	saveWriteHeader(pFile, "VHCL");
	fileWrite(pFile, &pVehicle->sSteer, sizeof(pVehicle->sSteer));
	// textBobSave(&pVehicle->sTextBob, pFile);
	// bobSave(&pVehicle->sBobBody, pFile);
	// bobSave(&pVehicle->sBobTrack, pFile);
	// bobSave(&pVehicle->sBobJet, pFile);
	// bobSave(&pVehicle->sBobTool, pFile);
	// bobSave(&pVehicle->sBobWreck, pFile);
	// bobSave(&pVehicle->sBobSmoke, pFile);
	fileWrite(pFile, &pVehicle->fX, sizeof(pVehicle->fX));
	fileWrite(pFile, &pVehicle->fY, sizeof(pVehicle->fY));
	fileWrite(pFile, &pVehicle->fDx, sizeof(pVehicle->fDx));
	fileWrite(pFile, &pVehicle->fDy, sizeof(pVehicle->fDy));
	fileWrite(pFile, &pVehicle->ubVehicleState, sizeof(pVehicle->ubVehicleState));
	fileWrite(pFile, &pVehicle->isFacingRight, sizeof(pVehicle->isFacingRight));
	fileWrite(pFile, &pVehicle->ubTrackFrame, sizeof(pVehicle->ubTrackFrame));
	fileWrite(pFile, &pVehicle->fTrackAnimCnt, sizeof(pVehicle->fTrackAnimCnt));
	fileWrite(pFile, &pVehicle->ubBodyShakeCnt, sizeof(pVehicle->ubBodyShakeCnt));
	fileWrite(pFile, &pVehicle->ubJetShowFrame, sizeof(pVehicle->ubJetShowFrame));
	fileWrite(pFile, &pVehicle->ubJetAnimFrame, sizeof(pVehicle->ubJetAnimFrame));
	fileWrite(pFile, &pVehicle->ubJetAnimCnt, sizeof(pVehicle->ubJetAnimCnt));
	fileWrite(pFile, &pVehicle->ubToolAnimCnt, sizeof(pVehicle->ubToolAnimCnt));
	fileWrite(pFile, &pVehicle->ubDrillDir, sizeof(pVehicle->ubDrillDir));
	fileWrite(pFile, &pVehicle->ubDrillVAnimCnt, sizeof(pVehicle->ubDrillVAnimCnt));
	fileWrite(pFile, &pVehicle->fDrillDestX, sizeof(pVehicle->fDrillDestX));
	fileWrite(pFile, &pVehicle->fDrillDestY, sizeof(pVehicle->fDrillDestY));
	fileWrite(pFile, &pVehicle->fDrillDelta, sizeof(pVehicle->fDrillDelta));
	fileWrite(pFile, &pVehicle->sDrillTile, sizeof(pVehicle->sDrillTile));
	fileWrite(pFile, &pVehicle->ubSmokeAnimFrame, sizeof(pVehicle->ubSmokeAnimFrame));
	fileWrite(pFile, &pVehicle->ubSmokeAnimCnt, sizeof(pVehicle->ubSmokeAnimCnt));
	fileWrite(pFile, &pVehicle->ubTeleportAnimFrame, sizeof(pVehicle->ubTeleportAnimFrame));
	fileWrite(pFile, &pVehicle->ubTeleportAnimCnt, sizeof(pVehicle->ubTeleportAnimCnt));
	fileWrite(pFile, &pVehicle->uwTeleportX, sizeof(pVehicle->uwTeleportX));
	fileWrite(pFile, &pVehicle->uwTeleportY, sizeof(pVehicle->uwTeleportY));
	fileWrite(pFile, &pVehicle->ubDrillState, sizeof(pVehicle->ubDrillState));
	fileWrite(pFile, &pVehicle->uwCargoCurr, sizeof(pVehicle->uwCargoCurr));
	fileWrite(pFile, &pVehicle->uwCargoScore, sizeof(pVehicle->uwCargoScore));
	fileWrite(pFile, pVehicle->pStock, sizeof(pVehicle->pStock));
	fileWrite(pFile, &pVehicle->lCash, sizeof(pVehicle->lCash));
	fileWrite(pFile, &pVehicle->uwDrillCurr, sizeof(pVehicle->uwDrillCurr));
	fileWrite(pFile, &pVehicle->wHullCurr, sizeof(pVehicle->wHullCurr));
	fileWrite(pFile, &pVehicle->ubPlayerIdx, sizeof(pVehicle->ubPlayerIdx));
	fileWrite(pFile, &pVehicle->ubDestructionState, sizeof(pVehicle->ubDestructionState));
	fileWrite(pFile, &pVehicle->sDynamite, sizeof(pVehicle->sDynamite));
	fileWrite(pFile, &pVehicle->ubDamageFrames, sizeof(pVehicle->ubDamageFrames));
}

UBYTE vehicleLoad(tVehicle *pVehicle, tFile *pFile) {
	if(!saveReadHeader(pFile, "VHCL")) {
		return 0;
	}

	fileRead(pFile, &pVehicle->sSteer, sizeof(pVehicle->sSteer));
	// textBobLoad(&pVehicle->sTextBob, pFile);
	// bobLoad(&pVehicle->sBobBody, pFile);
	// bobLoad(&pVehicle->sBobTrack, pFile);
	// bobLoad(&pVehicle->sBobJet, pFile);
	// bobLoad(&pVehicle->sBobTool, pFile);
	// bobLoad(&pVehicle->sBobWreck, pFile);
	// bobLoad(&pVehicle->sBobSmoke, pFile);
	fileRead(pFile, &pVehicle->fX, sizeof(pVehicle->fX));
	fileRead(pFile, &pVehicle->fY, sizeof(pVehicle->fY));
	fileRead(pFile, &pVehicle->fDx, sizeof(pVehicle->fDx));
	fileRead(pFile, &pVehicle->fDy, sizeof(pVehicle->fDy));
	fileRead(pFile, &pVehicle->ubVehicleState, sizeof(pVehicle->ubVehicleState));
	fileRead(pFile, &pVehicle->isFacingRight, sizeof(pVehicle->isFacingRight));
	fileRead(pFile, &pVehicle->ubTrackFrame, sizeof(pVehicle->ubTrackFrame));
	fileRead(pFile, &pVehicle->fTrackAnimCnt, sizeof(pVehicle->fTrackAnimCnt));
	fileRead(pFile, &pVehicle->ubBodyShakeCnt, sizeof(pVehicle->ubBodyShakeCnt));
	fileRead(pFile, &pVehicle->ubJetShowFrame, sizeof(pVehicle->ubJetShowFrame));
	fileRead(pFile, &pVehicle->ubJetAnimFrame, sizeof(pVehicle->ubJetAnimFrame));
	fileRead(pFile, &pVehicle->ubJetAnimCnt, sizeof(pVehicle->ubJetAnimCnt));
	fileRead(pFile, &pVehicle->ubToolAnimCnt, sizeof(pVehicle->ubToolAnimCnt));
	fileRead(pFile, &pVehicle->ubDrillDir, sizeof(pVehicle->ubDrillDir));
	fileRead(pFile, &pVehicle->ubDrillVAnimCnt, sizeof(pVehicle->ubDrillVAnimCnt));
	fileRead(pFile, &pVehicle->fDrillDestX, sizeof(pVehicle->fDrillDestX));
	fileRead(pFile, &pVehicle->fDrillDestY, sizeof(pVehicle->fDrillDestY));
	fileRead(pFile, &pVehicle->fDrillDelta, sizeof(pVehicle->fDrillDelta));
	fileRead(pFile, &pVehicle->sDrillTile, sizeof(pVehicle->sDrillTile));
	fileRead(pFile, &pVehicle->ubSmokeAnimFrame, sizeof(pVehicle->ubSmokeAnimFrame));
	fileRead(pFile, &pVehicle->ubSmokeAnimCnt, sizeof(pVehicle->ubSmokeAnimCnt));
	fileRead(pFile, &pVehicle->ubTeleportAnimFrame, sizeof(pVehicle->ubTeleportAnimFrame));
	fileRead(pFile, &pVehicle->ubTeleportAnimCnt, sizeof(pVehicle->ubTeleportAnimCnt));
	fileRead(pFile, &pVehicle->uwTeleportX, sizeof(pVehicle->uwTeleportX));
	fileRead(pFile, &pVehicle->uwTeleportY, sizeof(pVehicle->uwTeleportY));
	fileRead(pFile, &pVehicle->ubDrillState, sizeof(pVehicle->ubDrillState));
	fileRead(pFile, &pVehicle->uwCargoCurr, sizeof(pVehicle->uwCargoCurr));
	fileRead(pFile, &pVehicle->uwCargoScore, sizeof(pVehicle->uwCargoScore));
	fileRead(pFile, pVehicle->pStock, sizeof(pVehicle->pStock));
	fileRead(pFile, &pVehicle->lCash, sizeof(pVehicle->lCash));
	fileRead(pFile, &pVehicle->uwDrillCurr, sizeof(pVehicle->uwDrillCurr));
	fileRead(pFile, &pVehicle->wHullCurr, sizeof(pVehicle->wHullCurr));
	fileRead(pFile, &pVehicle->ubPlayerIdx, sizeof(pVehicle->ubPlayerIdx));
	fileRead(pFile, &pVehicle->ubDestructionState, sizeof(pVehicle->ubDestructionState));
	fileRead(pFile, &pVehicle->sDynamite, sizeof(pVehicle->sDynamite));
	fileRead(pFile, &pVehicle->ubDamageFrames, sizeof(pVehicle->ubDamageFrames));
	return 1;
}

void vehicleCreate(tVehicle *pVehicle, UBYTE ubIdx) {
	logBlockBegin("vehicleCreate(pVehicle: %p, ubIdx: %hhu)", pVehicle, ubIdx);

	// Setup bobs
  bobInit(
		&pVehicle->sBobBody, VEHICLE_WIDTH, VEHICLE_BODY_HEIGHT, 1,
		bobCalcFrameAddress(s_pBodyFrames[ubIdx], 0),
		bobCalcFrameAddress(s_pBodyMask, 0),
		0, 0
	);
	bobInit(
		&pVehicle->sBobTrack, VEHICLE_WIDTH, VEHICLE_TRACK_HEIGHT, 1,
		bobCalcFrameAddress(s_pTrackFrames, 0),
		bobCalcFrameAddress(s_pTrackMask, 0),
		0, 0
	);
	bobInit(
		&pVehicle->sBobJet, VEHICLE_WIDTH, VEHICLE_FLAME_HEIGHT, 1,
		bobCalcFrameAddress(s_pJetFrames, 0),
		bobCalcFrameAddress(s_pJetMask, 0),
		0, 0
	);
	bobInit(
		&pVehicle->sBobTool, VEHICLE_TOOL_WIDTH, VEHICLE_TOOL_HEIGHT, 1,
		bobCalcFrameAddress(s_pToolFrames[ubIdx], 0),
		bobCalcFrameAddress(s_pToolMask, 0),
		0, 0
	);
	bobInit(
		&pVehicle->sBobWreck, VEHICLE_WRECK_WIDTH, VEHICLE_WRECK_HEIGHT, 1,
		bobCalcFrameAddress(s_pWreckFrames[ubIdx], 0),
		bobCalcFrameAddress(s_pWreckMask, 0),
		0, 0
	);
	bobInit(
		&pVehicle->sBobSmoke, VEHICLE_SMOKE_WIDTH, VEHICLE_SMOKE_FRAME_HEIGHT, 1,
		bobCalcFrameAddress(s_pSmokeFrames, 0),
		bobCalcFrameAddress(s_pSmokeMask, 0),
		0, 0
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

	// No vehicle rotating when other state than moving
	if(pVehicle->ubVehicleState == VEHICLE_STATE_MOVING) {
		if(bDirX > 0) {
			pVehicle->isFacingRight = 1;
		}
		else if(bDirX < 0) {
			pVehicle->isFacingRight = 0;
		}
	}

	// Update body roration as well as white frames
	vehicleUpdateBodyBob(pVehicle);
}

static inline void vehicleSetTool(
	tVehicle *pVehicle, tToolState eToolState, UBYTE ubFrame
) {
	pVehicle->sBobTool.sPos.ulYX = pVehicle->sBobBody.sPos.ulYX;
	pVehicle->sBobTool.sPos.uwY -= VEHICLE_TOOL_OFFSET_Y;
	UBYTE ubFrameOffsetY;
	if(eToolState == TOOL_STATE_IDLE) {
		ubFrameOffsetY = 0;
	}
	else { // if(eToolState == TOOL_STATE_DRILL)
		ubFrameOffsetY = ubFrame ? VEHICLE_TOOL_HEIGHT : 0;
	}

	if(pVehicle->isFacingRight) {
		pVehicle->sBobTool.sPos.uwX += 24+3;
	}
	else {
		pVehicle->sBobTool.sPos.uwX += -13+3;
		ubFrameOffsetY += (
			VEHICLE_TOOL_ANIM_FRAMES * VEHICLE_DESTRUCTION_FRAMES *
			VEHICLE_TOOL_HEIGHT
		);
	}

	// Apply destruction
	ubFrameOffsetY += (
		VEHICLE_TOOL_HEIGHT * VEHICLE_TOOL_ANIM_FRAMES *
		pVehicle->ubDestructionState
	);

	// Vertical drill anim
	UBYTE *pFrame;
	if(pVehicle->ubDamageFrames) {
		pFrame = s_pWhiteTool->Planes[0];
	}
	else {
		pFrame = bobCalcFrameAddress(s_pToolFrames[pVehicle->ubPlayerIdx], ubFrameOffsetY);
	}
	bobSetFrame(
		&pVehicle->sBobTool,
		pFrame,
		bobCalcFrameAddress(s_pToolMask, ubFrameOffsetY)
	);
}

static inline UBYTE vehicleStartDrilling(
	tVehicle *pVehicle, UWORD uwTileX, UWORD uwTileY, UBYTE ubDrillDir
) {
	if(pVehicle->isChallengeEnded) {
		return 0;
	}

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
				audioMixerPlaySfx(g_pSfxPenalty, SFX_CHANNEL_EFFECT, 1, 0);
				ubCooldown = 25;
			}
			else {
				--ubCooldown;
			}
			return 0;
		}
		pVehicle->uwDrillCurr -= ubDrillCost;
		gameElapseTime(ubDrillCost);
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
	audioMixerPlaySfx(g_pSfxDrill, SFX_CHANNEL_DRILL, 1, 1);

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

static void vehicleEndChallenge(tVehicle *pVehicle) {
	vehicleRestock(pVehicle, 0);
	pVehicle->isChallengeEnded = 1;
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
		audioMixerPlaySfx(g_pSfxOre, SFX_CHANNEL_EFFECT, 1, 0);
	}
	else if(ubTile == TILE_MAGMA_1 || ubTile == TILE_MAGMA_2) {
		vehicleHullDamage(pVehicle, 5 + (randUw(&g_sRand) & 0x7));
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
		planUnlockMineral(warehouseGetCurrentPlan(), ubMineralType);

		hudSetCargo(pVehicle->ubPlayerIdx, pVehicle->uwCargoCurr, uwCargoMax);
		char szMsg[40];
		const char *szMessage;
		UBYTE ubColor;
		if(pVehicle->uwCargoCurr == uwCargoMax) {
			szMessage = g_pMsgs[MSG_MISC_CARGO_FULL];
			ubColor = COLOR_REDEST;
			audioMixerPlaySfx(g_pSfxPenalty, SFX_CHANNEL_EFFECT, 1, 0);
		}
		else {
			char *pEnd = szMsg;
			pEnd = stringCopy(g_pMineralNames[g_pTileDefs[ubTile].ubMineral], pEnd);
			*(pEnd++) = ' ';
			*(pEnd++) = 'x';
			pEnd = stringDecimalFromULong(g_pTileDefs[ubTile].ubSlots, pEnd);
			szMessage = szMsg;
			ubColor = pMineral->ubTitleColor;
			audioMixerPlaySfx(g_pSfxOre, SFX_CHANNEL_EFFECT, 1, 0);
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
				vehicleEndChallenge(pVehicle);
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
			audioMixerPlaySfx(g_pSfxOre, SFX_CHANNEL_EFFECT, 1, 0);
		}
	}

	tileExcavate(uwTileX, uwTileY);
}

static void vehicleProcessMovement(tVehicle *pVehicle) {
	UBYTE isOnGround = 0;
	const fix16_t fMaxDx = 2 * fix16_one;
	const fix16_t fAccX = fix16_one / 16;
	const fix16_t fFrictX = fix16_one / 12;

	// Challenge camera teleport
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
		audioMixerPlaySfx(g_pSfxPenalty, SFX_CHANNEL_EFFECT, 1, 0);
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
	}

	if(g_isChallenge && !pVehicle->isChallengeEnded) {
		UWORD uwTileY = fix16_to_int(pVehicle->fY) / 32;
		if(uwTileY >= TILE_ROW_CHALLENGE_FINISH) {
			vehicleEndChallenge(pVehicle);
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
		pVehicle->fY += pVehicle->fDy;
		if(pVehicle->fY < F16(VEHICLE_TOOL_OFFSET_Y)) {
			pVehicle->fY = F16(VEHICLE_TOOL_OFFSET_Y);
			pVehicle->fDy = 0;
		}
		else if(tileIsSolid(uwTileCenter, uwTileTop)) {
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
			pVehicle->fTrackAnimCnt += fix16_abs(pVehicle->fDx);
			if(fix16_to_int(pVehicle->fTrackAnimCnt) >= 5) {
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
				UWORD uwOffsY = pVehicle->ubTrackFrame * VEHICLE_TRACK_HEIGHT;
				bobSetFrame(
					&pVehicle->sBobTrack,
					bobCalcFrameAddress(s_pTrackFrames, uwOffsY),
					bobCalcFrameAddress(s_pTrackMask, uwOffsY)
				);
				pVehicle->fTrackAnimCnt = 0;
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
			UWORD uwOffsetY = (pVehicle->sSteer.bY ? TRACK_OFFSET_JET : 0);
			bobSetFrame(
				&pVehicle->sBobTrack,
				bobCalcFrameAddress(s_pTrackFrames, uwOffsetY),
				bobCalcFrameAddress(s_pTrackMask, uwOffsetY)
			);
		}
		else if(pVehicle->ubJetShowFrame == 10) {
			// Update jet pos
			pVehicle->ubJetAnimCnt = (pVehicle->ubJetAnimCnt + 1) & 63;
			UWORD uwOffsetY = VEHICLE_FLAME_HEIGHT * (pVehicle->ubJetAnimCnt / 4);
			bobSetFrame(
				&pVehicle->sBobJet,
				bobCalcFrameAddress(s_pJetFrames, uwOffsetY),
				bobCalcFrameAddress(s_pJetMask, uwOffsetY)
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
			audioMixerPlaySfx(g_pSfxOre, SFX_CHANNEL_EFFECT, 1, 0);
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
		UWORD uwOffsY = VEHICLE_SMOKE_FRAME_HEIGHT * pVehicle->ubSmokeAnimFrame;
		bobSetFrame(
			&pVehicle->sBobSmoke,
			bobCalcFrameAddress(s_pSmokeFrames, uwOffsY),
			bobCalcFrameAddress(s_pSmokeMask, uwOffsY)
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

static void tryStopDrillAudio(void) {
	if(
		g_pVehicles[0].ubDrillState != DRILL_STATE_DRILLING &&
		(!g_is2pPlaying || g_pVehicles[1].ubDrillState != DRILL_STATE_DRILLING)
	) {
		audioMixerStopSfxOnChannel(SFX_CHANNEL_DRILL);
	}
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
					bobSetFrame(
						&pVehicle->sBobTrack,
						bobCalcFrameAddress(s_pTrackFrames, TRACK_OFFSET_DRILL),
						bobCalcFrameAddress(s_pTrackMask, TRACK_OFFSET_DRILL)
					);
				}
			}
			else {
				if(!--pVehicle->ubDrillVAnimCnt) {
					pVehicle->ubDrillDir = DRILL_DIR_NONE;
					pVehicle->ubDrillState = DRILL_STATE_OFF;
					pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;
				}
				else if(pVehicle->ubDrillVAnimCnt == 5) {
					bobSetFrame(
						&pVehicle->sBobTrack,
						bobCalcFrameAddress(s_pTrackFrames, TRACK_OFFSET_TRACK),
						bobCalcFrameAddress(s_pTrackMask, TRACK_OFFSET_TRACK)
					);
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
					pVehicle->ubDrillState = DRILL_STATE_OFF;
					pVehicle->ubVehicleState = VEHICLE_STATE_MOVING;
					tryStopDrillAudio();
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
						tryStopDrillAudio();
					}
				}
			}
			else {
				// Body shake
				pVehicle->sBobBody.sPos.uwX += randUw(&g_sRand) & 1;
				pVehicle->sBobBody.sPos.uwY += randUw(&g_sRand) & 1;

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
					UWORD uwOffsY = TRACK_OFFSET_DRILL + (ubAnim ?  VEHICLE_TRACK_HEIGHT : 0);
					bobSetFrame(
						&pVehicle->sBobTrack,
						bobCalcFrameAddress(s_pTrackFrames, uwOffsY),
						bobCalcFrameAddress(s_pTrackMask, uwOffsY)
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
	// UWORD uwMaxHealth = inventoryGetPartDef(INVENTORY_PART_HULL)->uwMax;
	// if(randUwMax(&g_sRand, 100) <= 5) {
	// 	vehicleHullDamage(pVehicle, uwMaxHealth);
	// }
	// else if(randUwMax(&g_sRand, 100) <= 20) {
	// 	vehicleHullDamage(pVehicle, uwMaxHealth / 2);
	// }
}

static void vehicleOnTeleportOutPeak(ULONG ulData) {
	tVehicle *pVehicle = (tVehicle*)ulData;
	pVehicle->ubTeleportAnimFrame = 0;
	pVehicle->ubTeleportAnimCnt = 0;
	vehicleSetPos(pVehicle, pVehicle->uwTeleportX, pVehicle->uwTeleportY);
	pVehicle->ubVehicleState = VEHICLE_STATE_TELEPORTING_IN;
	explosionAdd(
		pVehicle->uwTeleportX, pVehicle->uwTeleportY,
		vehicleOnTeleportInPeak, (ULONG)pVehicle, 0, EXPLOSION_KIND_TELEPORT
	);
}

void vehicleTeleport(tVehicle *pVehicle, UWORD uwX, UWORD uwY) {
	pVehicle->uwTeleportX = uwX;
	pVehicle->uwTeleportY = uwY;
	pVehicle->ubVehicleState = VEHICLE_STATE_TELEPORTING_OUT;
	explosionAdd(
		pVehicle->sBobBody.sPos.uwX, pVehicle->sBobBody.sPos.uwY,
		vehicleOnTeleportOutPeak, (ULONG)pVehicle, 0, EXPLOSION_KIND_TELEPORT
	);
}

tVehicle g_pVehicles[2];
