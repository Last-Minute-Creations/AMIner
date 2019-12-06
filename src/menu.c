/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "menu.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include "game.h"
#include "bob_new.h"
#include "text_bob.h"
#include "ground_layer.h"
#include "build_ver.h"
#include "base_tile.h"
#include "hi_score.h"
#include "comm.h"

typedef enum _tMenuState {
	MENU_STATE_ROLL_IN = 0,
	MENU_STATE_SELECTING,
	MENU_STATE_ROLL_OUT
} tMenuState;

typedef enum _tOptionType {
	OPTION_TYPE_UINT8,
	OPTION_TYPE_CALLBACK
} tOptionType;

typedef void (*tOptionSelectCb)(void);

// All options are uint8_t, enums or numbers
typedef struct _tOption {
	char *szName;
	tOptionType eOptionType;
	UBYTE isHidden;
	UBYTE isDirty;
	union {
		struct {
			UBYTE *pVar;
			UBYTE ubMax;
			UBYTE ubDefault;
			UBYTE isCyclic;
			const char **pEnumLabels;
		} sOptUb;
		struct {
			tOptionSelectCb cbSelect;
		} sOptCb;
	};
} tOption;

void onStart(void);
void onExit(void);
void onShowScores(void);

static const char *s_pModeEnum[] = {"Campaign", "Challenge"};
static const char *s_pPlayersEnum[] = {"1", "2"};
static const char *s_pP1Enum[] = {"Joy", "WSAD"};
static const char *s_pP2Enum[] = {"Joy", "Arrows"};
static const char *s_pOnOffEnum[] = {"OFF", "ON"};

static tOption s_pOptions[] = {
	{"Start game", OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onStart}},
	{"Mode", OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_isChallenge, .ubMax = 1, .isCyclic = 1, .ubDefault = 0,
		.pEnumLabels = s_pModeEnum
	}},
	{"Players", OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_is2pPlaying, .ubMax = 1, .isCyclic = 0, .ubDefault = 0,
		.pEnumLabels = s_pPlayersEnum
	}},
	{"Player 1 controls", OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_is1pKbd, .ubMax = 1, .isCyclic = 1, .ubDefault = 0,
		.pEnumLabels = s_pP1Enum
	}},
	{"Player 2 controls", OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_is2pKbd, .ubMax = 1, .isCyclic = 1, .ubDefault = 1,
		.pEnumLabels = s_pP2Enum
	}},
	{"ATARI Mode", OPTION_TYPE_UINT8, .isHidden = 1, .sOptUb = {
		.pVar = &g_isAtari, .ubMax = 1, .isCyclic = 0, .ubDefault = 0,
		.pEnumLabels = s_pOnOffEnum
	}},
	{"Hi-Scores", OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onShowScores}},
	{"Exit to workbench", OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onExit}},
};
#define MENU_POS_COUNT (sizeof(s_pOptions) / sizeof(tOption))

//-------------------------------------------------------------- MENU COMP START

#define MENU_COLOR_INACTIVE 13
#define MENU_COLOR_ACTIVE 15

static UBYTE s_ubActivePos;
static UBYTE s_isScoreShowAfterRollIn = 0;

static void menuSetText(UBYTE ubPos, tTextBitMap *pTextBitmap) {
	char szBfr[50];
	if(s_pOptions[ubPos].eOptionType == OPTION_TYPE_UINT8) {
		if(s_pOptions[ubPos].sOptUb.pEnumLabels) {
			sprintf(
				szBfr, "%s: %s", s_pOptions[ubPos].szName,
				s_pOptions[ubPos].sOptUb.pEnumLabels[*s_pOptions[ubPos].sOptUb.pVar]
			);
		}
		else {
			sprintf(
				szBfr, "%s: %hhu", s_pOptions[ubPos].szName,
				*s_pOptions[ubPos].sOptUb.pVar
			);
		}
		fontFillTextBitMap(g_pFont, pTextBitmap, szBfr);
	}
	else if(s_pOptions[ubPos].eOptionType == OPTION_TYPE_CALLBACK) {
		fontFillTextBitMap(g_pFont, pTextBitmap, s_pOptions[ubPos].szName);
	}
}

static void menuSetHidden(UBYTE ubPos, UBYTE isHidden) {
	s_pOptions[ubPos].isHidden = isHidden;
}

static UBYTE menuNavigate(BYTE bDir) {
	WORD wNewPos = s_ubActivePos;

	// Find next non-hidden pos
	do {
		wNewPos += bDir;
	} while(0 < wNewPos && wNewPos < (WORD)MENU_POS_COUNT && s_pOptions[wNewPos].isHidden);

	if(wNewPos < 0 || wNewPos >= (WORD)MENU_POS_COUNT) {
		// Out of bounds - cancel
		return 0;
	}

	// Update active pos and mark as dirty
	s_pOptions[s_ubActivePos].isDirty = 1;
	s_pOptions[wNewPos].isDirty = 1;
	s_ubActivePos = wNewPos;
	return 1;
}

