/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
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
#include "steer.h"
#include "inventory.h"
#include "defs.h"

#define CAMERA_SPEED 4

typedef enum _tCameraType {
	CAMERA_TYPE_P1,
	CAMERA_TYPE_P2,
} tCameraType;

typedef struct _tModeSelection {
	tMode eMode;
	UBYTE isSelecting;
	tSteer eSteerFire;
	tSteer eSteerLeft, eSteerRight, eSteerUp, eSteerDown;
} tModeSelection;

static tModeSelection s_pModeSelection[2] = {
	{
		.eSteerFire = STEER_P1_FIRE,
		.eSteerLeft = STEER_P1_LEFT, .eSteerRight = STEER_P1_RIGHT,
		.eSteerUp = STEER_P1_UP, .eSteerDown = STEER_P1_DOWN
	},
	{
		.eSteerFire = STEER_P2_FIRE,
		.eSteerLeft = STEER_P2_LEFT, .eSteerRight = STEER_P2_RIGHT,
		.eSteerUp = STEER_P2_UP, .eSteerDown = STEER_P2_DOWN
	},
};

tSample *g_pSampleDrill, *g_pSampleOre, *g_pSamplePenalty;

static tCameraType s_eCameraType = CAMERA_TYPE_P1;

static UBYTE s_ubChallengeCamCnt;
static tVPort *s_pVpMain;
static UBYTE s_ubRebukes, s_ubAccolades, s_ubAccoladesFract;

tFont *g_pFont;
UBYTE g_is2pPlaying;
UBYTE g_is1pKbd, g_is2pKbd;
UBYTE g_isChallenge, g_isChallengeEnd, g_isAtari;
tStringArray g_sPlanMessages;
tBobNew g_pBombMarkers[3];

void gameTryPushBob(tBobNew *pBob) {
	if(
		pBob->sPos.uwY + pBob->uwHeight >= g_pMainBuffer->pCamera->uPos.uwY &&
		pBob->sPos.uwY < g_pMainBuffer->pCamera->uPos.uwY +  s_pVpMain->uwHeight
	) {
		bobNewPush(pBob);
	}
}

void modeReset(UBYTE ubPlayer) {
	s_pModeSelection[ubPlayer].isSelecting = 0;
	s_pModeSelection[ubPlayer].eMode = MODE_DRILL;
	hudSetMode(ubPlayer, MODE_DRILL);
}

void gameStart(void) {
	s_ubChallengeCamCnt = 0;
	g_isChallengeEnd = 0;
	dinoReset();
	tutorialReset();
	tileInit(g_isAtari, g_isChallenge);
	warehouseReset(g_is2pPlaying);
	inventoryReset();
	modeReset(0);
	modeReset(1);
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	s_ubRebukes = 0;
	s_ubAccolades = 0;
	s_ubAccoladesFract = 0;
	for(tMode eMode = 0; eMode < MODE_COUNT; ++eMode) {
		hudSetModeCounter(eMode, 0);
	}
	hudReset(g_isChallenge, g_is2pPlaying);
	groundLayerReset(1);
	s_pVpMain = g_pMainBuffer->sCommon.pVPort;
}

