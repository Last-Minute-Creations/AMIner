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
#include "base.h"
#include "warehouse.h"
#include "tutorial.h"
#include "explosion.h"
#include "fade.h"
#include "pause.h"
#include "core.h"
#include "dino.h"
#include "quest_gate.h"
#include "debug.h"
#include "inventory.h"
#include "defs.h"
#include "save.h"
#include "settings.h"
#include "collectibles.h"
#include "heat.h"
#include "assets.h"
#include "mode_menu.h"
#include "twister.h"

#define CAMERA_SPEED 4
#define CAMERA_SHAKE_AMPLITUDE 2

//------------------------------------------------------------------------ TYPES

typedef enum _tCameraType {
	CAMERA_TYPE_P1,
	CAMERA_TYPE_P2,
} tCameraType;

typedef enum tModePreset {
	MODE_PRESET_DEFAULT,
	MODE_PRESET_SHOP,
	MODE_PRESET_FAST_TRAVEL,
	MODE_PRESET_COUNT,
} tModePreset;

typedef enum tGateCutsceneStep {
	GATE_CUTSCENE_STEP_OFF,
	// TODO: wait for camera?
	GATE_CUTSCENE_STEP_START,
	GATE_CUTSCENE_STEP_WAIT_FOR_SHAKE,
	GATE_CUTSCENE_STEP_SHAKE_BEFORE_TWISTER,
	GATE_CUTSCENE_STEP_TWIST_BEFORE_FADE,
	GATE_CUTSCENE_STEP_FADE_OUT,
	GATE_CUTSCENE_STEP_FADE_IN,
	GATE_CUTSCENE_STEP_END,
} tGateCutsceneStep;

//----------------------------------------------------------------- PRIVATE VARS

static tBob s_pPlayerBombMarkers[2][3];
static tModePreset s_pPlayerModePreset[2];

static const UWORD s_pBaseTeleportY[2] = {220, 3428};
static tUwCoordYX s_sTeleportReturn;

static tSteer s_pPlayerSteers[2];
static tCameraType s_eCameraType = CAMERA_TYPE_P1;
static UBYTE s_ubChallengeCamCnt;
static tVPort *s_pVpMain;
static UBYTE s_ubRebukes, s_ubAccolades, s_ubAccoladesFract;
static WORD s_wLastReminder;
static UBYTE s_ubCurrentMod;
static ULONG s_ulGameTime;
static UWORD s_uwMaxTileY;
static tModeMenu s_pModeMenus[2];
static tGateCutsceneStep s_eGateCutsceneStep;
static UBYTE s_isCameraShake;

//------------------------------------------------------------------ PUBLIC VARS

UBYTE g_is2pPlaying;
UBYTE g_isChallenge;
UBYTE g_isAtari;

//------------------------------------------------------------ PRIVATE FNS: MODE
// TODO: reformat somehow? Move to separate file?

static UBYTE gameChangeModePreset(UBYTE ubPlayerIndex, tModePreset ePreset) {
	if(s_pPlayerModePreset[ubPlayerIndex] == ePreset) {
		return 0;
	}

	s_pPlayerModePreset[ubPlayerIndex] = ePreset;
	tModeMenu *pModeMenu = &s_pModeMenus[ubPlayerIndex];
	modeMenuClearOptions(pModeMenu);
	switch(ePreset) {
		case MODE_PRESET_SHOP:
			modeMenuAddOption(pModeMenu, MODE_OPTION_EXCLAMATION);
			break;
		case MODE_PRESET_DEFAULT:
			modeMenuAddOption(pModeMenu, MODE_OPTION_DRILL);
			if(inventoryGetPartDef(INVENTORY_PART_TNT)->ubLevel) {
				modeMenuAddOption(pModeMenu, MODE_OPTION_TNT);
			}
			// TODO: check conditions for teleport
			// if(inventoryGetPartDef(INVENTORY_PART_TELEPORT)->ubLevel) {
			modeMenuAddOption(pModeMenu, MODE_OPTION_TELEPORT);
			// }
			break;
		case MODE_PRESET_FAST_TRAVEL:
			break;
		default:
			break;
	}
	return 1;
}