static UBYTE menuToggle(BYTE bDelta) {
	if(s_pOptions[s_ubActivePos].eOptionType == OPTION_TYPE_UINT8) {
		WORD wNewVal = *s_pOptions[s_ubActivePos].sOptUb.pVar + bDelta;
		if(wNewVal < 0 || wNewVal > s_pOptions[s_ubActivePos].sOptUb.ubMax) {
			if(s_pOptions[s_ubActivePos].sOptUb.isCyclic) {
				wNewVal = wNewVal < 0 ? s_pOptions[s_ubActivePos].sOptUb.ubMax : 0;
			}
			else {
				return 0; // Out of bounds on non-cyclic option
			}
		}
		*s_pOptions[s_ubActivePos].sOptUb.pVar = wNewVal;
		s_pOptions[s_ubActivePos].isDirty = 1;
		return 1;
	}
	return 0;
}

static UBYTE menuEnter(void) {
	if(s_pOptions[s_ubActivePos].eOptionType == OPTION_TYPE_CALLBACK) {
		s_pOptions[s_ubActivePos].sOptCb.cbSelect();
		return 1;
	}
	return 0;
}

//---------------------------------------------------------------- MENU COMP END

tSample *s_pSampleEnter, *s_pSampleToggle, *s_pSampleNavigate, *s_pSampleAtari;

static UBYTE s_pKeyHistory[8] = {0};
static UBYTE s_pKeyKonami[8] = {
	KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_DOWN, KEY_UP, KEY_UP
};

static tMenuState s_eMenuState;
static tBitMap *s_pLogo;
static UWORD s_uwOffsY;
static tTextBitMap *s_pTextBitmap;

static void menuEnableAtari(void) {
	UBYTE isAtariHidden = s_pOptions[5].isHidden;
	if(isAtariHidden) {
		menuSetHidden(5, 0);
		audioPlay(AUDIO_CHANNEL_1, s_pSampleAtari, AUDIO_VOLUME_MAX, 1);
		s_pOptions[5].isDirty = 1;
	}
}

