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
#include "bob_new.h"
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

typedef enum _tCameraType {
	CAMERA_TYPE_BETWEEN,
	CAMERA_TYPE_P1,
	CAMERA_TYPE_P2,
	CAMERA_TYPE_COUNT
} tCameraType;

tSample *g_pSampleDrill, *g_pSampleOre, *g_pSampleTeleport;

static tCameraType s_eCameraType = CAMERA_TYPE_BETWEEN;
static tView *s_pView;
static tVPort *s_pVpMain;
tTileBufferManager *g_pMainBuffer;

static tBitMap *s_pTiles;
static tBitMap *s_pBones, *s_pBonesMask;
static UBYTE s_isDebug = 0;
static UWORD s_uwColorBg;
static UBYTE s_ubChallengeCamCnt;
static tTextBob s_sChallengeResult;

static tBobNew s_pDinoBobs[9];
static UBYTE s_pDinoWereDrawn[9];
UBYTE g_ubDinoBonesFound = 0;

tFont *g_pFont;
UBYTE g_is2pPlaying;
UBYTE g_is1pKbd, g_is2pKbd;
UBYTE g_isChallenge, g_isAtari;

UBYTE s_isMsgShown = 0;

static void goToMenu(void) {
	// Switch to menu, after popping it will process gameGsLoop
	gamePushState(menuGsCreate, menuGsLoop, menuGsDestroy);
}

void gameStart(void) {
	s_ubChallengeCamCnt = 0;
	g_ubDinoBonesFound = 0;
	for(UBYTE i = 0; i < 9; ++i) {
		s_pDinoWereDrawn[i] = 0;
	}
	s_isMsgShown = 0;
	tileInit(g_isAtari, g_isChallenge);
	warehouseReset(g_is2pPlaying);
	vehicleReset(&g_pVehicles[0]);
	vehicleReset(&g_pVehicles[1]);
	hudReset(g_isChallenge, g_is2pPlaying);
	groundLayerReset(1);
}

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

	paletteLoad("data/aminer.plt", s_pVpMain->pPalette, 1 << GAME_BPP);
	s_uwColorBg = s_pVpMain->pPalette[0];

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

	tileInit(0, 0);

	bobNewManagerCreate(
		g_pMainBuffer->pScroll->pFront, g_pMainBuffer->pScroll->pBack,
		g_pMainBuffer->pScroll->uwBmAvailHeight
	);
	groundLayerCreate(s_pVpMain);
	commCreate();
	commShopAlloc();
	vehicleBitmapsCreate();
	vehicleCreate(&g_pVehicles[0], PLAYER_1);
	vehicleCreate(&g_pVehicles[1], PLAYER_2);
	textBobCreate(&s_sChallengeResult, g_pFont, "Player 20 wins!");

	hiScoreBobsCreate();
	menuPreload();
	bobNewAllocateBgBuffers();
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
	tileBufferInitialDraw(g_pMainBuffer);

	// Load the view
	viewLoad(s_pView);
	goToMenu();
}

