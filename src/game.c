/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include <ace/utils/custom.h>
#include <ace/managers/blit.h>
#include "vehicle.h"
#include "hud.h"
#include "tile.h"
#include "comm.h"
#include "comm_shop.h"
#include "comm_msg.h"
#include "menu.h"
#include "hi_score.h"
#include "ground_layer.h"
#include "base_tile.h"
#include "warehouse.h"
#include "tutorial.h"
#include "explosion.h"
#include "fade.h"
#include "pause.h"
#include "core.h"
#include "dino.h"
#include "debug.h"

typedef enum _tCameraType {
	CAMERA_TYPE_P1,
	CAMERA_TYPE_P2,
} tCameraType;

#define CAMERA_SPEED 4

tSample *g_pSampleDrill, *g_pSampleOre, *g_pSampleTeleport;

static tCameraType s_eCameraType = CAMERA_TYPE_P1;

static UBYTE s_ubChallengeCamCnt;
static tVPort *s_pVpMain;

tFont *g_pFont;
UBYTE g_is2pPlaying;
UBYTE g_is1pKbd, g_is2pKbd;
UBYTE g_isChallenge, g_isChallengeEnd, g_isAtari;

void gameTryPushBob(tBobNew *pBob) {
	if(
		pBob->sPos.uwY + pBob->uwHeight >= g_pMainBuffer->pCamera->uPos.uwY &&
		pBob->sPos.uwY < g_pMainBuffer->pCamera->uPos.uwY +  s_pVpMain->uwHeight
	) {
		bobNewPush(pBob);
	}
}

void gameStart(void) {
	s_ubChallengeCamCnt = 0;
	g_isChallengeEnd = 0;
	dinoReset();
	tutorialReset();
	tileInit(g_isAtari, g_isChallenge);
	warehouseReset(g_is2pPlaying);
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	hudReset(g_isChallenge, g_is2pPlaying);
	groundLayerReset(1);
	s_pVpMain = g_pMainBuffer->sCommon.pVPort;
}

static void gameProcessInput(void) {
	if(g_isChallengeEnd) {
		return;
	}
	if(keyUse(KEY_F1) && !g_isChallenge) {
		if(!g_is2pPlaying) {
			g_is2pPlaying = 1;
			hudSet2pPlaying(1);
			vehicleResetPos(&g_pVehicles[1]);
			g_pVehicles[1].fX = g_pVehicles[0].fX;
			g_pVehicles[1].fY = g_pVehicles[0].fY;
		}
		else {
			g_is2pPlaying = 0;
			hudSet2pPlaying(0);
		}
	}
	else if(keyUse(KEY_F2)) {
		g_is1pKbd = !g_is1pKbd;
	}
	else if(keyUse(KEY_F3)) {
		g_is2pKbd = !g_is2pKbd;
	}
	else if(keyUse(KEY_F4)) {
		if(s_eCameraType == CAMERA_TYPE_P1) {
			s_eCameraType = CAMERA_TYPE_P2;
		}
		else {
			s_eCameraType = CAMERA_TYPE_P1;
		}
	}

	BYTE bDirX = 0, bDirY = 0;
	if(g_is1pKbd) {
		if(keyCheck(KEY_D)) { bDirX += 1; }
		if(keyCheck(KEY_A)) { bDirX -= 1; }
		if(keyCheck(KEY_S)) { bDirY += 1; }
		if(keyCheck(KEY_W)) { bDirY -= 1; }
	}
	else {
		if(joyCheck(JOY1_RIGHT)) { bDirX += 1; }
		if(joyCheck(JOY1_LEFT)) { bDirX -= 1; }
		if(joyCheck(JOY1_DOWN)) { bDirY += 1; }
		if(joyCheck(JOY1_UP)) { bDirY -= 1; }
	}
	vehicleMove(&g_pVehicles[0], bDirX, bDirY);

	if(g_is2pPlaying) {
		bDirX = 0; bDirY = 0;
		if(g_is2pKbd) {
			if(keyCheck(KEY_RIGHT)) { bDirX += 1; }
			if(keyCheck(KEY_LEFT)) { bDirX -= 1; }
			if(keyCheck(KEY_DOWN)) { bDirY += 1; }
			if(keyCheck(KEY_UP)) { bDirY -= 1; }
		}
		else {
			if(joyCheck(JOY2_RIGHT)) { bDirX += 1; }
			if(joyCheck(JOY2_LEFT)) { bDirX -= 1; }
			if(joyCheck(JOY2_DOWN)) { bDirY += 1; }
			if(joyCheck(JOY2_UP)) { bDirY -= 1; }
		}
		vehicleMove(&g_pVehicles[1], bDirX, bDirY);
	}
}

