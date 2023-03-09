/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/blit.h>
#include <ace/managers/system.h>
#include <ace/utils/custom.h>
#include <comm/gs_shop.h>
#include <comm/page_office.h>
#include <comm/inbox.h>
#include "vehicle.h"
#include "hud.h"
#include "tile.h"
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
#include "inventory.h"
#include "defs.h"
#include "save.h"
#include "settings.h"

#define CAMERA_SPEED 4

typedef enum _tCameraType {
	CAMERA_TYPE_P1,
	CAMERA_TYPE_P2,
} tCameraType;

typedef struct _tModeSelection {
	tMode eMode;
	UBYTE isSelecting;
} tModeSelection;

static tModeSelection s_pModeSelection[2] = {};

tPtplayerSfx *g_pSfxDrill, *g_pSfxOre, *g_pSfxPenalty;
tPtplayerMod *g_pGameMods[GAME_MOD_COUNT];
UBYTE g_is2pPlaying;
UBYTE g_isChallenge, g_isAtari;
tBobNew g_pBombMarkers[3];

static const UWORD s_pBaseTeleportY[2] = {220, 3428};

static UBYTE s_pBombCount[2];
static tBombDir s_pLastDir[2];
static tSteer s_pPlayerSteers[2];
static tCameraType s_eCameraType = CAMERA_TYPE_P1;
static UBYTE s_ubChallengeCamCnt;
static tVPort *s_pVpMain;
static UBYTE s_ubRebukes, s_ubAccolades, s_ubAccoladesFract;
static UBYTE s_isReminderShown;
static UBYTE s_ubCurrentMod;
static ULONG s_ulGameTime;

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

void gameStart(UBYTE isChallenge, tSteer sSteerP1, tSteer sSteerP2) {
	s_ubChallengeCamCnt = 0;
	g_isChallenge = isChallenge;
	s_pPlayerSteers[0] = sSteerP1;
	s_pPlayerSteers[1] = sSteerP2;
	inboxReset();
	dinoReset();
	tutorialReset();
	pageOfficeReset();
	tileReset(g_isAtari, g_isChallenge);
	warehouseReset();
	inventoryReset();
	modeReset(0);
	modeReset(1);
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	s_ubRebukes = 0;
	s_ubAccolades = 0;
	s_ubAccoladesFract = 0;
	s_isReminderShown = 0;
	for(tMode eMode = 0; eMode < MODE_COUNT; ++eMode) {
		hudSetModeCounter(eMode, 0);
	}
	hudReset(g_isChallenge, g_is2pPlaying);
	groundLayerReset(1);
	s_pVpMain = g_pMainBuffer->sCommon.pVPort;
}

void gameTriggerSave(void) {
	logWrite("game save");
	systemUse();
	tFile *pSave = fileOpen("save_story.tmp", "wb");
	gameSave(pSave);
	fileClose(pSave);
	fileDelete("save_story.dat");
	fileMove("save_story.tmp", "save_story.dat");
	systemUnuse();
}

