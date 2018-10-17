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

tSample *s_pSampleEnter, *s_pSampleToggle, *s_pSampleNavigate, *s_pSampleAtari;

typedef enum _tMenuState {
	MENU_STATE_ROLL_IN = 0,
	MENU_STATE_SELECTING,
	MENU_STATE_ROLL_OUT
} tMenuState;

typedef enum _tMenuPos {
	MENU_POS_START,
	MENU_POS_MODE,
	MENU_POS_PLAYERS,
	MENU_POS_P1_CONTROLS,
	MENU_POS_P2_CONTROLS,
	MENU_POS_ATARI,
	MENU_POS_EXIT,
	MENU_POS_COUNT
} tMenuPos;

static char * const s_pMenuTexts[MENU_POS_COUNT] = {
	"Start game", "Mode: free playyyy", "Players: 10",
	"Player 1 controls: arrows", "Player 2 controls: arrows",
	"ATARI MODE: OFF", "Exit to Workbench"
};

static UBYTE s_pKeyHistory[8] = {0};
static UBYTE s_pKeyKonami[8] = {
	KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_DOWN, KEY_UP, KEY_UP
};

#define MENU_COLOR_INACTIVE 10
#define MENU_COLOR_ACTIVE 14

static tMenuState s_eMenuState;
static tMenuPos s_eActivePos;
static tBitMap *s_pLogo, *s_pLogoMask;
static tTextBob s_pMenuPositions[MENU_POS_COUNT];
static tTextBob s_sCredits;
static tBobNew s_sBobLogo;
static UWORD s_uwOffsY;
static UBYTE s_isAtariDisplayed = 0;

void menuPreload(void) {
	s_pLogo = bitmapCreateFromFile("data/logo.bm");
	s_pLogoMask = bitmapCreateFromFile("data/logo_mask.bm");
	bobNewInit(
		&s_sBobLogo, bitmapGetByteWidth(s_pLogo) * 8, s_pLogo->Rows,
		0, s_pLogo, s_pLogoMask, 0, 0
	);
	for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
		textBobCreate(&s_pMenuPositions[i], g_pFont, "Some very very long menu text");
	}
	const char *szCredits = "Code: KaiN, Gfx: Softiron, Tests: Rav.En";
	textBobCreate(&s_sCredits, g_pFont, szCredits);
	textBobSetText(&s_sCredits, szCredits);
	textBobSetColor(&s_sCredits, 15);

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

	sampleDestroy(s_pSampleEnter);
	sampleDestroy(s_pSampleToggle);
	sampleDestroy(s_pSampleNavigate);
	sampleDestroy(s_pSampleAtari);
}

void menuGsCreate(void) {
	s_eMenuState = MENU_STATE_ROLL_IN;
	memset(s_pKeyHistory, 0, 8);
}

static void menuChangeText(tMenuPos ePos, const char *szFmt, ...) {
	va_list vaArgs;
	va_start(vaArgs, szFmt);
	vsprintf(s_pMenuTexts[ePos], szFmt, vaArgs);
	va_end(vaArgs);
	textBobSetText(&s_pMenuPositions[ePos], s_pMenuTexts[ePos]);
	textBobUpdate(&s_pMenuPositions[ePos]);
}