static void gameCameraProcess(void) {
	if(g_isChallenge) {
		const UWORD uwBottomPos = g_pMainBuffer->pCamera->uPos.uwY + g_pMainBuffer->sCommon.pVPort->uwHeight - 2 * 32;
		if(
			g_pVehicles[0].sBobBody.sPos.uwY >  uwBottomPos ||
			(g_is2pPlaying && g_pVehicles[1].sBobBody.sPos.uwY > uwBottomPos)
		) {
			g_pMainBuffer->pCamera->uPos.uwY += 1;
		}
		else {
			++s_ubChallengeCamCnt;
			if(s_ubChallengeCamCnt >= 2) {
				g_pMainBuffer->pCamera->uPos.uwY += 1;
				s_ubChallengeCamCnt = 0;
			}
		}
	}
	else {
		UWORD uwCamDestY, uwCamDestX = 32;
		if(g_is2pPlaying && vehiclesAreClose()) {
			uwCamDestY = (
				fix16_to_int(g_pVehicles[0].fY) +
				fix16_to_int(g_pVehicles[1].fY) + VEHICLE_HEIGHT
			) / 2;
		}
		else if(g_is2pPlaying && s_eCameraType == CAMERA_TYPE_P2) {
			uwCamDestY = fix16_to_int(g_pVehicles[1].fY) + VEHICLE_HEIGHT / 2;
		}
		else {
			uwCamDestY = fix16_to_int(g_pVehicles[0].fY) + VEHICLE_HEIGHT / 2;
		}
		WORD wCameraDistance = (
			uwCamDestY - g_pMainBuffer->pCamera->sCommon.pVPort->uwHeight / 2
		) - g_pMainBuffer->pCamera->uPos.uwY;
		UWORD uwAbsDistance = ABS(wCameraDistance);
		if(uwAbsDistance > CAMERA_SPEED * 50 && fadeGetState() == FADE_STATE_IN) {
			fadeMorphTo(FADE_STATE_OUT);
		}

		if(uwAbsDistance > CAMERA_SPEED) {
			cameraMoveBy(g_pMainBuffer->pCamera, 0, SGN(wCameraDistance) * CAMERA_SPEED);
		}
		else {
			cameraMoveBy(g_pMainBuffer->pCamera, 0, wCameraDistance);
		}
		if(g_pMainBuffer->pCamera->uPos.uwX < uwCamDestX) {
			g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
		}

		if(fadeGetState() == FADE_STATE_OUT) {
			cameraCenterAt(g_pMainBuffer->pCamera, uwCamDestX, uwCamDestY);
			g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
			baseTileProcess();
			tileBufferRedrawAll(g_pMainBuffer);
			bobNewDiscardUndraw();
			g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
			fadeMorphTo(FADE_STATE_IN);
		}
	}
}

//-------------------------------------------------------------------- CHALLENGE

void gameChallengeEnd(void) {
	g_isChallengeEnd = 1;
}

void gameChallengeResult(void) {
	if(!g_is2pPlaying) {
		char szBfr[30];
		sprintf(szBfr, "Score: %ld", g_pVehicles[0].lCash);
		hiScoreSetup(g_pVehicles[0].lCash, szBfr);
		menuGsEnter(1);
	}
	else {
		// No entering hi score for 2 players, just summary who wins
		const char *pMsg;
		if(g_pVehicles[0].lCash > g_pVehicles[1].lCash) {
			pMsg = "Player 1 wins!";
		}
		else if(g_pVehicles[0].lCash < g_pVehicles[1].lCash) {
			pMsg = "Player 2 wins!";
		}
		else {
			pMsg = "Draw!";
		}
		hiScoreSetup(0, pMsg);
		menuGsEnter(1);
	}

}