static void gameProcessHotkeys(void) {
  if(keyUse(KEY_ESCAPE) || keyUse(KEY_P)) {
		stateChange(g_pGameStateManager, &g_sStatePause);
		return;
  }
	if(keyUse(KEY_N)) {
		gameTriggerSave();
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
		g_sSettings.is1pKbd = !g_sSettings.is1pKbd;
	}
	else if(keyUse(KEY_F3)) {
		g_sSettings.is2pKbd = !g_sSettings.is2pKbd;
	}
	else if(keyUse(KEY_F4)) {
		if(s_eCameraType == CAMERA_TYPE_P1) {
			s_eCameraType = CAMERA_TYPE_P2;
		}
		else {
			s_eCameraType = CAMERA_TYPE_P1;
		}
	}
	else if(keyUse(KEY_MINUS)) {
		gameElapseDay();
	}
	else if(keyUse(KEY_EQUALS)) {
		hudShowMessage(0, g_pMsgs[MSG_HUD_NEW_PLAN]);
		warehouseNextPlan(NEXT_PLAN_REASON_FULFILLED);
	}
	else if(keyUse(KEY_0)) {
		tPlan *pPlan = warehouseGetCurrentPlan();
		gameElapseTime(pPlan->wTimeRemaining);
	}
}

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
	if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_LEFT)) {
		addBombInDir(ubPlayer, BOMB_DIR_LEFT, BOMB_DIR_RIGHT);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_RIGHT)) {
		addBombInDir(ubPlayer, BOMB_DIR_RIGHT, BOMB_DIR_LEFT);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_UP)) {
		addBombInDir(ubPlayer, BOMB_DIR_UP, BOMB_DIR_DOWN);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_DOWN)) {
		addBombInDir(ubPlayer, BOMB_DIR_DOWN, BOMB_DIR_UP);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_FIRE)) {
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
	tModeSelection *pSelection = &s_pModeSelection[ubPlayer];

	if(!g_isChallenge) {
		if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_FIRE)) {
			if(!pSelection->isSelecting) {
				if(vehicleIsNearShop(&g_pVehicles[ubPlayer])) {
					statePush(g_pGameStateManager, &g_sStateShop);
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
		else if(inboxIsUrgent() && vehicleIsNearShop(&g_pVehicles[ubPlayer])) {
			statePush(g_pGameStateManager, &g_sStateShop);
			return 1;
		}
	}

	if(pSelection->isSelecting) {
		if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_LEFT)) {
			if(pSelection->eMode > 0) {
				--pSelection->eMode;
			}
			else {
				pSelection->eMode = MODE_COUNT - 1;
			}
		}
		else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_RIGHT)) {
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
		if(!g_pVehicles[ubPlayer].isChallengeEnded) {
			if(steerDirCheck(&s_pPlayerSteers[ubPlayer], DIRECTION_RIGHT)) { bDirX += 1; }
			if(steerDirCheck(&s_pPlayerSteers[ubPlayer], DIRECTION_LEFT)) { bDirX -= 1; }
			if(steerDirCheck(&s_pPlayerSteers[ubPlayer], DIRECTION_DOWN)) { bDirY += 1; }
			if(steerDirCheck(&s_pPlayerSteers[ubPlayer], DIRECTION_UP)) { bDirY -= 1; }
		}
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

void gameAdvanceAccolade(void) {
	if(++s_ubAccoladesFract >= g_ubPlansPerAccolade) {
		s_ubAccoladesFract = 0;
		gameAddAccolade();
	}
}

void gameAddAccolade(void) {
	++s_ubAccolades;

	if(s_ubAccolades >= g_ubAccoladesInMainStory) {
		inboxPushBack(COMM_SHOP_PAGE_NEWS_ACCOLADES, 0);
	}
}

void gameAddRebuke(void) {
	if(s_ubRebukes < g_ubRebukesInMainStory) {
		++s_ubRebukes;
	}
	tCommShopPage ePage = CLAMP(
		COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_1 + s_ubRebukes - 1,
		COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_1,
		COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_3
	);

	pageOfficeTryUnlockPersonSubpage(FACE_ID_KOMISARZ, ePage);
	inboxPushBack(ePage, 1);
}

UBYTE gameGetAccolades(void) {
	return s_ubAccolades;
}

UBYTE gameGetRebukes(void) {
	return s_ubRebukes;
}

//-------------------------------------------------------------------- CHALLENGE

void gameChallengeResult(void) {
	if(!g_is2pPlaying) {
		char szBfr[30];
		sprintf(
			szBfr, "%s: %ld",
			g_pMsgs[MSG_HI_SCORE_WIN_SCORE], g_pVehicles[0].lCash
		);
		hiScoreSetup(g_pVehicles[0].lCash, szBfr);
		menuGsEnter(1);
	}
	else {
		// No entering hi score for 2 players, just summary who wins
		const char *pMsg;
		if(g_pVehicles[0].lCash > g_pVehicles[1].lCash) {
			pMsg = g_pMsgs[MSG_HI_SCORE_WIN_P1];
		}
		else if(g_pVehicles[0].lCash < g_pVehicles[1].lCash) {
			pMsg = g_pMsgs[MSG_HI_SCORE_WIN_P2];
		}
		else {
			pMsg = g_pMsgs[MSG_HI_SCORE_DRAW];
		}
		hiScoreSetup(0, pMsg);
		menuGsEnter(1);
	}
}

static void onSongEnd(void) {
	if(++s_ubCurrentMod >= GAME_MOD_COUNT) {
		s_ubCurrentMod = 0;
	}
	ptplayerLoadMod(g_pGameMods[s_ubCurrentMod], g_pModSampleData, 0);
	ptplayerEnableMusic(1);
}

static void processPlan(void) {
	tPlan *pPlan = warehouseGetCurrentPlan();
	if(!pPlan->isActive) {
		return;
	}

	WORD wRemainingDays = planGetRemainingDays(pPlan);
	if(wRemainingDays <= 0) {
		if(!pPlan->isPenaltyCountdownStarted && !pPlan->isExtendedTimeByFavor) {
			char szBfr[100];
			sprintf(szBfr, g_pMsgs[MSG_HUD_PLAN_EXTENDING], 14);
			hudShowMessage(0, szBfr);
			planStartPenaltyCountdown(pPlan);
		}
		else {
			hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_REBUKE]);
			warehouseNextPlan(NEXT_PLAN_REASON_FAILED);
			gameAddRebuke();
		}
	}
	else if(
		wRemainingDays == 15 || wRemainingDays == 5 || wRemainingDays == 3 ||
		wRemainingDays == 2 || wRemainingDays == 1
	) {
		if(!s_isReminderShown) {
			s_isReminderShown = 1;
			char szBuffer[50];
			sprintf(szBuffer, g_pMsgs[MSG_HUD_PLAN_REMAINING], wRemainingDays);
			hudShowMessage(0, szBuffer);
		}
	}
	else {
		s_isReminderShown = 0;
	}
}

