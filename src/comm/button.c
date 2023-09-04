#include "button.h"
#include <string.h>
#include <ace/utils/font.h>
#include <comm/comm.h>
#include "core.h"

#define BUTTON_COUNT_MAX 5

// Chars used in buttons don't take up most of the vertical font space
#define BORDER_LR_DEPTH 2
#define TEXT_PADDING_X 1
#define TEXT_PADDING_Y 1
#define TEXT_HEIGHT_DECREMENT 3
#define TEXT_BASELINE_POS 8
#define TEXT_CAP_POS 3
#define TEXT_HEIGHT (TEXT_BASELINE_POS - TEXT_CAP_POS)

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
	sSize.uwX += (BORDER_LR_DEPTH + TEXT_PADDING_X) * 2;
	sSize.uwY = buttonGetHeight();
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	UWORD uwBtnX = s_pButtons[ubIdx].sPos.uwX - sSize.uwX / 2;
	UWORD uwBtnY = s_pButtons[ubIdx].sPos.uwY - TEXT_PADDING_Y;
	blitRect( // left line
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY, 1, sSize.uwY, ubColor
	);
	blitRect( // top-left line
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY, BORDER_LR_DEPTH, 1, ubColor
	);
	blitRect( // bottom-left line
		pBfr, sOrigin.uwX + uwBtnX, sOrigin.uwY + uwBtnY + sSize.uwY - 1,
		BORDER_LR_DEPTH, 1, ubColor
	);
	blitRect( // right line
		pBfr, sOrigin.uwX + uwBtnX + sSize.uwX - 1, sOrigin.uwY + uwBtnY,
		1, sSize.uwY, ubColor
	);
	blitRect( // top-right line
		pBfr, sOrigin.uwX + uwBtnX + sSize.uwX - BORDER_LR_DEPTH,
		sOrigin.uwY + uwBtnY,
		BORDER_LR_DEPTH, 1, ubColor
	);
	blitRect( // bottom-right line
		pBfr, sOrigin.uwX + uwBtnX + sSize.uwX - BORDER_LR_DEPTH,
		sOrigin.uwY + uwBtnY + sSize.uwY - 1,
		BORDER_LR_DEPTH, 1, ubColor
	);
	commDrawText(
		uwBtnX + 3, uwBtnY - TEXT_CAP_POS + TEXT_PADDING_Y,
		s_pButtons[ubIdx].szName, FONT_COOKIE, ubColor
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
	return TEXT_HEIGHT + 2 * TEXT_PADDING_Y;
}

tButtonPreset buttonGetPreset(void) {
	return s_ePreset;
}

void buttonInitAcceptDecline(
	const char *szTextAccept, const char *szTextDecline
) {
	buttonRmAll();
	UBYTE ubButtonHeight = buttonGetHeight();
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - ubButtonHeight;
	UWORD uwOffsX = COMM_DISPLAY_WIDTH / 3;
	buttonAdd(szTextAccept, uwOffsX, uwOffsY);

	uwOffsX *= 2;
	buttonAdd(szTextDecline, uwOffsX, uwOffsY);
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
