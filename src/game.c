/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/custom.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
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

typedef enum _tCameraType {
	CAMERA_TYPE_P1,
	CAMERA_TYPE_P2,
} tCameraType;

#define CAMERA_SPEED 4

typedef enum _tCameraFade {
	CAMERA_FADE_NONE,
	CAMERA_FADE_OUT,
	CAMERA_FADE_IN
} tCameraFade;

static tCameraFade s_eCameraFade = CAMERA_FADE_NONE;
static UBYTE s_ubCameraFadeLevel, s_ubCameraFadeLevelPrev;

tSample *g_pSampleDrill, *g_pSampleOre, *g_pSampleTeleport;

static tCameraType s_eCameraType = CAMERA_TYPE_P1;
static tView *s_pView;
static tVPort *s_pVpMain;
tTileBufferManager *g_pMainBuffer;

static tBitMap *s_pTiles;
static tBitMap *s_pBones, *s_pBonesMask;
static UBYTE s_isDebug = 0;
static UWORD s_pPaletteRef[1 << GAME_BPP];
static UWORD *s_pColorBg;
static UBYTE s_ubChallengeCamCnt;

static tBobNew s_pDinoBobs[9];
static UBYTE s_pDinoWereDrawn[9];
UBYTE g_ubDinoBonesFound = 0;

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
	g_ubDinoBonesFound = 0;
	for(UBYTE i = 0; i < 9; ++i) {
		s_pDinoWereDrawn[i] = 0;
	}
	tutorialReset();
	tileInit(g_isAtari, g_isChallenge);
	warehouseReset(g_is2pPlaying);
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	hudReset(g_isChallenge, g_is2pPlaying);
	groundLayerReset(1);
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

static inline void debugColor(UWORD uwColor) {
	if(s_isDebug) {
		g_pCustom->color[0] = uwColor;
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
		if(uwAbsDistance > CAMERA_SPEED * 50 && s_eCameraFade == CAMERA_FADE_NONE) {
			s_eCameraFade = CAMERA_FADE_OUT;
			s_ubCameraFadeLevel = 0xF;
		}
		if(
			s_eCameraFade == CAMERA_FADE_NONE || s_eCameraFade == CAMERA_FADE_OUT
		) {
			if(uwAbsDistance > CAMERA_SPEED) {
				cameraMoveBy(g_pMainBuffer->pCamera, 0, SGN(wCameraDistance) * CAMERA_SPEED);
			}
			else {
				cameraMoveBy(g_pMainBuffer->pCamera, 0, wCameraDistance);
			}
			if(g_pMainBuffer->pCamera->uPos.uwX < uwCamDestX) {
				g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
			}
		}
		if(s_eCameraFade == CAMERA_FADE_OUT) {
			if(s_ubCameraFadeLevel > 0) {
				--s_ubCameraFadeLevel;
			}
			else {
				cameraCenterAt(g_pMainBuffer->pCamera, uwCamDestX, uwCamDestY);
				g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
				baseTileProcess();
				tileBufferRedrawAll(g_pMainBuffer);
				bobNewDiscardUndraw();
				g_pMainBuffer->pCamera->uPos.uwX = uwCamDestX;
				s_eCameraFade = CAMERA_FADE_IN;
			}
		}
		else if(s_eCameraFade == CAMERA_FADE_IN) {
			if(s_ubCameraFadeLevel < 0xF) {
				++s_ubCameraFadeLevel;
			}
			else {
				s_eCameraFade = CAMERA_FADE_NONE;
			}
		}
	}
}