void gameElapseTime(UWORD uwTime) {
	if(ULONG_MAX - s_ulGameTime > uwTime) {
		s_ulGameTime += uwTime;
	}
	else {
		s_ulGameTime = ULONG_MAX;
	}

	tPlan *pPlan = warehouseGetCurrentPlan();
	planElapseTime(pPlan, uwTime);
	if(!pPlan->isActive && pPlan->wTimeRemaining == 0 && pPlan->uwIndex > 0) {
		// first plan start (index 0) is handled by tutorial
		hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_HUD_NEW_PLAN]);
		planStart(pPlan);
	}
}

void gameElapseDay(void) {
	gameElapseTime(GAME_TIME_PER_DAY);
}

ULONG gameGetTime(void) {
	return s_ulGameTime;
}

UBYTE gameIsElapsedDays(ULONG ulStart, UBYTE ubDays) {
	if(s_ulGameTime - ulStart >= ubDays * GAME_TIME_PER_DAY) {
		return 1;
	}
	return 0;
}

tSteer *gameGetSteers(void) {
	return s_pPlayerSteers;
}

void gameSave(tFile *pFile) {
	saveWriteHeader(pFile, "GAME");
	fileWrite(pFile, &g_is2pPlaying, sizeof(g_is2pPlaying));
	fileWrite(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileWrite(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileWrite(pFile, &g_isChallenge, sizeof(g_isChallenge));
	fileWrite(pFile, &g_isAtari, sizeof(g_isAtari));
	// for(UBYTE i = 0; i < 3; ++i) {
	// 	bobNewSave(pFile, g_pBombMarkers[i]);
	// }

	fileWrite(pFile, s_pBombCount, sizeof(s_pBombCount));
	fileWrite(pFile, s_pLastDir, sizeof(s_pLastDir));
	fileWrite(pFile, &s_eCameraType, sizeof(s_eCameraType));
	fileWrite(pFile, &s_ubChallengeCamCnt, sizeof(s_ubChallengeCamCnt));

	fileWrite(pFile, &s_ubRebukes, sizeof(s_ubRebukes));
	fileWrite(pFile, &s_ubAccolades, sizeof(s_ubAccolades));
	fileWrite(pFile, &s_ubAccoladesFract, sizeof(s_ubAccoladesFract));
	fileWrite(pFile, &s_isReminderShown, sizeof(s_isReminderShown));
	fileWrite(pFile, &s_ubCurrentMod, sizeof(s_ubCurrentMod));
	fileWrite(pFile, &s_ulGameTime, sizeof(s_ulGameTime));

	inboxSave(pFile);
	dinoSave(pFile);
	tutorialSave(pFile);
	pageOfficeSave(pFile);
	tileSave(pFile);
	warehouseSave(pFile);
	inventorySave(pFile);
	vehicleSave(&g_pVehicles[0], pFile);
	vehicleSave(&g_pVehicles[1], pFile);
	hudSave(pFile);
}

UBYTE gameLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "GAME")) {
		return 0;
	}

	fileRead(pFile, &g_is2pPlaying, sizeof(g_is2pPlaying));
	fileRead(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileRead(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileRead(pFile, &g_isChallenge, sizeof(g_isChallenge));
	fileRead(pFile, &g_isAtari, sizeof(g_isAtari));
	// for(UBYTE i = 0; i < 3; ++i) {
	// 	bobNewLoad(pFile, g_pBombMarkers[i]);
	// }

	fileRead(pFile, s_pBombCount, sizeof(s_pBombCount));
	fileRead(pFile, s_pLastDir, sizeof(s_pLastDir));
	fileRead(pFile, &s_eCameraType, sizeof(s_eCameraType));
	fileRead(pFile, &s_ubChallengeCamCnt, sizeof(s_ubChallengeCamCnt));

	fileRead(pFile, &s_ubRebukes, sizeof(s_ubRebukes));
	fileRead(pFile, &s_ubAccolades, sizeof(s_ubAccolades));
	fileRead(pFile, &s_ubAccoladesFract, sizeof(s_ubAccoladesFract));
	fileRead(pFile, &s_isReminderShown, sizeof(s_isReminderShown));
	fileRead(pFile, &s_ubCurrentMod, sizeof(s_ubCurrentMod));
	fileRead(pFile, &s_ulGameTime, sizeof(s_ulGameTime));

	return inboxLoad(pFile) &&
		dinoLoad(pFile) &&
		tutorialLoad(pFile) &&
		pageOfficeLoad(pFile) &&
		tileLoad(pFile) &&
		warehouseLoad(pFile) &&
		inventoryLoad(pFile) &&
		vehicleLoad(&g_pVehicles[0], pFile) &&
		vehicleLoad(&g_pVehicles[1], pFile) &&
		hudLoad(pFile);
}

//-------------------------------------------------------------------- GAMESTATE

static void gameGsCreate(void) {
	s_ulGameTime = 0;
	s_ubCurrentMod = GAME_MOD_COUNT;
	ptplayerConfigureSongRepeat(0, onSongEnd);
	onSongEnd();
}

static void gameGsLoop(void) {
	if(tutorialProcess()) {
		return;
	}
	dinoProcess();

	debugColor(0x080);
	gameCameraProcess();
	steerProcess(&s_pPlayerSteers[0]);
	steerProcess(&s_pPlayerSteers[1]);
	gameProcessHotkeys();
	UBYTE isGameStateChange = gameProcessSteer(0) | gameProcessSteer(1);
	if(isGameStateChange) {
		return;
	}
	vehicleProcessText();

	processPlan();
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

	if(
		g_pVehicles[0].isChallengeEnded &&
		(!g_is2pPlaying || g_pVehicles[1].isChallengeEnded)
	) {
		gameChallengeResult();
	}
}

static void gameGsDestroy(void) {
	ptplayerStop();
}

tState g_sStateGame = {
	.cbCreate = gameGsCreate, .cbLoop = gameGsLoop, .cbDestroy = gameGsDestroy
};