static void menuEnableAtari(void) {
	if(!s_isAtariDisplayed) {
		s_isAtariDisplayed = 1;
		audioPlay(AUDIO_CHANNEL_1, s_pSampleAtari, AUDIO_VOLUME_MAX, 1);
	}
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
				s_eActivePos = MENU_POS_START;
				s_sBobLogo.sPos.ulYX = g_pMainBuffer->pCamera->uPos.ulYX;
				s_sBobLogo.sPos.sUwCoord.uwX += (320 - s_sBobLogo.uwWidth)/2;
				s_sBobLogo.sPos.sUwCoord.uwY += 16;
				s_uwOffsY = s_sBobLogo.sPos.sUwCoord.uwY + s_sBobLogo.uwHeight + 30;
				sprintf(s_pMenuTexts[MENU_POS_MODE], g_isChallenge ? "Mode: Challenge" : "Mode: Free play");
				sprintf(s_pMenuTexts[MENU_POS_PLAYERS], g_is2pPlaying ? "Players: 2" : "Players: 1");
				sprintf(s_pMenuTexts[MENU_POS_P1_CONTROLS], "Player 1 controls: %s", g_is1pKbd ? "WSAD" : "Joy");
				sprintf(s_pMenuTexts[MENU_POS_P2_CONTROLS], "Player 2 controls: %s", g_is2pKbd ? "Arrows" : "Joy");
				sprintf(s_pMenuTexts[MENU_POS_ATARI], "ATARI MODE: %s", g_isAtari ? "On" : "Off");
				for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
					textBobSet(
						&s_pMenuPositions[i], s_pMenuTexts[i],
						i == 0 ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE,
						g_pMainBuffer->pCamera->uPos.sUwCoord.uwX + 160, s_uwOffsY + i * 10,
						0, 1
					);
					textBobUpdate(&s_pMenuPositions[i]);
				}
				textBobSetPos(
					&s_sCredits, g_pMainBuffer->pCamera->uPos.sUwCoord.uwX + 160,
					g_pMainBuffer->pCamera->uPos.sUwCoord.uwY + g_pMainBuffer->pCamera->sCommon.pVPort->uwHeight - 15, 0, 1
				);
				textBobUpdate(&s_sCredits);
			}
		} break;

		case MENU_STATE_SELECTING: {
			UBYTE ubNewKey = 0;
			UBYTE isToggle = 0;
			if(
				keyUse(KEY_UP) || keyUse(KEY_W) ||
				joyUse(JOY1_UP) || joyUse(JOY2_UP)
			) {
				ubNewKey = KEY_UP;
				if(s_eActivePos) {
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_INACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
					--s_eActivePos;
					if(!s_isAtariDisplayed && s_eActivePos == MENU_POS_ATARI) {
						--s_eActivePos;
					}
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_ACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
				}
			}
			else if(
				keyUse(KEY_DOWN) || keyUse(KEY_S) ||
				joyUse(JOY1_DOWN) || joyUse(JOY2_DOWN)
			) {
				ubNewKey = KEY_DOWN;
				if(s_eActivePos < MENU_POS_COUNT-1) {
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_INACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
					++s_eActivePos;
					if(!s_isAtariDisplayed && s_eActivePos == MENU_POS_ATARI) {
						++s_eActivePos;
					}
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_ACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
				}
			}
			else if(
				keyUse(KEY_LEFT) || keyUse(KEY_A) || joyUse(JOY1_LEFT) || joyUse(JOY2_LEFT)
			) {
				ubNewKey = KEY_LEFT;
			}
			else if(
				keyUse(KEY_RIGHT) || keyUse(KEY_D)|| joyUse(JOY1_RIGHT) || joyUse(JOY2_RIGHT)
			) {
				ubNewKey = KEY_RIGHT;
			}
			if(ubNewKey == KEY_LEFT || ubNewKey == KEY_RIGHT) {
				isToggle = 1;
				if(s_eActivePos == MENU_POS_PLAYERS) {
					g_is2pPlaying = !g_is2pPlaying;
					// ?: aint working here, wtf
					if(g_is2pPlaying) {
						menuChangeText(MENU_POS_PLAYERS, "Players: 2");
					}
					else {
						menuChangeText(MENU_POS_PLAYERS, "Players: 1");
					}
				}
				else if(s_eActivePos == MENU_POS_P1_CONTROLS) {
					g_is1pKbd = !g_is1pKbd;
					menuChangeText(MENU_POS_P1_CONTROLS, "Player 1 controls: %s", g_is1pKbd ? "WSAD" : "Joy");
				}
				else if(s_eActivePos == MENU_POS_P2_CONTROLS) {
					g_is2pKbd = !g_is2pKbd;
					menuChangeText(MENU_POS_P2_CONTROLS, "Player 2 controls: %s", g_is2pKbd ? "Arrows" : "Joy");
				}
				else if(s_eActivePos == MENU_POS_MODE) {
					g_isChallenge = !g_isChallenge;
					if(g_isChallenge) {
						menuChangeText(MENU_POS_MODE, "Mode: Challenge");
					}
					else {
						menuChangeText(MENU_POS_MODE, "Mode: Free play");
					}
				}
				else if(s_eActivePos == MENU_POS_ATARI) {
					g_isAtari = !g_isAtari;
					if(g_isAtari) {
						menuChangeText(MENU_POS_ATARI, "ATARI MODE: On");
					}
					else {
						menuChangeText(MENU_POS_ATARI, "ATARI MODE: Off");
					}
				}
				else {
					isToggle = 0;
				}
			}
			else if(
				keyUse(KEY_RETURN) || keyUse(KEY_SPACE) ||
				joyUse(JOY1_FIRE) || joyUse(JOY1_FIRE)
			) {
				if(s_eActivePos == MENU_POS_START) {
					audioPlay(AUDIO_CHANNEL_0, s_pSampleEnter, AUDIO_VOLUME_MAX, 1);
					gameStart();
					s_eMenuState = MENU_STATE_ROLL_OUT;
				}
				else if(s_eActivePos == MENU_POS_EXIT) {
					gameClose();
					return;
				}
			}

			if(ubNewKey) {
				audioPlay(
					AUDIO_CHANNEL_0,
					isToggle ? s_pSampleToggle : s_pSampleNavigate, AUDIO_VOLUME_MAX, 1
				);
				memmove(s_pKeyHistory+1, s_pKeyHistory, 8-1);
				s_pKeyHistory[0] = ubNewKey;
				if(!memcmp(s_pKeyHistory, s_pKeyKonami, 8)) {
					menuEnableAtari();
				}
			}

			bobNewPush(&s_sBobLogo);
			for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
				if(i != MENU_POS_ATARI || s_isAtariDisplayed) {
					bobNewPush(&s_pMenuPositions[i].sBob);
				}
			}
			bobNewPush(&s_sCredits.sBob);
		} break;

		case MENU_STATE_ROLL_OUT: {
			if(g_pMainBuffer->pCamera->uPos.sUwCoord.uwY) {
				g_pMainBuffer->pCamera->uPos.sUwCoord.uwY -= 4;
			}
			else {
				gamePopState();
			}
		}
	}

	bobNewPushingDone();
	bobNewEnd();

	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void menuGsDestroy(void) {

}