static void mainPaletteProcess(UBYTE ubFadeLevel) {
	if(s_ubCameraFadeLevelPrev != ubFadeLevel) {
		*s_pColorBg = paletteColorDim(s_pPaletteRef[0], ubFadeLevel);
		paletteDim(s_pPaletteRef, g_pCustom->color, 27, ubFadeLevel);
		s_ubCameraFadeLevelPrev = ubFadeLevel;
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
	hiScoreLoad();
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_END);

	g_pFont = fontCreate("data/uni54.fnt");
	textBobManagerCreate(g_pFont);
	s_pTiles = bitmapCreateFromFile("data/tiles.bm", 0);
	s_pBones = bitmapCreateFromFile("data/bones.bm", 0);
	s_pBonesMask = bitmapCreateFromFile("data/bones_mask.bm", 0);

	bobNewInit(&s_pDinoBobs[0], 80, 22, 0, s_pBones, s_pBonesMask, 32 + 92, 100 * 32 + 170);
	bobNewSetBitMapOffset(&s_pDinoBobs[0], 0);

	bobNewInit(&s_pDinoBobs[1], 80, 10, 0, s_pBones, s_pBonesMask, 32 + 116, 100 * 32 + 179);
	bobNewSetBitMapOffset(&s_pDinoBobs[1], 24);

	bobNewInit(&s_pDinoBobs[2], 80, 15, 0, s_pBones, s_pBonesMask, 32 + 147, 100 * 32 + 172);
	bobNewSetBitMapOffset(&s_pDinoBobs[2], 35);

	bobNewInit(&s_pDinoBobs[3], 80, 24, 0, s_pBones, s_pBonesMask, 32 + 159, 100 * 32 + 189);
	bobNewSetBitMapOffset(&s_pDinoBobs[3], 51);

	bobNewInit(&s_pDinoBobs[4], 80, 44, 0, s_pBones, s_pBonesMask, 32 + 178, 100 * 32 + 170);
	bobNewSetBitMapOffset(&s_pDinoBobs[4], 76);

	bobNewInit(&s_pDinoBobs[5], 80, 29, 0, s_pBones, s_pBonesMask, 32 + 215, 100 * 32 + 192);
	bobNewSetBitMapOffset(&s_pDinoBobs[5], 121);

	bobNewInit(&s_pDinoBobs[6], 80, 45, 0, s_pBones, s_pBonesMask, 32 + 209, 100 * 32 + 201);
	bobNewSetBitMapOffset(&s_pDinoBobs[6], 151);

	bobNewInit(&s_pDinoBobs[7], 80, 45, 0, s_pBones, s_pBonesMask, 32 + 220, 100 * 32 + 205);
	bobNewSetBitMapOffset(&s_pDinoBobs[7], 197);

	bobNewInit(&s_pDinoBobs[8], 80, 22, 0, s_pBones, s_pBonesMask, 32 + 250, 100 * 32 + 218);
	bobNewSetBitMapOffset(&s_pDinoBobs[8], 243);

	hudCreate(s_pView, g_pFont);

	s_pVpMain = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, GAME_BPP,
	TAG_END);
	g_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, 11,
		TAG_TILEBUFFER_BOUND_TILE_Y, 2047,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 5,
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
		TAG_TILEBUFFER_TILESET, s_pTiles,
	TAG_END);

	paletteLoad("data/aminer.plt", s_pPaletteRef, 1 << GAME_BPP);
	CopyMem(s_pPaletteRef, s_pVpMain->pPalette, 1 << GAME_BPP);
	s_pColorBg = &s_pVpMain->pPalette[0];

	baseTileCreate(g_pMainBuffer);
	audioCreate();
	g_pSampleDrill = sampleCreateFromFile("data/sfx/drill1.raw8", 8000);
	g_pSampleOre = sampleCreateFromFile("data/sfx/ore2.raw8", 8000);
	g_pSampleTeleport = sampleCreateFromFile("data/sfx/teleport.raw8", 8000);

#ifdef GAME_DEBUG
	randInit(2184);
#else
	// Seed from beam pos Y & X
	randInit((g_pRayPos->bfPosY << 8) | g_pRayPos->bfPosX);
