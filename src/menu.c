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

static const char *s_pModeEnum[] = {"Free play", "Challenge"};
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
	{"Exit to workbench", OPTION_TYPE_CALLBACK, .isHidden = 0, .sOptCb = {.cbSelect = onExit}},
};
#define MENU_POS_COUNT (sizeof(s_pOptions) / sizeof(tOption))

//-------------------------------------------------------------- MENU COMP START

#define MENU_COLOR_INACTIVE 13
#define MENU_COLOR_ACTIVE 15

static UBYTE s_ubActivePos;
static tTextBob s_pMenuPositions[MENU_POS_COUNT];

static void menuUpdateText(UBYTE ubPos) {
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
		textBobSetText(&s_pMenuPositions[ubPos], szBfr);
	}
	else if(s_pOptions[ubPos].eOptionType == OPTION_TYPE_CALLBACK) {
		textBobSetText(&s_pMenuPositions[ubPos], s_pOptions[ubPos].szName);
	}
	textBobUpdate(&s_pMenuPositions[ubPos]);
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

	// Set old pos color to inactive
	textBobSetColor(&s_pMenuPositions[s_ubActivePos], MENU_COLOR_INACTIVE);
	textBobUpdate(&s_pMenuPositions[s_ubActivePos]);

	// Update active pos and set color to active
	s_ubActivePos = wNewPos;
	textBobSetColor(&s_pMenuPositions[s_ubActivePos], MENU_COLOR_ACTIVE);
	textBobUpdate(&s_pMenuPositions[s_ubActivePos]);

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
		menuUpdateText(s_ubActivePos);
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
static tBitMap *s_pLogo, *s_pLogoMask;
static tTextBob s_sCredits, s_sVersion;
static UWORD s_uwOffsY;

static void menuEnableAtari(void) {
	UBYTE isAtariHidden = s_pOptions[5].isHidden;
	if(isAtariHidden) {
		menuSetHidden(5, 0);
		audioPlay(AUDIO_CHANNEL_1, s_pSampleAtari, AUDIO_VOLUME_MAX, 1);
	}
}

void onStart(void) {
	audioPlay(AUDIO_CHANNEL_0, s_pSampleEnter, AUDIO_VOLUME_MAX, 1);
	gameStart();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

void onExit(void) {
	gameClose();
}

void menuPreload(void) {
	s_pLogo = bitmapCreateFromFile("data/logo.bm");
	s_pLogoMask = bitmapCreateFromFile("data/logo_mask.bm");
	for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
		textBobCreate(&s_pMenuPositions[i], g_pFont, "Some very very long menu text");
	}
	const char *szCredits = "Code: KaiN, Gfx: Softiron, Tests: Rav.En";
	textBobCreate(&s_sCredits, g_pFont, szCredits);
	textBobSetText(&s_sCredits, szCredits);
	textBobSetColor(&s_sCredits, MENU_COLOR_ACTIVE);

	char szVersion[15];
	sprintf(szVersion, "v.%02d.%02d.%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY);
	textBobCreate(&s_sVersion, g_pFont, szVersion);
	textBobSetText(&s_sVersion, szVersion);
	textBobSetColor(&s_sVersion, MENU_COLOR_ACTIVE);

	s_pSampleEnter = sampleCreateFromFile("data/sfx/menu_enter.raw8", 22050);
	s_pSampleToggle = sampleCreateFromFile("data/sfx/menu_toggle.raw8", 22050);
	s_pSampleNavigate = sampleCreateFromFile("data/sfx/menu_navigate.raw8", 22050);
	s_pSampleAtari = sampleCreateFromFile("data/sfx/atari.raw8", 22050);
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	bitmapDestroy(s_pLogoMask);
	for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
		textBobDestroy(&s_pMenuPositions[i]);
	}
	textBobDestroy(&s_sCredits);
	textBobDestroy(&s_sVersion);

	sampleDestroy(s_pSampleEnter);
	sampleDestroy(s_pSampleToggle);
	sampleDestroy(s_pSampleNavigate);
	sampleDestroy(s_pSampleAtari);
}

void menuGsCreate(void) {
	s_eMenuState = MENU_STATE_ROLL_IN;
	memset(s_pKeyHistory, 0, 8);
}

