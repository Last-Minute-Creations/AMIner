/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "menu.h"
#include <ace/managers/system.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <comm/base.h>
#include "game.h"
#include "bob_new.h"
#include "text_bob.h"
#include "ground_layer.h"
#include "base_tile.h"
#include "hi_score.h"
#include "hud.h"
#include "core.h"

#define SFX_CHANNEL_ATARI 3

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
	tOptionType eOptionType;
	UBYTE isHidden;
	UBYTE isDirty;
	union {
		struct {
			UBYTE *pVar;
			UBYTE ubMax;
			UBYTE ubDefault;
			UBYTE isCyclic;
			char *** const pEnumLabels;
		} sOptUb;
		struct {
			tOptionSelectCb cbSelect;
		} sOptCb;
	};
} tOption;

void onStart(void);
void onLoad(void);
void onExit(void);
void onShowScores(void);

char **g_pMenuCaptions, **g_pMenuEnumMode, **g_pMenuEnumP1, **g_pMenuEnumP2,
	**g_pMenuEnumOnOff, **g_pMenuEnumPlayerCount;
tPtplayerMod *g_pMenuMod;

static tState s_sStateMenuScore;

static tOption s_pOptions[] = {
	{OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onStart}},
	{OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onLoad}},
	{OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_isChallenge, .ubMax = 1, .isCyclic = 1, .ubDefault = 0,
		.pEnumLabels = &g_pMenuEnumMode
	}},
	{OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_is2pPlaying, .ubMax = 1, .isCyclic = 0, .ubDefault = 0,
		.pEnumLabels = &g_pMenuEnumPlayerCount
	}},
	{OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_is1pKbd, .ubMax = 1, .isCyclic = 1, .ubDefault = 0,
		.pEnumLabels = &g_pMenuEnumP1
	}},
	{OPTION_TYPE_UINT8, .isHidden = 0, .sOptUb = {
		.pVar = &g_is2pKbd, .ubMax = 1, .isCyclic = 1, .ubDefault = 1,
		.pEnumLabels = &g_pMenuEnumP2
	}},
	{OPTION_TYPE_UINT8, .isHidden = 1, .sOptUb = {
		.pVar = &g_isAtari, .ubMax = 1, .isCyclic = 0, .ubDefault = 0,
		.pEnumLabels = &g_pMenuEnumOnOff
	}},
	{OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onShowScores}},
	{OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onExit}},
};
#define MENU_POS_COUNT (sizeof(s_pOptions) / sizeof(tOption))

//-------------------------------------------------------------- MENU COMP START

#define MENU_COLOR_INACTIVE 13
#define MENU_COLOR_ACTIVE 15

static UBYTE s_ubActivePos;
static UBYTE s_isScoreShowAfterRollIn = 0;

