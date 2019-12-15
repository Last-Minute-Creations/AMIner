#include "button.h"
#include <string.h>
#include <ace/utils/font.h>
#include "core.h"
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

void buttonDraw(UBYTE ubIdx, tBitMap *pBfr) {
	UBYTE ubColor = (
		ubIdx == s_ubSelected ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);
	tUwCoordYX sSize = fontMeasureText(g_pFont, s_pButtons[ubIdx].szName);
	sSize.uwX += 5;
	sSize.uwY += 4;
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	UWORD uwBtnX = s_pButtons[ubIdx].sPos.uwX - sSize.uwX / 2;
	UWORD uwBtnY = s_pButtons[ubIdx].sPos.uwY;
	blitRect(
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY, sSize.uwX, 1, ubColor
	);
	blitRect(
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY, 1, sSize.uwY, ubColor
	);
	blitRect(
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY + sSize.uwY - 1,
		sSize.uwX, 1, ubColor
	);
	blitRect(
		pBfr, sOrigin.uwX + uwBtnX + sSize.uwX - 1, sOrigin.uwY + uwBtnY,
		1, sSize.uwY, ubColor
	);
	commDrawText(
		uwBtnX + 3, uwBtnY  + 3, s_pButtons[ubIdx].szName, FONT_COOKIE, ubColor
	);
}

void buttonDrawAll(tBitMap *pBfr) {
	for(UBYTE i = 0; i < BUTTON_COUNT_MAX; ++i) {
		if(!s_pButtons[i].szName[0]) {
			break;
		}
		buttonDraw(i, pBfr);
	}
}

void buttonSelect(UBYTE ubIdx) {
	s_ubSelected = ubIdx;
}

UBYTE buttonGetSelected(void) {
	return s_ubSelected;
}