static void gameProcessHotkeys(void) {
  if(keyUse(KEY_ESCAPE) || keyUse(KEY_P)) {
		gameChangeState(pauseGsCreate, pauseGsLoop, pauseGsDestroy);
		return;
  }
	if(keyUse(KEY_B)) {
		debugToggle();
	}
	if(keyCheck(KEY_M)) {
		vPortWaitForEnd(s_pVpMain);
		vPortWaitForEnd(s_pVpMain);
		vPortWaitForEnd(s_pVpMain);
	}

	if(keyUse(KEY_F1) && !g_isChallenge) {
		if(!g_is2pPlaying) {
			g_is2pPlaying = 1;
			hudSet2pPlaying(1);
			modeReset(1);
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
}

static const UWORD s_pBaseTeleportY[2] = {220, 3428};

static UBYTE s_pBombCount[2];
static tBombDir s_pLastDir[2];

static void addBombInDir(UBYTE ubPlayer, tBombDir eDir, tBombDir eOpposite) {
	if(s_pLastDir[ubPlayer] == eOpposite && s_pBombCount[ubPlayer]) {
		// opposite dir
		--s_pBombCount[ubPlayer];
	}
	else if(s_pLastDir[ubPlayer] == eDir) {
		// same dir
		if(s_pBombCount[ubPlayer] < 3) {
			++s_pBombCount[ubPlayer];
		}
	}
	else {
		// other dir
		s_pBombCount[ubPlayer] = 1;
		s_pLastDir[ubPlayer] = eDir;
	}
}

static void gameProcessModeTnt(UBYTE ubPlayer) {
	tModeSelection *pSelection = &s_pModeSelection[ubPlayer];
	if(steerUse(pSelection->eSteerLeft)) {
		addBombInDir(ubPlayer, BOMB_DIR_LEFT, BOMB_DIR_RIGHT);
	}
	else if(steerUse(pSelection->eSteerRight)) {
		addBombInDir(ubPlayer, BOMB_DIR_RIGHT, BOMB_DIR_LEFT);
	}
	else if(steerUse(pSelection->eSteerUp)) {
		addBombInDir(ubPlayer, BOMB_DIR_UP, BOMB_DIR_DOWN);
	}
	else if(steerUse(pSelection->eSteerDown)) {
		addBombInDir(ubPlayer, BOMB_DIR_DOWN, BOMB_DIR_UP);
	}
	else if(steerUse(pSelection->eSteerFire)) {
		UBYTE ubTntCount = inventoryGetItemDef(INVENTORY_ITEM_TNT)->ubCount;
		ubTntCount -= dynamiteTrigger(
			&g_pVehicles[ubPlayer].sDynamite,
			(g_pVehicles[ubPlayer].sBobBody.sPos.uwX + VEHICLE_WIDTH / 2) >> 5,
			(g_pVehicles[ubPlayer].sBobBody.sPos.uwY + VEHICLE_WIDTH / 2) >> 5,
			MIN(s_pBombCount[ubPlayer], ubTntCount), s_pLastDir[ubPlayer]
		);
		inventorySetItemCount(INVENTORY_ITEM_TNT, ubTntCount);
		hudSetModeCounter(MODE_TNT, ubTntCount);
		s_pBombCount[ubPlayer] = 0;
		s_pLastDir[ubPlayer] = BOMB_DIR_NONE;
		s_pModeSelection[ubPlayer].eMode = MODE_DRILL;
	}
}

static void gameProcessNuke(UBYTE ubPlayer) {
	tModeSelection *pSelection = &s_pModeSelection[ubPlayer];
	pSelection->eMode = MODE_DRILL;
}

static void gameDisplayModeTnt(UBYTE ubPlayer) {
	if(
		s_pModeSelection[ubPlayer].eMode != MODE_TNT ||
		s_pModeSelection[ubPlayer].isSelecting
	) {
		return;
	}
	UWORD uwTileX = (g_pVehicles[ubPlayer].sBobBody.sPos.uwX + VEHICLE_WIDTH / 2) >> 5;
	UWORD uwTileY = (g_pVehicles[ubPlayer].sBobBody.sPos.uwY + VEHICLE_WIDTH / 2) >> 5;
	BYTE bDeltaX = 0, bDeltaY = 0;
	switch(s_pLastDir[ubPlayer]) {
		case BOMB_DIR_LEFT:
			bDeltaX = -1;
			break;
		case BOMB_DIR_RIGHT:
			bDeltaX = 1;
			break;
		case BOMB_DIR_UP:
			bDeltaY = -1;
			break;
		case BOMB_DIR_DOWN:
			bDeltaY = 1;
			break;
		case BOMB_DIR_NONE:
		default:
			return;
	}
	for(UBYTE i = 0; i < s_pBombCount[ubPlayer]; ++i) {
		uwTileX += bDeltaX;
		uwTileY += bDeltaY;
		if(1 <= uwTileX && uwTileX <= 10) {
			g_pBombMarkers[i].sPos.uwX = (uwTileX << 5) + 8;
			g_pBombMarkers[i].sPos.uwY = (uwTileY << 5) + 11;
			gameTryPushBob(&g_pBombMarkers[i]);
		}
		else {
			break;
		}
	}
}

static void gameProcessModeTeleport(UBYTE ubPlayer) {
	tModeSelection *pSelection = &s_pModeSelection[ubPlayer];
	vehicleTeleport(&g_pVehicles[ubPlayer], 160, s_pBaseTeleportY[0]);
	pSelection->eMode = MODE_DRILL;
	hudSetMode(0, pSelection->eMode);
}

static UBYTE gameProcessModeDrill(UBYTE ubPlayer) {
	if(g_isChallengeEnd) {
		return 0;
	}
	tModeSelection *pSelection = &s_pModeSelection[ubPlayer];

	if(steerUse(pSelection->eSteerFire)) {
		if(!pSelection->isSelecting) {
			if(vehicleIsNearShop(&g_pVehicles[ubPlayer])) {
				gamePushState(commShopGsCreate, commShopGsLoop, commShopGsDestroy);
				return 1;
			}
			pSelection->isSelecting = 1;
			hudShowMode();
		}
		else {
			pSelection->isSelecting = 0;
			hudHideMode();
			if(pSelection->eMode == MODE_TNT) {
				s_pBombCount[ubPlayer] = 0;
				s_pLastDir[ubPlayer] = BOMB_DIR_NONE;
			}
		}
	}
	if(pSelection->isSelecting) {
		if(steerUse(pSelection->eSteerLeft)) {
			if(pSelection->eMode > 0) {
				--pSelection->eMode;
			}
			else {
				pSelection->eMode = MODE_COUNT - 1;
			}
		}
		else if(steerUse(pSelection->eSteerRight)) {
			if(pSelection->eMode < MODE_COUNT - 1) {
			++pSelection->eMode;
			}
			else {
				pSelection->eMode = 0;
			}
		}
		hudSetMode(ubPlayer, pSelection->eMode);
	}
	else {
		BYTE bDirX = 0, bDirY = 0;
		if(steerGet(pSelection->eSteerRight)) { bDirX += 1; }
		if(steerGet(pSelection->eSteerLeft)) { bDirX -= 1; }
		if(steerGet(pSelection->eSteerDown)) { bDirY += 1; }
		if(steerGet(pSelection->eSteerUp)) { bDirY -= 1; }
		vehicleMove(&g_pVehicles[ubPlayer], bDirX, bDirY);
	}
	return 0;
}

static UBYTE gameProcessSteer(UBYTE ubPlayer) {
	UBYTE isReturnImmediately = 0;
	if((ubPlayer == 0 || g_is2pPlaying)) {
		if(s_pModeSelection[ubPlayer].isSelecting) {
			isReturnImmediately = gameProcessModeDrill(ubPlayer);
		}
		else {
			switch(s_pModeSelection[ubPlayer].eMode) {
				case MODE_DRILL:
					isReturnImmediately = gameProcessModeDrill(ubPlayer);
					break;
				case MODE_TNT:
					gameProcessModeTnt(ubPlayer);
					break;
				case MODE_NUKE:
					gameProcessNuke(ubPlayer);
					break;
				case MODE_TELEPORT:
					gameProcessModeTeleport(ubPlayer);
					break;
				default:
					break;
			}
		}
	}
	return isReturnImmediately;
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
			groundLayerReset(groundLayerGetLowerAtDepth(g_pMainBuffer->pCamera->uPos.uwY));
			tileBufferRedrawAll(g_pMainBuffer);
			bobNewDiscardUndraw();
			g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
			fadeMorphTo(FADE_STATE_IN);
		}
	}
}

