#include "button.h"
#include <string.h>
#include <ace/utils/font.h>
#include "game.h"
#include "comm.h"

#define BUTTON_COUNT_MAX 5

static tButton s_pButtons[BUTTON_COUNT_MAX];
static UBYTE s_ubButtonCount;
static UBYTE s_ubSelected = BUTTON_INVALID;

void buttonRmAll(void) {
	for(UBYTE i = BUTTON_COUNT_MAX; i--;) {
		s_pButtons[i].szName[0] = '\0';
	}
	s_ubButtonCount = 0;
	s_ubSelected = BUTTON_INVALID;
}

UBYTE buttonAdd(const char *szName, UWORD uwX, UWORD uwY) {
	for(UBYTE i = 0; i < BUTTON_COUNT_MAX; ++i) {
		if(s_pButtons[i].szName[0] == '\0') {
			strcpy(s_pButtons[i].szName, szName);
			s_pButtons[i].sPos.uwX = uwX;
			s_pButtons[i].sPos.uwY = uwY;
			return i;
		}
	}
	return BUTTON_INVALID;
}

void buttonDraw(UBYTE ubIdx, tBitMap *pBfr, tTextBitMap *pTextBitmap) {
	UBYTE ubColor = (
		ubIdx == s_ubSelected ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);
	tUwCoordYX sSize = fontMeasureText(g_pFont, s_pButtons[ubIdx].szName);
	sSize.uwX += 5;
	sSize.uwY += 4;
	UWORD uwBtnX = s_pButtons[ubIdx].sPos.uwX - sSize.uwX / 2;
	UWORD uwBtnY = s_pButtons[ubIdx].sPos.uwY;
	blitRect(pBfr, uwBtnX, uwBtnY, sSize.uwX, 1, ubColor);
	blitRect(pBfr, uwBtnX, uwBtnY, 1, sSize.uwY, ubColor);
	blitRect(pBfr, uwBtnX, uwBtnY + sSize.uwY - 1, sSize.uwX, 1, ubColor);
	blitRect(pBfr, uwBtnX + sSize.uwX - 1, uwBtnY, 1, sSize.uwY, ubColor);
	fontFillTextBitMap(g_pFont, pTextBitmap, s_pButtons[ubIdx].szName);
	fontDrawTextBitMap(
		pBfr, pTextBitmap, uwBtnX + 3, uwBtnY  + 3,
		ubColor, FONT_COOKIE
	);
}

void buttonDrawAll(tBitMap *pBfr, tTextBitMap *pTextBitmap) {
	for(UBYTE i = 0; i < BUTTON_COUNT_MAX; ++i) {
		if(!s_pButtons[i].szName[0]) {
			break;
		}
		buttonDraw(i, pBfr, pTextBitmap);
	}
}

void buttonSelect(UBYTE ubIdx) {
	s_ubSelected = ubIdx;
}

UBYTE buttonGetSelected(void) {
	return s_ubSelected;
}