static void menuProcessSelecting(void) {
	UBYTE ubNewKey = 0;
	if(
		keyUse(KEY_UP) || keyUse(KEY_W) || joyUse(JOY1_UP) || joyUse(JOY2_UP)
	) {
		ubNewKey = KEY_UP;
		if(menuNavigate(-1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleNavigate, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(
		keyUse(KEY_DOWN) || keyUse(KEY_S) || joyUse(JOY1_DOWN) || joyUse(JOY2_DOWN)
	) {
		ubNewKey = KEY_DOWN;
		if(menuNavigate(+1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleNavigate, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(
		keyUse(KEY_LEFT) || keyUse(KEY_A) || joyUse(JOY1_LEFT) || joyUse(JOY2_LEFT)
	) {
		ubNewKey = KEY_LEFT;
		if(menuToggle(-1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleToggle, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(
		keyUse(KEY_RIGHT) || keyUse(KEY_D)|| joyUse(JOY1_RIGHT) || joyUse(JOY2_RIGHT)
	) {
		ubNewKey = KEY_RIGHT;
		if(menuToggle(+1)) {
			audioPlay(AUDIO_CHANNEL_0, s_pSampleToggle, AUDIO_VOLUME_MAX, 1);
		}
	}
	else if(
		keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || joyUse(JOY1_FIRE) || joyUse(JOY1_FIRE)
	) {
		menuEnter();
	}

	if(ubNewKey) {
		memmove(s_pKeyHistory+1, s_pKeyHistory, 8-1);
		s_pKeyHistory[0] = ubNewKey;
		if(!memcmp(s_pKeyHistory, s_pKeyKonami, 8)) {
			menuEnableAtari();
		}
	}

	for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
		if(!s_pOptions[i].isHidden) {
			bobNewPush(&s_pMenuPositions[i].sBob);
		}
	}
	bobNewPush(&s_sCredits.sBob);
	bobNewPush(&s_sVersion.sBob);
}

void menuGsLoop(void) {
  if(keyUse(KEY_ESCAPE)) {
    gameClose();
		return;
  }

	bobNewBegin();

	UWORD *pCamY = &g_pMainBuffer->pCamera->uPos.sUwCoord.uwY;
	UWORD uwAvailHeight = g_pMainBuffer->pScroll->uwBmAvailHeight;
	switch(s_eMenuState) {
		case MENU_STATE_ROLL_IN: {
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
				UWORD uwLogoWidth = bitmapGetByteWidth(s_pLogo)*8;
				UWORD uwOffsX = g_pMainBuffer->pCamera->uPos.sUwCoord.uwX;
				blitCopyMask(
					s_pLogo, 0, 0,
					g_pMainBuffer->pScroll->pBack, uwOffsX + (320 - uwLogoWidth)/2, 16,
					uwLogoWidth, s_pLogo->Rows, (UWORD*)s_pLogoMask->Planes[0]
				);
				blitCopyMask(
					s_pLogo, 0, 0,
					g_pMainBuffer->pScroll->pFront, uwOffsX + (320 - uwLogoWidth)/2, 16,
					uwLogoWidth, s_pLogo->Rows, (UWORD*)s_pLogoMask->Planes[0]
				);
				s_uwOffsY = 16 + s_pLogo->Rows + 30;
				for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
					textBobSetColor(
						&s_pMenuPositions[i],
						i == 0 ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE
					);
					menuUpdateText(i);
					textBobSetPos(&s_pMenuPositions[i], uwOffsX + 160, s_uwOffsY + i * 10, 0, 1);
				}
				textBobSetPos(
					&s_sCredits, uwOffsX + 160,
					*pCamY + g_pMainBuffer->pCamera->sCommon.pVPort->uwHeight - 15, 0, 1
				);
				textBobUpdate(&s_sCredits);
				textBobSetPos(
					&s_sVersion, uwOffsX + 320 - s_sVersion.sBob.uwWidth,
					*pCamY + 4, 0, 0
				);
				textBobUpdate(&s_sVersion);
			}
		} break;

		case MENU_STATE_SELECTING:
			menuProcessSelecting();
		break;

		case MENU_STATE_ROLL_OUT: {
			if(*pCamY) {
				*pCamY -= 4;
			}
			else {
				gamePopState();
			}
		}
	}

	bobNewPushingDone();
	bobNewEnd();
	groundLayerProcess(*pCamY);
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void menuGsDestroy(void) {

}