static void menuDrawPos(UBYTE ubPos, UWORD uwOffsTop) {
	UWORD uwOffsY = uwOffsTop + ubPos * (g_pFont->uwHeight);
	commErase(0, uwOffsY, COMM_DISPLAY_WIDTH, g_pFont->uwHeight);

	char szBfr[50];
	const char *szText = 0;
	if(s_pOptions[ubPos].eOptionType == OPTION_TYPE_UINT8) {
		if(s_pOptions[ubPos].sOptUb.pEnumLabels) {
			sprintf(
				szBfr, "%s: %s", g_pMenuCaptions[ubPos],
				(*s_pOptions[ubPos].sOptUb.pEnumLabels)[*s_pOptions[ubPos].sOptUb.pVar]
			);
		}
		else {
			sprintf(
				szBfr, "%s: %hhu", g_pMenuCaptions[ubPos],
				*s_pOptions[ubPos].sOptUb.pVar
			);
		}
		szText = szBfr;
	}
	else if(s_pOptions[ubPos].eOptionType == OPTION_TYPE_CALLBACK) {
		szText = g_pMenuCaptions[ubPos];
	}

	if(szText != 0) {
		commDrawText(COMM_DISPLAY_WIDTH / 2, uwOffsY, szText,
			FONT_LAZY | FONT_HCENTER | FONT_COOKIE | FONT_SHADOW,
			(ubPos == s_ubActivePos) ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE
		);
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

tPtplayerSfx *s_pSfxAtari;

static UBYTE s_pKeyHistory[8] = {0};
static UBYTE s_pKeyKonami[8] = {
	KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_DOWN, KEY_UP, KEY_UP
};

static tMenuState s_eMenuState;
static tBitMap *s_pLogo;
static UWORD s_uwOffsY;

static void menuEnableAtari(void) {
	UBYTE isAtariHidden = s_pOptions[5].isHidden;
	if(isAtariHidden) {
		menuSetHidden(5, 0);
		ptplayerSfxPlay(s_pSfxAtari, SFX_CHANNEL_ATARI, 64, 1);
		s_pOptions[5].isDirty = 1;
	}
}

void onStart(void) {
	commEraseAll();
	gameStart();
	commHide();
	// viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	// copProcessBlocks();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

void onLoad(void) {
	commEraseAll();
	gameStart();

	systemUse();
	tFile *pSave = fileOpen("save.dat", "rb");
	if(pSave) {
		if(!gameLoad(pSave)) {
			logWrite("ERR: Failed to load game\n");
		}
		fileClose(pSave);
	}
	else {
		logWrite("ERR: Save file not found\n");
	}
	systemUnuse();

	commHide();
	// viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	// copProcessBlocks();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

void onShowScores(void) {
	commEraseAll();
	hiScoreSetup(0, 0);
	statePush(g_pGameStateManager, &s_sStateMenuScore);
}

void onExit(void) {
	statePopAll(g_pGameStateManager);
}

void menuInitialDraw(tBitMap *pDisplayBuffer) {
	commEraseAll();
	UWORD uwLogoWidth = bitmapGetByteWidth(s_pLogo)*8;
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	blitCopy(
		s_pLogo, 0, 0, pDisplayBuffer,
		sOrigin.uwX + (COMM_DISPLAY_WIDTH - uwLogoWidth) / 2, sOrigin.uwY,
		uwLogoWidth, s_pLogo->Rows, MINTERM_COOKIE
	);

	const char szVersion[15] = "v." GAME_VERSION;
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, COMM_DISPLAY_HEIGHT, szVersion,
		FONT_LAZY | FONT_HCENTER | FONT_COOKIE | FONT_SHADOW | FONT_BOTTOM,
		COMM_DISPLAY_COLOR_TEXT
	);
	memset(s_pKeyHistory, 0, 8);
	for(UBYTE ubMenuPos = 0; ubMenuPos < MENU_POS_COUNT; ++ubMenuPos) {
		s_pOptions[ubMenuPos].isDirty = 1;
	}
}

void menuPreload(void) {
	s_pLogo = bitmapCreateFromFile("data/logo.bm", 0);
	s_pSfxAtari = ptplayerSfxCreateFromFile("data/sfx/atari.sfx");
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	ptplayerSfxDestroy(s_pSfxAtari);
}

static void menuGsCreate(void) {
	ptplayerLoadMod(g_pMenuMod, g_pModSampleData, 0);
	ptplayerEnableMusic(1);
	ptplayerConfigureSongRepeat(0, 0);
	s_eMenuState = MENU_STATE_ROLL_IN;
	hudShowMain();
}

static void menuProcessSelecting(void) {
	commProcess();
	hudUpdate();

	for(UBYTE ubMenuPos = 0; ubMenuPos < MENU_POS_COUNT; ++ubMenuPos) {
		if(!s_pOptions[ubMenuPos].isHidden && s_pOptions[ubMenuPos].isDirty) {
			menuDrawPos(ubMenuPos, s_pLogo->Rows + 10);
			s_pOptions[ubMenuPos].isDirty = 0;
		}
	}

	UBYTE ubNewKey = 0;
	if(commNavUse(COMM_NAV_UP)) {
		ubNewKey = KEY_UP;
		menuNavigate(-1);
	}
	else if(commNavUse(COMM_NAV_DOWN)) {
		ubNewKey = KEY_DOWN;
		menuNavigate(+1);
	}
	else if(commNavUse(COMM_NAV_LEFT)) {
		ubNewKey = KEY_LEFT;
		menuToggle(-1);
	}
	else if(commNavUse(COMM_NAV_RIGHT)) {
		ubNewKey = KEY_RIGHT;
		menuToggle(+1);
	}
	else if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		menuEnter();
	}

	if(ubNewKey) {
		memmove(s_pKeyHistory+1, s_pKeyHistory, 8-1);
		s_pKeyHistory[0] = ubNewKey;
		if(!memcmp(s_pKeyHistory, s_pKeyKonami, 8)) {
			menuEnableAtari();
		}
	}
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void menuProcessRollIn(void) {
	bobNewDiscardUndraw();
	coreProcessBeforeBobs();

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

		if(!commTryShow()) {
			// TODO do something
		}
		if(s_isScoreShowAfterRollIn) {
			statePush(g_pGameStateManager, &s_sStateMenuScore);
		}
		else {
			menuSetHidden(1, !fileExists("save.dat"));
			menuInitialDraw(g_pMainBuffer->pScroll->pBack);
		}
	}

	coreProcessAfterBobs();
}

static void menuProcessRollOut(void) {
	coreProcessBeforeBobs();

	UWORD *pCamY = &g_pMainBuffer->pCamera->uPos.uwY;
	if(*pCamY >= 64) {
		*pCamY -= 4;
	}
	else {
		stateChange(g_pGameStateManager, &g_sStateGame);
	}
	coreProcessAfterBobs();
}

static void menuGsLoop(void) {
  if(keyUse(KEY_ESCAPE)) {
    statePopAll(g_pGameStateManager);
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
}

static void menuGsDestroy(void) {

}

static void menuScoreGsCreate(void) {
	hiScoreDrawAll();
}

static void menuScoreGsLoop(void) {
	commProcess();
	if(hiScoreIsEnteringNew()) {
		hiScoreEnteringProcess();
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			statePop(g_pGameStateManager);
			return;
		}
	}
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void menuScoreGsDestroy(void) {
	menuInitialDraw(g_pMainBuffer->pScroll->pFront);
}

void menuGsEnter(UBYTE isScoreShow) {
	// Switch to menu, after popping it will process gameGsLoop
	s_isScoreShowAfterRollIn = isScoreShow;
	if(!isScoreShow) {
		hudReset(0, 0);
	}
	stateChange(g_pGameStateManager, &g_sStateMenu);
}

tState g_sStateMenu = {
	.cbCreate = menuGsCreate, .cbLoop = menuGsLoop, .cbDestroy = menuGsDestroy
};

static tState s_sStateMenuScore = {
	.cbCreate = menuScoreGsCreate, .cbLoop = menuScoreGsLoop, .cbDestroy = menuScoreGsDestroy
};