static void gameResetModePreset(UBYTE ubPlayerIndex) {
	// set to  anything else to trigger change
	s_pPlayerModePreset[ubPlayerIndex] = MODE_PRESET_SHOP;

	gameChangeModePreset(ubPlayerIndex, MODE_PRESET_DEFAULT);
}

static void gameProcessModeTnt(UBYTE ubPlayer) {
	tTnt *pTnt = &g_pVehicles[ubPlayer].sDynamite;
	if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_LEFT)) {
		tntAdd(pTnt, DIRECTION_LEFT);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_RIGHT)) {
		tntAdd(pTnt, DIRECTION_RIGHT);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_UP)) {
		tntAdd(pTnt, DIRECTION_UP);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_DOWN)) {
		tntAdd(pTnt, DIRECTION_DOWN);
	}
	else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_FIRE)) {
		tntDetonate(pTnt);
		s_pModeMenus[ubPlayer].ubCurrent = 0;
	}
}

static void gameDisplayModeTnt(UBYTE ubPlayer) {
	if(
		s_pModeMenus[ubPlayer].isActive ||
		modeMenuGetSelected(&s_pModeMenus[ubPlayer]) != MODE_OPTION_TNT
	) {
		return;
	}
	const tTnt *pTnt = &g_pVehicles[ubPlayer].sDynamite;
	tBob *pPlayerBombMarkers = s_pPlayerBombMarkers[ubPlayer];
	for(UBYTE i = 0; i < pTnt->ubCoordCount; ++i) {
		pPlayerBombMarkers[i].sPos.uwX = (pTnt->pCoords[i].uwX << TILE_SHIFT) + 8;
		pPlayerBombMarkers[i].sPos.uwY = (pTnt->pCoords[i].uwY << TILE_SHIFT) + 11;
		gameTryPushBob(&pPlayerBombMarkers[i]);
	}
}

static void gameProcessModeTeleport(UBYTE ubPlayer) {
	s_sTeleportReturn = (tUwCoordYX) {
		.uwX = fix16_to_int(g_pVehicles[ubPlayer].fX),
		.uwY = fix16_to_int(g_pVehicles[ubPlayer].fY)
	};
	vehicleTeleport(&g_pVehicles[ubPlayer], 160, s_pBaseTeleportY[0]);
	s_pModeMenus[ubPlayer].ubCurrent = 0;
}

static UBYTE gameProcessModeDrill(UBYTE ubPlayer) {
	tModeMenu *pModeMenu = &s_pModeMenus[ubPlayer];

	if(!g_isChallenge) {
		if(vehicleIsNearShop(&g_pVehicles[ubPlayer])) {
			if(gameChangeModePreset(ubPlayer, MODE_PRESET_SHOP)) {
				modeMenuEnterSelection(&s_pModeMenus[ubPlayer]);
			}
			if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_FIRE) || inboxGetState() == INBOX_STATE_URGENT) {
				statePush(g_pGameStateManager, &g_sStateShop);
				return 1;
			}
		}
		else {
			if(gameChangeModePreset(ubPlayer, MODE_PRESET_DEFAULT)) {
				modeMenuExitSelection(&s_pModeMenus[ubPlayer]);
			}
			if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_FIRE)) {
				if(g_pVehicles[ubPlayer].isMarkerShown) {
					s_eCameraType = (ubPlayer == 0) ? CAMERA_TYPE_P1 : CAMERA_TYPE_P2;
				}
				else if(!pModeMenu->isActive) {
					modeMenuEnterSelection(pModeMenu);
				}
				else {
					tModeOption eSelectedmode = modeMenuExitSelection(pModeMenu);
					if(eSelectedmode == MODE_OPTION_TNT) {
						tUwCoordYX sTilePos = {
							.uwX = (g_pVehicles[ubPlayer].sBobBody.sPos.uwX + VEHICLE_WIDTH / 2) >> 5,
							.uwY = (g_pVehicles[ubPlayer].sBobBody.sPos.uwY + VEHICLE_WIDTH / 2) >> 5
						};
						tntReset(&g_pVehicles[ubPlayer].sDynamite, ubPlayer, sTilePos);
					}
				}
			}
		}
	}

	if(pModeMenu->isActive && s_pPlayerModePreset[ubPlayer] != MODE_PRESET_SHOP) {
		tDirection eDirection = DIRECTION_COUNT;
		if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_LEFT)) {
			eDirection = DIRECTION_LEFT;
		}
		else if(steerDirUse(&s_pPlayerSteers[ubPlayer], DIRECTION_RIGHT)) {
			eDirection = DIRECTION_RIGHT;
		}
		modeMenuProcess(pModeMenu, eDirection);
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
		if(s_pModeMenus[ubPlayer].isActive) {
			isReturnImmediately = gameProcessModeDrill(ubPlayer);
		}
		else {
			tModeOption eMode = modeMenuGetSelected(&s_pModeMenus[ubPlayer]);
			switch(eMode) {
				case MODE_OPTION_DRILL:
					isReturnImmediately = gameProcessModeDrill(ubPlayer);
					break;
				case MODE_OPTION_TNT:
					gameProcessModeTnt(ubPlayer);
					break;
				case MODE_OPTION_TELEPORT:
					gameProcessModeTeleport(ubPlayer);
					break;
				default:
					break;
			}
		}
	}
	return isReturnImmediately;
}

