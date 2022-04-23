#include "button.h"
#include <string.h>
#include <ace/utils/font.h>
#include <comm/base.h>
#include "core.h"

#define BUTTON_COUNT_MAX 5

#define BUTTON_BORDER_WIDTH 1
#define BUTTON_PADDING 2

static tButton s_pButtons[BUTTON_COUNT_MAX];
static UBYTE s_ubButtonCount;
static UBYTE s_ubSelected = BUTTON_INVALID;
static tButtonPreset s_ePreset;

void buttonRmAll(void) {
	for(UBYTE i = BUTTON_COUNT_MAX; i--;) {
		s_pButtons[i].szName[0] = '\0';
	}
	s_ubButtonCount = 0;
	s_ubSelected = BUTTON_INVALID;
	s_ePreset = BUTTON_PRESET_CUSTOM;
}

UBYTE buttonAdd(const char *szName, UWORD uwX, UWORD uwY) {
	for(UBYTE i = 0; i < BUTTON_COUNT_MAX; ++i) {
		if(s_pButtons[i].szName[0] == '\0') {
			strcpy(s_pButtons[i].szName, szName);
			s_pButtons[i].sPos.uwX = uwX;
			s_pButtons[i].sPos.uwY = uwY;
			s_ePreset = BUTTON_PRESET_CUSTOM;
			++s_ubButtonCount;
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
	sSize.uwY += 2 * (BUTTON_BORDER_WIDTH + BUTTON_PADDING);
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	UWORD uwBtnX = s_pButtons[ubIdx].sPos.uwX - sSize.uwX / 2;
	UWORD uwBtnY = s_pButtons[ubIdx].sPos.uwY;
	blitRect( // top line
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY, sSize.uwX, 1, ubColor
	);
	blitRect( // left line
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY, 1, sSize.uwY, ubColor
	);
	blitRect( // bottom line
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY + sSize.uwY - 1,
		sSize.uwX, 1, ubColor
	);
	blitRect( // right line
		pBfr, sOrigin.uwX + uwBtnX + sSize.uwX - 1, sOrigin.uwY + uwBtnY,
		1, sSize.uwY, ubColor
	);
	commDrawText(
		uwBtnX + 3, uwBtnY  + 2, s_pButtons[ubIdx].szName, FONT_COOKIE, ubColor
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

UBYTE buttonSelect(UBYTE ubIdx) {
	if(ubIdx >= s_ubButtonCount) {
		return 0;
	}
	s_ubSelected = ubIdx;
	return 1;
}

UBYTE buttonGetCount(void) {
	return s_ubButtonCount;
}

UBYTE buttonGetSelected(void) {
	return s_ubSelected;
}

UBYTE buttonGetHeight(void) {
	return g_pFont->uwHeight + 2 * (BUTTON_BORDER_WIDTH + BUTTON_PADDING);
}

tButtonPreset buttonGetPreset(void) {
	return s_ePreset;
}

void buttonInitAcceptDecline(
	const char *szTextAccept, const char *szTextDecline
) {
	buttonRmAll();
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - (3 * buttonGetHeight()) / 2;
	buttonAdd(szTextAccept, COMM_DISPLAY_WIDTH / 2, uwOffsY);

	uwOffsY += buttonGetHeight();
	buttonAdd(szTextDecline, COMM_DISPLAY_WIDTH / 2, uwOffsY);
	s_ePreset = BUTTON_PRESET_ACCEPT_DECLINE;
	buttonSelect(0);
}

void buttonInitOk(const char *szText) {
	buttonRmAll();
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - buttonGetHeight();
	buttonAdd(szText, COMM_DISPLAY_WIDTH / 2, uwOffsY);
	s_ePreset = BUTTON_PRESET_OK;
	buttonSelect(0);
}