void onStart(void) {
	audioPlay(AUDIO_CHANNEL_0, s_pSampleEnter, AUDIO_VOLUME_MAX, 1);
	gameStart();
	commHide();
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

void menuGsLoopScore(void);

void onShowScores(void) {
	commClearDisplay();
	hiScoreSetup(0, 0);
	hiScoreDrawAll(g_pMainBuffer->pScroll->pFront);
	gameChangeLoop(menuGsLoopScore);
}

void onExit(void) {
	gameClose();
}

void menuInitialDraw(tBitMap *pDisplayBuffer) {
	commClearDisplay();
	UWORD uwLogoWidth = bitmapGetByteWidth(s_pLogo)*8;
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	blitCopy(
		s_pLogo, 0, 0, pDisplayBuffer,
		sOrigin.uwX + (COMM_DISPLAY_WIDTH - uwLogoWidth) / 2, sOrigin.uwY,
		uwLogoWidth, s_pLogo->Rows, MINTERM_COOKIE, 0xFF
	);

	char szVersion[15];
	sprintf(szVersion, "v.%d.%d.%d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY);
	fontFillTextBitMap(g_pFont, s_pTextBitmap, szVersion);
	fontDrawTextBitMap(
		pDisplayBuffer, s_pTextBitmap,
		sOrigin.uwX + COMM_DISPLAY_WIDTH / 2,
		sOrigin.uwY + COMM_DISPLAY_HEIGHT,
		MENU_COLOR_ACTIVE,
		FONT_LAZY | FONT_HCENTER | FONT_COOKIE | FONT_SHADOW | FONT_BOTTOM
	);
	memset(s_pKeyHistory, 0, 8);
	for(UBYTE ubMenuPos = 0; ubMenuPos < MENU_POS_COUNT; ++ubMenuPos) {
		s_pOptions[ubMenuPos].isDirty = 1;
	}
}

void menuPreload(void) {
	s_pLogo = bitmapCreateFromFile("data/logo.bm", 0);
	s_pTextBitmap = fontCreateTextBitMap(320, g_pFont->uwHeight);

	s_pSampleEnter = sampleCreateFromFile("data/sfx/menu_enter.raw8", 22050);
	s_pSampleToggle = sampleCreateFromFile("data/sfx/menu_toggle.raw8", 22050);
	s_pSampleNavigate = sampleCreateFromFile("data/sfx/menu_navigate.raw8", 22050);
	s_pSampleAtari = sampleCreateFromFile("data/sfx/atari.raw8", 22050);
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	fontDestroyTextBitMap(s_pTextBitmap);

	sampleDestroy(s_pSampleEnter);
	sampleDestroy(s_pSampleToggle);
	sampleDestroy(s_pSampleNavigate);
	sampleDestroy(s_pSampleAtari);
}

void menuGsCreate(void) {
	s_eMenuState = MENU_STATE_ROLL_IN;
}

static void menuProcessSelecting(void) {
	commProcess();
	tUwCoordYX sOrigin = commGetOriginDisplay();
	UWORD uwOffsY = sOrigin.uwY + s_pLogo->Rows + 10;
	for(UBYTE ubMenuPos = 0; ubMenuPos < MENU_POS_COUNT; ++ubMenuPos) {
		if(!s_pOptions[ubMenuPos].isHidden && s_pOptions[ubMenuPos].isDirty) {
			blitRect(
				g_pMainBuffer->pScroll->pFront, sOrigin.uwX, uwOffsY,
				COMM_DISPLAY_WIDTH, g_pFont->uwHeight, COMM_DISPLAY_COLOR_BG
			);
			menuSetText(ubMenuPos, s_pTextBitmap);
			fontDrawTextBitMap(
				g_pMainBuffer->pScroll->pFront, s_pTextBitmap,
				sOrigin.uwX + COMM_DISPLAY_WIDTH / 2, uwOffsY,
				(ubMenuPos == s_ubActivePos) ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE,
				FONT_LAZY | FONT_HCENTER | FONT_COOKIE | FONT_SHADOW
			);
			s_pOptions[ubMenuPos].isDirty = 0;
		}
		uwOffsY += g_pFont->uwHeight + 2;
	}

	UBYTE ubNewKey = 0;
	if(commNavUse(COMM_NAV_UP)) {
		ubNewKey = KEY_UP;
		if(menuNavigate(-1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleNavigate, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(commNavUse(COMM_NAV_DOWN)) {
		ubNewKey = KEY_DOWN;
		if(menuNavigate(+1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleNavigate, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(commNavUse(COMM_NAV_LEFT)) {
		ubNewKey = KEY_LEFT;
		if(menuToggle(-1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleToggle, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(commNavUse(COMM_NAV_RIGHT)) {
		ubNewKey = KEY_RIGHT;
		if(menuToggle(+1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleToggle, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(commNavUse(COMM_NAV_BTN)) {
		menuEnter();
	}

	if(ubNewKey) {
		memmove(s_pKeyHistory+1, s_pKeyHistory, 8-1);
		s_pKeyHistory[0] = ubNewKey;
		if(!memcmp(s_pKeyHistory, s_pKeyKonami, 8)) {
			menuEnableAtari();
		}
	}
}

static void menuProcessRollIn(void) {
	tileBufferQueueProcess(g_pMainBuffer);
	bobNewDiscardUndraw();

	UWORD *pCamY = &g_pMainBuffer->pCamera->uPos.uwY;
	UWORD uwAvailHeight = g_pMainBuffer->pScroll->uwBmAvailHeight;
	if(*pCamY < uwAvailHeight) {
		*pCamY += 4;
	}
	else if(*pCamY > uwAvailHeight) {
		*pCamY = uwAvailHeight + (*pCamY % uwAvailHeight);
		if(*pCamY - uwAvailHeight > 4) {
			*pCamY -= 4;
		}
		else {
			*pCamY = uwAvailHeight;
		}
	}
	else {
		s_eMenuState = MENU_STATE_SELECTING;
		s_ubActivePos = 0;
		s_uwOffsY = 16 + s_pLogo->Rows + 30;

		if(!commShow()) {
			// TODO do something
		}
		if(s_isScoreShowAfterRollIn) {
			hiScoreDrawAll(g_pMainBuffer->pScroll->pBack);
			gameChangeLoop(menuGsLoopScore);
		}
		else {
			menuInitialDraw(g_pMainBuffer->pScroll->pBack);
		}
	}

	baseTileProcess();
	groundLayerProcess(*pCamY, 0xF);
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
}

static void menuProcessRollOut(void) {
	tileBufferQueueProcess(g_pMainBuffer);

	UWORD *pCamY = &g_pMainBuffer->pCamera->uPos.uwY;
	if(*pCamY >= 64) {
		*pCamY -= 4;
	}
	else {
		gamePopState();
	}

	baseTileProcess();
	groundLayerProcess(*pCamY, 0xF);
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
}

void menuGsLoop(void) {
  if(keyUse(KEY_ESCAPE)) {
    gameClose();
		return;
  }

	switch(s_eMenuState) {
		case MENU_STATE_ROLL_IN:
			menuProcessRollIn();
			break;
		case MENU_STATE_SELECTING:
			menuProcessSelecting();
			break;
		case MENU_STATE_ROLL_OUT:
			menuProcessRollOut();
	}
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void menuGsLoopScore(void) {
	if(hiScoreIsEntering()) {
		hiScoreEnteringProcess(g_pMainBuffer->pScroll->pFront);
	}
	else {
		if(
			keyUse(KEY_ESCAPE) || keyUse(KEY_RETURN) ||
			joyUse(JOY1_FIRE) || joyUse(JOY2_FIRE)
		) {
			menuInitialDraw(g_pMainBuffer->pScroll->pFront);
			gameChangeLoop(menuGsLoop);
			return;
		}
	}
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void menuGsDestroy(void) {

}

void menuGsEnter(UBYTE isScoreShow) {
	// Switch to menu, after popping it will process gameGsLoop
	s_isScoreShowAfterRollIn = isScoreShow;
	gamePushState(menuGsCreate, menuGsLoop, menuGsDestroy);
}
