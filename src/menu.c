/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "menu.h"
#include <ace/managers/key.h>
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
	MENU_POS_EXIT
} tMenuPos;

#define MENU_COLOR_INACTIVE 10
#define MENU_COLOR_ACTIVE 14

static tMenuState s_eMenuState;
static tMenuPos s_eActivePos;
static tBitMap *s_pLogo, *s_pLogoMask;
static tTextBob s_pMenuPositions[4];
static tBobNew s_sBobLogo;
static UWORD s_uwOffsY;

void menuPreload(void) {
	s_pLogo = bitmapCreateFromFile("data/logo.bm");
	s_pLogoMask = bitmapCreateFromFile("data/logo_mask.bm");
	bobNewInit(
		&s_sBobLogo, bitmapGetByteWidth(s_pLogo) * 8, s_pLogo->Rows,
		0, s_pLogo, s_pLogoMask, 0, 0
	);
	for(UBYTE i = 0; i < 4; ++i) {
		textBobCreate(&s_pMenuPositions[i], g_pFont, "Some longish menu text");
	}
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	bitmapDestroy(s_pLogoMask);
	for(UBYTE i = 0; i < 4; ++i) {
		textBobDestroy(&s_pMenuPositions[i]);
	}
}

void menuGsCreate(void) {
	s_eMenuState = MENU_STATE_ROLL_IN;
}

void menuGsLoop(void) {
  if(keyCheck(KEY_ESCAPE)) {
    gameClose();
		return;
  }

	bobNewBegin();

	switch(s_eMenuState) {
		case MENU_STATE_ROLL_IN: {
			if(g_pMainBuffer->pCamera->uPos.sUwCoord.uwY < g_pMainBuffer->pScroll->uwBmAvailHeight) {
				g_pMainBuffer->pCamera->uPos.sUwCoord.uwY += 4;
			}
			else {
				s_eMenuState = MENU_STATE_SELECTING;
				s_eActivePos = MENU_POS_START;
				s_sBobLogo.sPos.ulYX = g_pMainBuffer->pCamera->uPos.ulYX;
				s_sBobLogo.sPos.sUwCoord.uwX += (320 - s_sBobLogo.uwWidth)/2;
				s_sBobLogo.sPos.sUwCoord.uwY += 16;
				s_uwOffsY = s_sBobLogo.sPos.sUwCoord.uwY + s_sBobLogo.uwHeight + 50;
				const char * const pMenuTexts[4] = {
					"Start game", "Mode: free play", "Players: 1", "Exit to Workbench"
				};
				for(UBYTE i = 0; i < 4; ++i) {
					textBobSet(
						&s_pMenuPositions[i], pMenuTexts[i],
						i == 0 ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE,
						160 + 32 - s_pMenuPositions[i].uwWidth / 2,
						s_uwOffsY + i * 10, 0
					);
					textBobUpdate(&s_pMenuPositions[i]);
				}
			}
		} break;

		case MENU_STATE_SELECTING: {
			if(keyUse(KEY_UP) || keyUse(KEY_W)) {
				if(s_eActivePos) {
					textBobChangeColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_INACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
					--s_eActivePos;
					textBobChangeColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_ACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
				}
			}
			else if(keyUse(KEY_DOWN) || keyUse(KEY_S)) {
				if(s_eActivePos < 4-1) {
					textBobChangeColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_INACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
					++s_eActivePos;
					textBobChangeColor(&s_pMenuPositions[s_eActivePos], MENU_COLOR_ACTIVE);
					textBobUpdate(&s_pMenuPositions[s_eActivePos]);
				}
			}
			else if(keyUse(KEY_LEFT) || keyUse(KEY_A)) {
				if(s_eActivePos == MENU_POS_PLAYERS && g_is2pPlaying) {
					g_is2pPlaying = 0;
					textBobChangeText(&s_pMenuPositions[MENU_POS_PLAYERS], "Players: 1");
					textBobUpdate(&s_pMenuPositions[MENU_POS_PLAYERS]);
				}
			}
			else if(keyUse(KEY_RIGHT) || keyUse(KEY_D)) {
				if(s_eActivePos == MENU_POS_PLAYERS && !g_is2pPlaying) {
					g_is2pPlaying = 1;
					textBobChangeText(&s_pMenuPositions[MENU_POS_PLAYERS], "Players: 2");
					textBobUpdate(&s_pMenuPositions[MENU_POS_PLAYERS]);
				}
			}
			else if(keyUse(KEY_RETURN) || keyUse(KEY_SPACE)) {
				if(s_eActivePos == MENU_POS_START) {
					s_eMenuState = MENU_STATE_ROLL_OUT;
				}
				else if(s_eActivePos == MENU_POS_EXIT) {
					gameClose();
					return;
				}
			}
			bobNewPush(&s_sBobLogo);
			bobNewPush(&s_pMenuPositions[MENU_POS_START].sBob);
			bobNewPush(&s_pMenuPositions[MENU_POS_MODE].sBob);
			bobNewPush(&s_pMenuPositions[MENU_POS_PLAYERS].sBob);
			bobNewPush(&s_pMenuPositions[MENU_POS_EXIT].sBob);
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