#endif

	tileInit(0, 1);

	bobNewManagerCreate(
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	explosionManagerCreate();
	groundLayerCreate(s_pVpMain);
	commCreate();
	vehicleBitmapsCreate();
	vehicleCreate(&g_pVehicles[0], PLAYER_1);
	vehicleCreate(&g_pVehicles[1], PLAYER_2);

	menuPreload();
	bobNewAllocateBgBuffers();
	s_ubCameraFadeLevelPrev = 0xF;
	s_ubCameraFadeLevel = 0xF;
	systemUnuse();

	g_pMainBuffer->pCamera->uPos.uwX = 32;

	s_isDebug = 0;

	// Default config
	g_is2pPlaying = 0;
	g_is1pKbd = 0;
	g_is2pKbd = 1;
	g_isChallenge = 1;
	g_isAtari = 0;

	// Initial background
	tileBufferRedrawAll(g_pMainBuffer);

	// Load the view
	viewLoad(s_pView);
	menuGsEnter(0);
}

void gameGsLoop(void) {
	static UBYTE ubLastDino = 0;

	if(!g_isChallenge) {
		if(tutorialProcess()) {
			return;
		}
	}

  if(keyUse(KEY_ESCAPE)) {
    menuGsEnter(0);
		return;
  }
	if(keyUse(KEY_B)) {
		s_isDebug = !s_isDebug;
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

	debugColor(0x008);
	bobNewBegin();
	tileBufferQueueProcess(g_pMainBuffer);
	gameCameraProcess();
	gameProcessInput();
	vehicleProcessText();
	debugColor(0x080);

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

	if(g_ubDinoBonesFound && tileBufferIsTileOnBuffer(
		g_pMainBuffer,
		s_pDinoBobs[ubLastDino].sPos.uwX / 32,
		s_pDinoBobs[ubLastDino].sPos.uwY / 32
	) && s_pDinoWereDrawn[ubLastDino] < 2) {
		bobNewPush(&s_pDinoBobs[ubLastDino]);
		++s_pDinoWereDrawn[ubLastDino];
		if(s_pDinoWereDrawn[ubLastDino] >= 2) {
			++ubLastDino;
			if(ubLastDino >= g_ubDinoBonesFound) {
				ubLastDino = 0;
			}
		}
	}
	else {
		s_pDinoWereDrawn[ubLastDino] = 0;
		++ubLastDino;
		if(ubLastDino >= g_ubDinoBonesFound) {
			ubLastDino = 0;
		}
	}
	vehicleProcess(&g_pVehicles[0]);
	if(g_is2pPlaying) {
		debugColor(0x880);
		vehicleProcess(&g_pVehicles[1]);
	}
	debugColor(0x808);
	explosionManagerProcess();
	debugColor(0x088);
	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	baseTileProcess();

	groundLayerProcess(g_pMainBuffer->pCamera->uPos.uwY, s_ubCameraFadeLevel);
	mainPaletteProcess(s_ubCameraFadeLevel);

	debugColor(0x800);
	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugColor(*s_pColorBg);
	vPortWaitForEnd(s_pVpMain);

	if(g_isChallengeEnd && g_pVehicles[0].ubDrillDir == DRILL_DIR_NONE && (
		!g_is2pPlaying || g_pVehicles[1].ubDrillDir == DRILL_DIR_NONE
	)) {
		gameChallengeResult();
	}
}

void gameGsDestroy(void) {
  // Cleanup when leaving this gamestate
  systemUse();

	menuUnload();
	bitmapDestroy(s_pTiles);
	bitmapDestroy(s_pBones);
	bitmapDestroy(s_pBonesMask);
	baseTileDestroy();
	textBobManagerDestroy();
	fontDestroy(g_pFont);
	vehicleDestroy(&g_pVehicles[0]);
	vehicleDestroy(&g_pVehicles[1]);
	vehicleBitmapsDestroy();
	commDestroy();
	bobNewManagerDestroy();

	audioDestroy();
	sampleDestroy(g_pSampleDrill);
	sampleDestroy(g_pSampleOre);
	sampleDestroy(g_pSampleTeleport);

	explosionManagerDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}