//-------------------------------------------------------------------- GAMESTATE

void gameGsCreate(void) {

}

void gameGsLoop(void) {

  if(keyUse(KEY_ESCAPE) || keyUse(KEY_P)) {
		gameChangeState(pauseGsCreate, pauseGsLoop, pauseGsDestroy);
		return;
  }

	if(!g_isChallenge) {
		if(tutorialProcess()) {
			return;
		}
	}

	if(keyUse(KEY_B)) {
		debugToggle();
	}
	if(keyCheck(KEY_M)) {
		vPortWaitForEnd(s_pVpMain);
		vPortWaitForEnd(s_pVpMain);
		vPortWaitForEnd(s_pVpMain);
	}
	if(
		(keyUse(KEY_RETURN) || keyUse(KEY_SPACE)) &&
		vehicleIsNearShop(&g_pVehicles[0])
	) {
		gamePushState(commShopGsCreate, commShopGsLoop, commShopGsDestroy);
		return;
	}
	if(g_pVehicles[0].ubDrillDir == DRILL_DIR_NONE) {
		if(keyUse(KEY_K)) {
			dynamiteTrigger(
				&g_pVehicles[0].sDynamite,
				(g_pVehicles[0].sBobBody.sPos.uwX + VEHICLE_WIDTH / 2) >> 5,
				(g_pVehicles[0].sBobBody.sPos.uwY + VEHICLE_WIDTH / 2) >> 5,
				DYNAMITE_TYPE_HORZ
			);
		}
		else if(keyUse(KEY_L)) {
			dynamiteTrigger(
				&g_pVehicles[0].sDynamite,
				(g_pVehicles[0].sBobBody.sPos.uwX + VEHICLE_WIDTH / 2) >> 5,
				(g_pVehicles[0].sBobBody.sPos.uwY + VEHICLE_WIDTH / 2) >> 5,
				DYNAMITE_TYPE_3X3
			);
		}
		else if(keyUse(KEY_O)) {
			dynamiteTrigger(
				&g_pVehicles[0].sDynamite,
				(g_pVehicles[0].sBobBody.sPos.uwX + VEHICLE_WIDTH / 2) >> 5,
				(g_pVehicles[0].sBobBody.sPos.uwY + VEHICLE_WIDTH / 2) >> 5,
				DYNAMITE_TYPE_VERT
			);
		}
		else if(keyUse(KEY_1)) {
			vehicleTeleport(&g_pVehicles[0], 160, 220);
		}
		else if(keyUse(KEY_2)) {
			vehicleTeleport(&g_pVehicles[0], 160, 3428);
		}
	}

	debugColor(0x080);
	gameCameraProcess();
	gameProcessInput();
	vehicleProcessText();

	// Process plan being complete
	if(warehouseGetPlan()->wTimeRemaining <= 0 || keyUse(KEY_U)) {
		if(warehouseTryFulfillPlan()) {
			hudShowMessage(0, "We've fulfilled plan with your reserves.\nNew has arrived");
			warehouseNewPlan(1, g_is2pPlaying);
		}
		else {
			hudShowMessage(0, "Comrade, plan not done, warning and new plan");
			warehouseNewPlan(0, g_is2pPlaying);
		}
	}

	coreProcessBeforeBobs();
	debugColor(0x088);
	vehicleProcess(&g_pVehicles[0]);
	if(g_is2pPlaying) {
		debugColor(0x880);
		vehicleProcess(&g_pVehicles[1]);
	}
	debugColor(0x808);
	explosionManagerProcess();
	coreProcessAfterBobs();

	if(g_isChallengeEnd && g_pVehicles[0].ubDrillDir == DRILL_DIR_NONE && (
		!g_is2pPlaying || g_pVehicles[1].ubDrillDir == DRILL_DIR_NONE
	)) {
		gameChallengeResult();
	}
}

void gameGsDestroy(void) {

}
