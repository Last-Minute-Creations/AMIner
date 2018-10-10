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
	MENU_POS_EXIT,
	MENU_POS_COUNT
} tMenuPos;

static char * const s_pMenuTexts[MENU_POS_COUNT] = {
	"Start game", "Mode: free playyyy", "Players: 10",
	"Player 1 controls: arrows", "Player 2 controls: arrows",
	"Exit to Workbench"
};

#define MENU_COLOR_INACTIVE 10
#define MENU_COLOR_ACTIVE 14

static tMenuState s_eMenuState;
static tMenuPos s_eActivePos;
static tBitMap *s_pLogo, *s_pLogoMask;
static tTextBob s_pMenuPositions[MENU_POS_COUNT];
static tBobNew s_sBobLogo;
static UWORD s_uwOffsY;

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
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	bitmapDestroy(s_pLogoMask);
	for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
		textBobDestroy(&s_pMenuPositions[i]);
	}
}

void menuGsCreate(void) {
	s_eMenuState = MENU_STATE_ROLL_IN;
}

static void menuChangeText(tMenuPos ePos, const char *szFmt, ...) {
	va_list vaArgs;
	va_start(vaArgs, szFmt);
	vsprintf(s_pMenuTexts[ePos], szFmt, vaArgs);
	va_end(vaArgs);
	textBobSetText(&s_pMenuPositions[ePos], s_pMenuTexts[ePos]);
	textBobUpdate(&s_pMenuPositions[ePos]);
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
				s_uwOffsY = s_sBobLogo.sPos.sUwCoord.uwY + s_sBobLogo.uwHeight + 50;
				sprintf(s_pMenuTexts[MENU_POS_MODE], g_isChallenge ? "Mode: challenge" : "Mode: free play");
				sprintf(s_pMenuTexts[MENU_POS_PLAYERS], g_is2pPlaying ? "Players: 2" : "Players: 1");
				sprintf(s_pMenuTexts[MENU_POS_P1_CONTROLS], "Player 1 controls: %s", g_is1pKbd ? "WSAD" : "Joy");
				sprintf(s_pMenuTexts[MENU_POS_P2_CONTROLS], "Player 2 controls: %s", g_is2pKbd ? "Arrows" : "Joy");
				for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
					textBobSet(
						&s_pMenuPositions[i], s_pMenuTexts[i],
						i == 0 ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE,
						g_pMainBuffer->pCamera->uPos.sUwCoord.uwX + 160, s_uwOffsY + i * 10,
						0, 1
					);
					textBobUpdate(&s_pMenuPositions[i]);
				}
			}
		} break;

		case MENU_STATE_SELECTING: {
			if(
				keyUse(KEY_UP) || keyUse(KEY_W) ||
				joyUse(JOY1_UP) || joyUse(JOY2_UP)
			) {
				if(s_eActivePos) {
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_INACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
					--s_eActivePos;
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_ACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
				}
			}
			else if(
				keyUse(KEY_DOWN) || keyUse(KEY_S) ||
				joyUse(JOY1_DOWN) || joyUse(JOY2_DOWN)
			) {
				if(s_eActivePos < MENU_POS_COUNT-1) {
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_INACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
					++s_eActivePos;
					textBobSetColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_ACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
				}
			}
			else if(
				keyUse(KEY_LEFT) || keyUse(KEY_A) || keyUse(KEY_RIGHT) || keyUse(KEY_D) ||
				joyUse(JOY1_LEFT) || joyUse(JOY2_LEFT) || joyUse(JOY1_RIGHT) || joyUse(JOY2_RIGHT)
			) {
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
						menuChangeText(MENU_POS_MODE, "Mode: challenge");
					}
					else {
						menuChangeText(MENU_POS_MODE, "Mode: free play");
					}
				}
			}
			else if(
				keyUse(KEY_RETURN) || keyUse(KEY_SPACE) ||
				joyUse(JOY1_FIRE) || joyUse(JOY1_FIRE)
			) {
				if(s_eActivePos == MENU_POS_START) {
					gameStart();
					s_eMenuState = MENU_STATE_ROLL_OUT;
				}
				else if(s_eActivePos == MENU_POS_EXIT) {
					gameClose();
					return;
				}
			}
			bobNewPush(&s_sBobLogo);
			for(UBYTE i = 0; i < MENU_POS_COUNT; ++i) {
				bobNewPush(&s_pMenuPositions[i].sBob);
			}
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