static void gameProcessInput(void) {
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
		if(++s_eCameraType == CAMERA_TYPE_COUNT) {
			s_eCameraType = CAMERA_TYPE_BETWEEN;
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

void gameGsLoop(void) {
	static UBYTE ubLastDino = 0;

	if(!g_isChallenge && !s_isMsgShown) {
		gamePushState(commMsgGsCreate, commMsgGsLoop, commMsgGsDestroy);
		s_isMsgShown = 1;
		return;
	}

  if(keyUse(KEY_ESCAPE)) {
    goToMenu();
		return;
  }
	if(keyUse(KEY_B)) {
		s_isDebug = !s_isDebug;
	}
	if(
		(keyUse(KEY_RETURN) || keyUse(KEY_SPACE)) &&
		vehicleIsNearShop(&g_pVehicles[0])
	) {
		gamePushState(commShopGsCreate, commShopGsLoop, commShopGsDestroy);
		return;
	}

	debugColor(0x008);
	bobNewBegin();
	tileBufferQueueProcess(g_pMainBuffer);
	gameProcessInput();
	vehicleProcessText();
	debugColor(0x080);

	// Process plan being complete
	if(warehouseGetPlan()->wTimeRemaining <= 0 || keyUse(KEY_U)) {
		if(warehouseIsPlanFulfilled()) {
			hudShowMessage(0, "Comrade, plan done, new has arrived");
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
	debugColor(0x088);
	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	if(g_isChallenge) {
		++s_ubChallengeCamCnt;
		if(s_ubChallengeCamCnt >= 2) {
			g_pMainBuffer->pCamera->uPos.uwY += 1;
			s_ubChallengeCamCnt = 0;
		}
	}
	else {
		UWORD uwCamY, uwCamX = 32;
		if(!g_is2pPlaying) {
			// One player only
			// uwCamX = fix16_to_int(g_pVehicles[0].fX) + VEHICLE_WIDTH / 2;
			uwCamY = fix16_to_int(g_pVehicles[0].fY) + VEHICLE_HEIGHT / 2;
		}
		else {
			// Two players
			if(s_eCameraType == CAMERA_TYPE_P1) {
				uwCamY = fix16_to_int(g_pVehicles[0].fY) + VEHICLE_HEIGHT / 2;
			}
			else if(s_eCameraType == CAMERA_TYPE_P2) {
				uwCamY = fix16_to_int(g_pVehicles[1].fY) + VEHICLE_HEIGHT / 2;
			}
			else {
				uwCamY = (
					fix16_to_int(g_pVehicles[0].fY) +
					fix16_to_int(g_pVehicles[1].fY) + VEHICLE_HEIGHT
				) / 2;
			}
		}
		WORD wDist = (
			uwCamY - g_pMainBuffer->pCamera->sCommon.pVPort->uwHeight / 2
		) - g_pMainBuffer->pCamera->uPos.uwY;
		if(ABS(wDist) > 4) {
			cameraMoveBy(g_pMainBuffer->pCamera, 0, SGN(wDist) * 4);
		}
		else {
			cameraMoveBy(g_pMainBuffer->pCamera, 0, wDist);
		}
		if(g_pMainBuffer->pCamera->uPos.uwX < uwCamX) {
			g_pMainBuffer->pCamera->uPos.uwX = uwCamX;
		}
	}
	baseTileProcess();

	groundLayerProcess(g_pMainBuffer->pCamera->uPos.uwY);

	debugColor(0x800);
	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugColor(s_uwColorBg);
	vPortWaitForEnd(s_pVpMain);
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
	hiScoreBobsDestroy();
	textBobDestroy(&s_sChallengeResult);
	vehicleDestroy(&g_pVehicles[0]);
	vehicleDestroy(&g_pVehicles[1]);
	vehicleBitmapsDestroy();
	commDestroy();
	commShopDealloc();
	bobNewManagerDestroy();

	audioDestroy();
	sampleDestroy(g_pSampleDrill);
	sampleDestroy(g_pSampleOre);
	sampleDestroy(g_pSampleTeleport);

  hudDestroy();
  viewDestroy(s_pView);
}

void gameGsLoopChallengeEnd(void) {
	if(
		keyUse(KEY_ESCAPE) ||
		(!hiScoreIsEntering() && (keyUse(KEY_RETURN) || joyUse(JOY1_FIRE) || joyUse(JOY2_FIRE)))
	) {
		gameChangeLoop(gameGsLoop);
		goToMenu();
		return;
	}
	bobNewBegin();
	tileBufferQueueProcess(g_pMainBuffer);

	if(hiScoreIsEntering()) {
		hiScoreEnteringProcess();
	}

	vehicleProcess(&g_pVehicles[0]);
	if(g_is2pPlaying) {
		vehicleProcess(&g_pVehicles[1]);
	}
	bobNewPush(&s_sChallengeResult.sBob);
	hiScoreBobsDisplay();

	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugColor(s_uwColorBg);
	vPortWaitForEnd(s_pVpMain);
}

void gameGsLoopScorePreview(void) {
	if(
		keyUse(KEY_ESCAPE) || keyUse(KEY_RETURN) ||
		joyUse(JOY1_FIRE) || joyUse(JOY2_FIRE)
	) {
		gameChangeLoop(gameGsLoop);
		goToMenu();
		return;
	}

	bobNewBegin();
	hiScoreBobsDisplay();
	bobNewPushingDone();
	bobNewEnd();

	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugColor(s_uwColorBg);
	vPortWaitForEnd(s_pVpMain);
}

void gameChallengeEnd(void) {
	g_pVehicles[0].sSteer.bX = 0;
	g_pVehicles[0].sSteer.bY = 0;
	g_pVehicles[1].sSteer.bX = 0;
	g_pVehicles[1].sSteer.bY = 0;


	// Result text
	if(g_is2pPlaying) {
		if(g_pVehicles[0].lCash > g_pVehicles[1].lCash) {
			textBobSetText(&s_sChallengeResult, "Player 1 wins!");
		}
		else if(g_pVehicles[0].lCash < g_pVehicles[1].lCash) {
			textBobSetText(&s_sChallengeResult, "Player 2 wins!");
		}
		else {
			textBobSetText(&s_sChallengeResult, "Draw!");
		}
	}
	else {
		textBobSetText(&s_sChallengeResult, "Score: %ld", g_pVehicles[0].lCash);
	}
	UWORD uwCenterX = 160 + g_pMainBuffer->pCamera->uPos.uwX;
	textBobSetPos(
		&s_sChallengeResult, uwCenterX, g_pMainBuffer->pCamera->uPos.uwY + 50, 0, 1
	);
	textBobSetColor(&s_sChallengeResult, 14);
	textBobUpdate(&s_sChallengeResult);

	if(!g_is2pPlaying) {
		hiScoreSetup(g_pVehicles[0].lCash);
	}
	else {
		// No hi score for 2 players
		hiScoreSetup(0);
	}

	gameChangeLoop(gameGsLoopChallengeEnd);
}
