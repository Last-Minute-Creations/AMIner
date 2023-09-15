#include "button.h"
#include <string.h>
#include <ace/utils/font.h>
#include <ace/utils/string.h>
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
static UWORD s_uwY;

void buttonReset(UWORD uwY) {
	s_uwY = uwY;

	for(UBYTE i = BUTTON_COUNT_MAX; i--;) {
		s_pButtons[i].szName[0] = '\0';
	}
	s_ubButtonCount = 0;
	s_ubSelected = BUTTON_INVALID;
	s_ePreset = BUTTON_PRESET_CUSTOM;
}

UBYTE buttonAdd(const char *szName) {
	if(s_ubButtonCount >= BUTTON_COUNT_MAX) {
		return BUTTON_INVALID;
	}

	UBYTE ubButtonIndex = s_ubButtonCount++;
	tButton *pButton = &s_pButtons[ubButtonIndex];
	strcpy(pButton->szName, szName);
	pButton->uwWidth = buttonGetWidth(ubButtonIndex);
	pButton->uwPosX = 0;
	s_ePreset = BUTTON_PRESET_CUSTOM;
	return ubButtonIndex;
}

void buttonDraw(UBYTE ubIdx, tBitMap *pBfr) {
	UBYTE ubColor = (
		ubIdx == s_ubSelected ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);
	const tButton *pButton = &s_pButtons[ubIdx];
	tUwCoordYX sSize = {.uwX = pButton->uwWidth, .uwY = buttonGetHeight()};
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	UWORD uwBtnX = pButton->uwPosX;
	UWORD uwBtnY = s_uwY - TEXT_PADDING_Y;
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
		pButton->szName, FONT_COOKIE, ubColor
	);
}

void buttonRowApply(void) {
	// Get free horizontal space
	UWORD uwFreeSpace = COMM_DISPLAY_WIDTH;
	for(UBYTE i = 0; i < s_ubButtonCount; ++i) {
		uwFreeSpace -= s_pButtons[i].uwWidth;
	}
	UBYTE ubPad = uwFreeSpace / (s_ubButtonCount + 1);

	// Spread buttons evenly
	UWORD uwOffsX = ubPad;
	for(UBYTE i = 0; i < s_ubButtonCount; ++i) {
		s_pButtons[i].uwPosX = uwOffsX;
		uwOffsX += s_pButtons[i].uwWidth + ubPad;
	}
}

void buttonDrawAll(tBitMap *pBfr) {
	for(UBYTE i = 0; i < s_ubButtonCount; ++i) {
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

UWORD buttonGetWidth(UBYTE ubIndex) {
	const tButton *pButton = &s_pButtons[ubIndex];
	UWORD uwWidth = fontMeasureText(g_pFont, pButton->szName).uwX + (BORDER_LR_DEPTH + TEXT_PADDING_X) * 2;
	return uwWidth;
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
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - buttonGetHeight();
	buttonReset(uwOffsY);
	buttonAdd(szTextAccept);
	buttonAdd(szTextDecline);
	s_ePreset = BUTTON_PRESET_ACCEPT_DECLINE;
	buttonSelect(0);
	buttonRowApply();
}

void buttonInitOk(const char *szText) {
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - buttonGetHeight();
	buttonReset(uwOffsY);
	buttonAdd(szText);
	s_ePreset = BUTTON_PRESET_OK;
	buttonSelect(0);
	buttonRowApply();
}