//------------------------------------------------------------------ PRIVATE FNS

static void gameProcessHotkeys(void) {
  if(keyUse(KEY_ESCAPE) || keyUse(KEY_P)) {
		stateChange(g_pGameStateManager, &g_sStatePause);
		return;
  }
	if(keyUse(KEY_B)) {
		debugToggle();
	}
	if(keyCheck(KEY_SLASH)) {
		vPortWaitForEnd(s_pVpMain);
		vPortWaitForEnd(s_pVpMain);
		vPortWaitForEnd(s_pVpMain);
	}
	if(keyUse(KEY_R) && s_sTeleportReturn.ulYX != -1u && g_pVehicles[0].ubVehicleState == VEHICLE_STATE_MOVING) {
		vehicleTeleport(&g_pVehicles[0], s_sTeleportReturn.uwX, s_sTeleportReturn.uwY);
	}
	if(keyUse(KEY_N)) {
		vehicleTeleport(&g_pVehicles[0], 4 * TILE_SIZE, 212 * TILE_SIZE);
	}
	if(keyUse(KEY_M)) {
		vehicleTeleport(&g_pVehicles[0], 4 * TILE_SIZE, 4 * TILE_SIZE);
	}
	if(keyUse(KEY_COMMA)) {
		vehicleTeleport(&g_pVehicles[0], 4 * TILE_SIZE, 104 * TILE_SIZE);
	}

	if(keyUse(KEY_F1) && !g_isChallenge) {
		if(!g_is2pPlaying) {
			g_is2pPlaying = 1;
			hudSet2pPlaying(1);
			modeMenuReset(&s_pModeMenus[1], 1);
			gameResetModePreset(1);
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
	else if(keyUse(KEY_5)) {
		blitRect(g_pMainBuffer->pScroll->pBack, 32, 64, 100, 5, 17);
		blitRect(g_pMainBuffer->pScroll->pBack, 32, g_pMainBuffer->pScroll->pBack->Rows - 64, 100, 5, 25);
	}
	else if(keyUse(KEY_6)) {
		// twisterEnable();
		s_eGateCutsceneStep = GATE_CUTSCENE_STEP_START;
	}
	else if(keyUse(KEY_7)) {
		hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_GUEST]);
		inboxPushBack(COMM_SHOP_PAGE_OFFICE_ARCH_PLAN_FAIL, 0);
	}
	else if(keyUse(KEY_8)) {
		g_pVehicles[0].lCash += 1000;
	}
	else if(keyUse(KEY_9)) {
		questGateAddFragment();
	}
	else if(keyUse(KEY_MINUS)) {
		gameElapseDay();
	}
	else if(keyUse(KEY_EQUALS)) {
		hudShowMessage(0, g_pMsgs[MSG_HUD_NEW_PLAN]);
		warehouseNextPlan(NEXT_PLAN_REASON_FULFILLED);
	}
	else if(keyUse(KEY_0)) {
		gameElapseTime(planManagerGet()->wTimeRemaining);
	}
}