void gameAddAccolade(void) {
	if(++s_ubAccoladesFract >= g_ubPlansPerAccolade) {
		s_ubAccoladesFract = 0;
		++s_ubAccolades;
	}
}

void gameAddRebuke(void) {
	++s_ubRebukes;
}

UBYTE gameGetAccolades(void) {
	return s_ubAccolades;
}

UBYTE gameGetRebukes(void) {
	return s_ubRebukes;
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
	if(!g_isChallenge) {
		if(tutorialProcess()) {
			return;
		}
	}

	debugColor(0x080);
	gameCameraProcess();
	steerUpdateFromInput(g_is1pKbd, g_is2pKbd);
	gameProcessHotkeys();
	UBYTE isGameStateChange = gameProcessSteer(0) | gameProcessSteer(1);
	if(isGameStateChange) {
		return;
	}
	vehicleProcessText();

	// Process plan being complete
	if(warehouseGetPlan()->wTimeRemaining <= 0 || keyUse(KEY_U)) {
		if(warehouseTryFulfillPlan()) {
			hudShowMessage(0, g_sPlanMessages.pStrings[0]);
			warehouseNewPlan(1, g_is2pPlaying);
			gameAddAccolade();
		}
		else {
			hudShowMessage(0, g_sPlanMessages.pStrings[1]);
			warehouseNewPlan(0, g_is2pPlaying);
			gameAddRebuke();
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
	gameDisplayModeTnt(0);
	gameDisplayModeTnt(1);
	coreProcessAfterBobs();

	if(g_isChallengeEnd && g_pVehicles[0].ubDrillDir == DRILL_DIR_NONE && (
		!g_is2pPlaying || g_pVehicles[1].ubDrillDir == DRILL_DIR_NONE
	)) {
		gameChallengeResult();
	}
}

void gameGsDestroy(void) {

}