static UWORD gameGetCameraDestinationY(void) {
	UWORD uwCamDestY;
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
	return uwCamDestY;
}

static UBYTE s_ubGateCutsceneCooldown;

static void gameProcessGateCutscene(void) {
	if(s_eGateCutsceneStep == GATE_CUTSCENE_STEP_OFF) {
		return;
	}

	switch(s_eGateCutsceneStep) {
		case GATE_CUTSCENE_STEP_START:
			++s_eGateCutsceneStep;
			s_ubGateCutsceneCooldown = 0;
			// TODO: disable movement
			// TODO: disable context menu
			// TODO: disable entering pause
			ptplayerEnableMusic(0);
			break;
		case GATE_CUTSCENE_STEP_WAIT_FOR_SHAKE:
			if(++s_ubGateCutsceneCooldown > 30) {
				s_ubGateCutsceneCooldown = 0;
				++s_eGateCutsceneStep;
				s_isCameraShake = 1;
			}
			break;
		case GATE_CUTSCENE_STEP_SHAKE_BEFORE_TWISTER:
			if(++s_ubGateCutsceneCooldown > 80) {
				s_ubGateCutsceneCooldown = 0;
				++s_eGateCutsceneStep;
				twisterEnable();
			}
			break;
		case GATE_CUTSCENE_STEP_TWIST_BEFORE_FADE:
			if(++s_ubGateCutsceneCooldown > 35) {
				s_ubGateCutsceneCooldown = 0;
				fadeMorphTo(FADE_STATE_OUT);
				++s_eGateCutsceneStep;
			}
			break;
		case GATE_CUTSCENE_STEP_FADE_OUT:
			if(fadeGetState() == FADE_STATE_OUT) {
				vehicleSetPos(&g_pVehicles[0], 160, s_pBaseTeleportY[0]);
				vehicleSetPos(&g_pVehicles[1], 160, s_pBaseTeleportY[0]);
				s_isCameraShake = 0;
				twisterDisable();
				++s_eGateCutsceneStep;
			}
			break;
		case GATE_CUTSCENE_STEP_FADE_IN:
			if(fadeGetState() == FADE_STATE_IN) {
				++s_eGateCutsceneStep;
				// TODO: launch the outro commrade
			}
			break;
		case GATE_CUTSCENE_STEP_END:
		case GATE_CUTSCENE_STEP_OFF:
			s_eGateCutsceneStep = GATE_CUTSCENE_STEP_OFF;
			break;
	}
}

static void gameCameraProcess(void) {
	if(g_isChallenge) {
		const UWORD uwBottomPos = g_pMainBuffer->pCamera->uPos.uwY + g_pMainBuffer->sCommon.pVPort->uwHeight - 2 * TILE_SIZE;
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
		UWORD uwCamDestX = 32;
		UWORD uwCamDestY = gameGetCameraDestinationY();
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
		g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;

		if(fadeGetState() == FADE_STATE_OUT) {
			cameraCenterAt(g_pMainBuffer->pCamera, uwCamDestX, uwCamDestY);
			g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
			baseProcess();
			groundLayerReset(groundLayerGetLowerAtDepth(g_pMainBuffer->pCamera->uPos.uwY));
			tileBufferRedrawAll(g_pMainBuffer);
			bobDiscardUndraw();
			g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
			fadeMorphTo(FADE_STATE_IN);
		}

		if(s_isCameraShake) {
			BYTE bShake = randUwMinMax(
				&g_sRand, 0, 2 * CAMERA_SHAKE_AMPLITUDE
			) - CAMERA_SHAKE_AMPLITUDE;
			cameraMoveBy(g_pMainBuffer->pCamera, 0, bShake);
		}
	}
}

static void onSongEnd(void) {
	if(++s_ubCurrentMod >= ASSETS_GAME_MOD_COUNT) {
		s_ubCurrentMod = 0;
	}
	ptplayerLoadMod(g_pGameMods[s_ubCurrentMod], g_pModSampleData, 0);
	ptplayerEnableMusic(1);
}

static void processPlan(void) {
	if(!planManagerGet()->isPlanActive) {
		return;
	}

	WORD wRemainingDays = planGetRemainingDays();
	if(wRemainingDays <= 0) {
		if(!planManagerGet()->isExtendedTimeByFavor && planTryProlong()) {
			char szBfr[100];
			sprintf(szBfr, g_pMsgs[MSG_HUD_PLAN_EXTENDING], 14);
			hudShowMessage(0, szBfr);
		}
		else {
			hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_WAITING_KOMISARZ]);
			gameAddRebuke();
			planFailDeadline();
		}
	}
	else if(
		wRemainingDays == 10 || wRemainingDays == 5 || wRemainingDays == 3 ||
		wRemainingDays == 2 || wRemainingDays == 1
	) {
		if(wRemainingDays != s_wLastReminder) {
			s_wLastReminder = wRemainingDays;
			char szBuffer[50];
			sprintf(szBuffer, g_pMsgs[MSG_HUD_PLAN_REMAINING], wRemainingDays);
			hudShowMessage(0, szBuffer);
		}
	}
	else {
		s_wLastReminder = 0;
	}
}

static void gameChallengeResult(void) {
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

static void gameSaveSummary(tFile *pFile) {
	saveWriteHeader(pFile, "SMRY");
	tGameSummary sSummary = {
		.lCash = g_pVehicles[0].lCash,
		.ubAccolades = s_ubAccolades,
		.ubHeatPercent = heatGetPercent(),
		.ubPlanIndex = planManagerGet()->ubCurrentPlanIndex,
		.ubRebukes = s_ubRebukes,
		.ulGameTime = s_ulGameTime,
		.uwMaxDepth = s_uwMaxTileY,
	};

	fileWrite(pFile, &sSummary.ulGameTime, sizeof(sSummary.ulGameTime));
	fileWrite(pFile, &sSummary.lCash, sizeof(sSummary.lCash));
	fileWrite(pFile, &sSummary.uwMaxDepth, sizeof(sSummary.uwMaxDepth));
	fileWrite(pFile, &sSummary.ubRebukes, sizeof(sSummary.ubRebukes));
	fileWrite(pFile, &sSummary.ubAccolades, sizeof(sSummary.ubAccolades));
	fileWrite(pFile, &sSummary.ubHeatPercent, sizeof(sSummary.ubHeatPercent));
	fileWrite(pFile, &sSummary.ubPlanIndex, sizeof(sSummary.ubPlanIndex));
}

static void gameSave(tFile *pFile) {
	gameSaveSummary(pFile);
	saveWriteHeader(pFile, "GAME");
	fileWrite(pFile, &g_is2pPlaying, sizeof(g_is2pPlaying));
	fileWrite(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileWrite(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileWrite(pFile, &g_isChallenge, sizeof(g_isChallenge));
	fileWrite(pFile, &g_isAtari, sizeof(g_isAtari));

	fileWrite(pFile, &s_sTeleportReturn.ulYX, sizeof(s_sTeleportReturn.ulYX));
	fileWrite(pFile, &s_eCameraType, sizeof(s_eCameraType));
	fileWrite(pFile, &s_ubChallengeCamCnt, sizeof(s_ubChallengeCamCnt));

	fileWrite(pFile, &s_ubRebukes, sizeof(s_ubRebukes));
	fileWrite(pFile, &s_ubAccolades, sizeof(s_ubAccolades));
	fileWrite(pFile, &s_ubAccoladesFract, sizeof(s_ubAccoladesFract));
	fileWrite(pFile, &s_wLastReminder, sizeof(s_wLastReminder));
	fileWrite(pFile, &s_ubCurrentMod, sizeof(s_ubCurrentMod));
	fileWrite(pFile, &s_ulGameTime, sizeof(s_ulGameTime));
	fileWrite(pFile, &s_uwMaxTileY, sizeof(s_uwMaxTileY));

	inboxSave(pFile);
	dinoSave(pFile);
	questGateSave(pFile);
	tutorialSave(pFile);
	pageOfficeSave(pFile);
	warehouseSave(pFile);
	tileSave(pFile);
	inventorySave(pFile);
	vehicleSave(&g_pVehicles[0], pFile);
	vehicleSave(&g_pVehicles[1], pFile);
	hudSave(pFile);
	heatSave(pFile);
}

//------------------------------------------------------------------- PUBLIC FNS

void gameCancelModeForPlayer(UBYTE ubPlayer) {
	if(!s_pModeMenus[ubPlayer].isActive) {
		s_pModeMenus[ubPlayer].ubCurrent = 0;
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
		if(!dinoIsAllFound()) {
			inboxPushBack(COMM_SHOP_PAGE_OFFICE_ARCH_PLAN_FAIL, 0);
		}
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

void gameInitBombMarkerBobs(void) {
	UBYTE *pAddressFrame = bobCalcFrameAddress(g_pBombMarker, 0);
	UBYTE *pAddressMask = bobCalcFrameAddress(g_pBombMarkerMask, 0);
	for(UBYTE ubPlayer = 0; ubPlayer < 2; ++ubPlayer) {
		for(UBYTE i = 0; i < 3; ++i) {
			bobInit(
				&s_pPlayerBombMarkers[ubPlayer][i], 16, 10, 1,
				pAddressFrame, pAddressMask, 0, 0
			);
		}
	}
}

UBYTE gameCanPushBob(const tBob *pBob) {
	UBYTE isOnCamera = (
		pBob->sPos.uwY + pBob->uwHeight >= g_pMainBuffer->pCamera->uPos.uwY &&
		pBob->sPos.uwY < g_pMainBuffer->pCamera->uPos.uwY + s_pVpMain->uwHeight
	);
	return isOnCamera;
}

UBYTE gameTryPushBob(tBob *pBob) {
	if(gameCanPushBob(pBob)) {
		bobPush(pBob);
		return 1;
	}
	else {
		return 0;
	}
}

void gameElapseTime(UWORD uwTime) {
	if(ULONG_MAX - s_ulGameTime > uwTime) {
		s_ulGameTime += uwTime;
	}
	else {
		s_ulGameTime = ULONG_MAX;
	}

	planElapseTime(uwTime);
	if(!planManagerGet()->isPlanActive && planManagerGet()->wTimeRemaining == 0 && planManagerGet()->ubCurrentPlanIndex > 0) {
		// first plan start (index 0) is handled by tutorial
		hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_HUD_NEW_PLAN]);
		planStart();
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

UBYTE gameLoad(tFile *pFile) {
	{
		tGameSummary sSummary; ///< Only for skipping it
		if(!gameLoadSummary(pFile, &sSummary)) {
			return 0;
		}
	}

	if(!saveReadHeader(pFile, "GAME")) {
		return 0;
	}

	fileRead(pFile, &g_is2pPlaying, sizeof(g_is2pPlaying));
	fileRead(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileRead(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileRead(pFile, &g_isChallenge, sizeof(g_isChallenge));
	fileRead(pFile, &g_isAtari, sizeof(g_isAtari));

	fileRead(pFile, &s_sTeleportReturn.ulYX, sizeof(s_sTeleportReturn.ulYX));
	fileRead(pFile, &s_eCameraType, sizeof(s_eCameraType));
	fileRead(pFile, &s_ubChallengeCamCnt, sizeof(s_ubChallengeCamCnt));

	fileRead(pFile, &s_ubRebukes, sizeof(s_ubRebukes));
	fileRead(pFile, &s_ubAccolades, sizeof(s_ubAccolades));
	fileRead(pFile, &s_ubAccoladesFract, sizeof(s_ubAccoladesFract));
	fileRead(pFile, &s_wLastReminder, sizeof(s_wLastReminder));
	fileRead(pFile, &s_ubCurrentMod, sizeof(s_ubCurrentMod));
	fileRead(pFile, &s_ulGameTime, sizeof(s_ulGameTime));
	fileRead(pFile, &s_uwMaxTileY, sizeof(s_uwMaxTileY));

	return inboxLoad(pFile) &&
		dinoLoad(pFile) &&
		questGateLoad(pFile) &&
		tutorialLoad(pFile) &&
		pageOfficeLoad(pFile) &&
		warehouseLoad(pFile) &&
		tileLoad(pFile) &&
		inventoryLoad(pFile) &&
		vehicleLoad(&g_pVehicles[0], pFile) &&
		vehicleLoad(&g_pVehicles[1], pFile) &&
		hudLoad(pFile) &&
		heatLoad(pFile);
}

UBYTE gameLoadSummary(tFile *pFile, tGameSummary *pSummary) {
	if(!saveReadHeader(pFile, "SMRY")) {
		return 0;
	}

	fileRead(pFile, &pSummary->ulGameTime, sizeof(pSummary->ulGameTime));
	fileRead(pFile, &pSummary->lCash, sizeof(pSummary->lCash));
	fileRead(pFile, &pSummary->uwMaxDepth, sizeof(pSummary->uwMaxDepth));
	fileRead(pFile, &pSummary->ubRebukes, sizeof(pSummary->ubRebukes));
	fileRead(pFile, &pSummary->ubAccolades, sizeof(pSummary->ubAccolades));
	fileRead(pFile, &pSummary->ubHeatPercent, sizeof(pSummary->ubHeatPercent));
	fileRead(pFile, &pSummary->ubPlanIndex, sizeof(pSummary->ubPlanIndex));
	return 1;
}

void gameStart(UBYTE isChallenge, tSteer sSteerP1, tSteer sSteerP2) {
	s_ubChallengeCamCnt = 0;
	g_isChallenge = isChallenge;
	s_pPlayerSteers[0] = sSteerP1;
	s_pPlayerSteers[1] = sSteerP2;
	inboxReset();
	dinoReset();
	questGateReset();
	collectiblesReset();
	tutorialReset();
	pageOfficeReset();
	warehouseReset();
	tileReset(g_isAtari, g_isChallenge);
	inventoryReset();
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	modeMenuReset(&s_pModeMenus[0], 0);
	modeMenuReset(&s_pModeMenus[1], 1);
	gameResetModePreset(0);
	gameResetModePreset(1);
	s_ulGameTime = 0;
	s_uwMaxTileY = 0;
	s_ubCurrentMod = ASSETS_GAME_MOD_COUNT;
	s_ubRebukes = 0;
	s_ubAccolades = 0;
	s_ubAccoladesFract = 0;
	s_wLastReminder = 0;
	s_sTeleportReturn.ulYX = -1;
	s_eGateCutsceneStep = GATE_CUTSCENE_STEP_OFF;
	twisterDisable();
	hudReset(g_isChallenge, g_is2pPlaying);
	heatReset();
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

void gameUpdateMaxDepth(UWORD uwTileY) {
	s_uwMaxTileY = MAX(s_uwMaxTileY, uwTileY);
}

//-------------------------------------------------------------------- GAMESTATE

static void gameGsCreate(void) {
	ptplayerConfigureSongRepeat(0, onSongEnd);
	onSongEnd();
}

static void gameGsLoop(void) {
	if(tutorialProcess()) {
		return;
	}
	dinoProcess();
	questGateProcess();
	gameProcessGateCutscene();

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
	twisterProcess();
	debugColor(0x088);
	vehicleProcess(&g_pVehicles[0]);
	if(g_is2pPlaying) {
		debugColor(0x880);
		vehicleProcess(&g_pVehicles[1]);
	}
	debugColor(0x808);
	explosionManagerProcess();
	modeMenuTryDisplay(&s_pModeMenus[0]);
	modeMenuTryDisplay(&s_pModeMenus[1]);
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
