/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base.h"
#include <ace/managers/audio.h>
#include <ace/managers/rand.h>
#include <ace/managers/system.h>
#include "../core.h"
#include "../steer.h"
#include "../game.h"

static tBitMap *s_pBmRestore;
static tBitMap *s_pBg, *s_pButtons;
static UBYTE s_pNav[COMM_NAV_COUNT] = {BTN_STATE_NACTIVE};
static UBYTE s_pNavEx[COMM_NAV_EX_COUNT] = {BTN_STATE_NACTIVE};
static tBitMap *s_pBmDraw;
static tTextBitMap *s_pLineBuffer;
static UBYTE s_isShown = 0;
static tSample *s_pSamplesKeyPress[4];
static tSample *s_pSamplesKeyRelease[4];
tBitMap *g_pCommBmFaces, *g_pCommBmSelection;

void commCreate(void) {
	systemUse();
	s_pBmRestore = bitmapCreate(
		COMM_WIDTH, COMM_HEIGHT,
		g_pMainBuffer->sCommon.pVPort->ubBPP, BMF_INTERLEAVED
	);
	s_pBg = bitmapCreateFromFile("data/comm_bg.bm", 0);
	s_pButtons = bitmapCreateFromFile("data/comm_buttons.bm", 0);
	s_pLineBuffer = fontCreateTextBitMap(
		ROUND_TO_MULTIPLE(COMM_DISPLAY_WIDTH, 16), g_pFont->uwHeight
	);

	for(UBYTE i = 0; i < 4; ++i) {
		char szPath[40];
		sprintf(szPath, "data/sfx/key_press_%hhu.raw8", i);
		s_pSamplesKeyPress[i] = sampleCreateFromFile(szPath, 48000);
		sprintf(szPath, "data/sfx/key_release_%hhu.raw8", i);
		s_pSamplesKeyRelease[i] = sampleCreateFromFile(szPath, 48000);
	}
	systemUnuse();

	g_pCommBmFaces = bitmapCreateFromFile("data/comm_faces_office.bm", 0);
	g_pCommBmSelection = bitmapCreateFromFile("data/comm_office_selection.bm", 0);

	s_isShown = 0;
}

void commDestroy(void) {
	systemUse();
	for(UBYTE i = 0; i < 4; ++i) {
		sampleDestroy(s_pSamplesKeyPress[i]);
		sampleDestroy(s_pSamplesKeyRelease[i]);
	}

	bitmapDestroy(s_pBmRestore);
	bitmapDestroy(s_pBg);
	bitmapDestroy(s_pButtons);
	fontDestroyTextBitMap(s_pLineBuffer);

	bitmapDestroy(g_pCommBmFaces);
	bitmapDestroy(g_pCommBmSelection);

	systemUnuse();
}

void commSetActiveLed(tCommLed eLed) {
	const UBYTE ubLedWidth = 16;
	const UBYTE ubLedHeight = 13;
	const UBYTE ubGrnLedY = ubLedHeight;
	static const UBYTE pLedX[] = {19, 95, 181};
	const UBYTE ubLedY = 169;

	tUwCoordYX sOrigin = commGetOrigin();
	for(UBYTE i = 0; i < COMM_LED_COUNT; ++i) {
		blitCopy(
			s_pButtons, 0, (i == eLed ? ubGrnLedY : 0),
			s_pBmDraw, sOrigin.uwX + pLedX[i], sOrigin.uwY + ubLedY,
			ubLedWidth, ubLedHeight, MINTERM_COOKIE
		);
	}
}

UBYTE commTryShow(void) {
	tUwCoordYX sOrigin = commGetOrigin();
	if(g_pMainBuffer->uwMarginedHeight - sOrigin.uwX < COMM_HEIGHT) {
		// Not positioned evenly
		return 0;
	}
	s_isShown = 1;

	s_pBmDraw = g_pMainBuffer->pScroll->pBack;

	// Store content beneath commrade
	blitCopyAligned(
		s_pBmDraw, sOrigin.uwX, sOrigin.uwY,
		s_pBmRestore, 0, 0, COMM_WIDTH, COMM_HEIGHT
	);

	// Draw commrade background
	blitCopyAligned(
		s_pBg, 0, 0, s_pBmDraw, sOrigin.uwX, sOrigin.uwY,
		COMM_WIDTH, COMM_HEIGHT
	);

	// Skip the initial fire press
	s_pNav[COMM_NAV_BTN] = BTN_STATE_USED;

	return 1;
}

void commProcess(void) {
	static const UWORD pBtnPos[COMM_NAV_COUNT][4] = {
		// dX, dY, sY, h
		{218, 114, 26, 14},
		{218, 143, 54, 14},
		{203, 128, 82, 15},
		{234, 128, 112, 15},
		{218, 129, 142, 13}
	};

	steerUpdateFromInput(g_is1pKbd, g_is2pKbd);
	UBYTE pTests[COMM_NAV_COUNT] = {
		steerGet(STEER_P1_UP)  || steerGet(STEER_P2_UP),
		steerGet(STEER_P1_DOWN) || steerGet(STEER_P2_DOWN),
		steerGet(STEER_P1_LEFT) || steerGet(STEER_P2_LEFT),
		steerGet(STEER_P1_RIGHT) || steerGet(STEER_P2_RIGHT),
		steerGet(STEER_P1_FIRE) || steerGet(STEER_P2_FIRE),
	};

	tUwCoordYX sOrigin = commGetOrigin();

	for(UBYTE i = 0; i < COMM_NAV_COUNT; ++i) {
		if(pTests[i]) {
			if(s_pNav[i] == BTN_STATE_NACTIVE) {
				// audioPlay(AUDIO_CHANNEL_0, s_pSamplesKeyPress[uwRand() & 3], AUDIO_VOLUME_MAX, 1);
				s_pNav[i] = BTN_STATE_ACTIVE;
				blitCopy(
					s_pButtons, 0, pBtnPos[i][2], s_pBmDraw,
					sOrigin.uwX + pBtnPos[i][0],
					sOrigin.uwY + pBtnPos[i][1],
					16, pBtnPos[i][3], MINTERM_COOKIE
				);
			}
		}
		else if(s_pNav[i] != BTN_STATE_NACTIVE) {
			// audioPlay(AUDIO_CHANNEL_0, s_pSamplesKeyRelease[uwRand() & 3], AUDIO_VOLUME_MAX, 1);
			s_pNav[i] = BTN_STATE_NACTIVE;
			blitCopy(
				s_pButtons, 0, pBtnPos[i][2] + pBtnPos[i][3], s_pBmDraw,
				sOrigin.uwX + pBtnPos[i][0],
				sOrigin.uwY + pBtnPos[i][1],
				16, pBtnPos[i][3], MINTERM_COOKIE
			);
		}
	}

	// Process ex events
	static UBYTE isShift = 0;
	static UBYTE wasShiftAction = 0;
	if(commNavCheck(COMM_NAV_BTN) == BTN_STATE_ACTIVE) {
		isShift = 1;
	}
	else {
		s_pNavEx[COMM_NAV_EX_BTN_CLICK] = BTN_STATE_NACTIVE;
		if(isShift) {
			if(!wasShiftAction) {
				// Btn released and no other pressed in the meantime
				s_pNavEx[COMM_NAV_EX_BTN_CLICK] = BTN_STATE_ACTIVE;
			}
			isShift = 0;
			wasShiftAction = 0;
		}
	}

	// Tab nav using shift+left / shift+right
	if(isShift) {
		if(commNavUse(COMM_NAV_LEFT)) {
			s_pNavEx[COMM_NAV_EX_SHIFT_LEFT] = BTN_STATE_ACTIVE;
			wasShiftAction = 1;
		}
		else if(commNavUse(COMM_NAV_RIGHT)) {
			s_pNavEx[COMM_NAV_EX_SHIFT_RIGHT] = BTN_STATE_ACTIVE;
			wasShiftAction = 1;
		}
	}
	else {
		s_pNavEx[COMM_NAV_EX_SHIFT_LEFT] = BTN_STATE_NACTIVE;
		s_pNavEx[COMM_NAV_EX_SHIFT_RIGHT] = BTN_STATE_NACTIVE;
	}
}

tBtnState commNavCheck(tCommNav eNav) {
	return s_pNav[eNav];
}

UBYTE commNavUse(tCommNav eNav) {
	// Relying on btn states from commProcess() makes it independent from state
	// changes during interrupts and allows hierarchical usage of buttons
	if(s_pNav[eNav] == BTN_STATE_ACTIVE) {
		s_pNav[eNav] = BTN_STATE_USED;
		return 1;
	}
	return 0;
}

UBYTE commNavExUse(tCommNavEx eNavEx) {
	if(s_pNavEx[eNavEx] == BTN_STATE_ACTIVE) {
		s_pNavEx[eNavEx] = BTN_STATE_USED;
		return 1;
	}
	return 0;
}

void commHide(void) {
	if(!s_isShown) {
		return;
	}
	s_isShown = 0;
	tUwCoordYX sOrigin = commGetOrigin();
	// Restore content beneath commrade
	blitCopyAligned(
		s_pBmRestore, 0, 0, s_pBmDraw, sOrigin.uwX, sOrigin.uwY,
		COMM_WIDTH, COMM_HEIGHT
	);
}

tBitMap *commGetDisplayBuffer(void) {
	return s_pBmDraw;
}

tUwCoordYX commGetOrigin(void) {
	UWORD uwScrollX = g_pMainBuffer->pCamera->uPos.uwX;
	UWORD uwScrollY = g_pMainBuffer->pCamera->uPos.uwY;
	UWORD uwBufferHeight = g_pMainBuffer->uwMarginedHeight;
	UWORD uwFoldScrollY = uwScrollY & (uwBufferHeight-1);
	UWORD uwCommOffsY = (g_pMainBuffer->sCommon.pVPort->uwHeight - COMM_HEIGHT) / 2;
	UWORD uwCommOffsX = (g_pMainBuffer->sCommon.pVPort->uwWidth - COMM_WIDTH) / 2;

	tUwCoordYX sOrigin;
	sOrigin.uwX = uwScrollX + uwCommOffsX;
	sOrigin.uwY = uwFoldScrollY + uwCommOffsY;

	return sOrigin;
}

tUwCoordYX commGetOriginDisplay(void) {
	tUwCoordYX sOrigin = commGetOrigin();
	sOrigin.uwX += COMM_DISPLAY_X;
	sOrigin.uwY += COMM_DISPLAY_Y;
	return sOrigin;
}


void commDrawText(
	UWORD uwX, UWORD uwY, const char *szText, UBYTE ubFontFlags, UBYTE ubColor
) {
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	fontDrawStr(
		g_pFont, s_pBmDraw, sOrigin.uwX + uwX, sOrigin.uwY + uwY,
		szText, ubColor, ubFontFlags, s_pLineBuffer
	);
}

UBYTE commDrawMultilineText(
	const char *szText, UWORD uwStartX, UWORD uwStartY
) {
	UWORD uwTextOffs = 0;
	UBYTE ubLinesWritten = 0;
	UWORD uwCurrY = uwStartY;
	UBYTE ubLineHeight = commGetLineHeight();
	char szLineBfr[50];
	do {
		// Measure chars in line
		UBYTE ubCharsInLine = commBreakTextToWidth(
			&szText[uwTextOffs], COMM_DISPLAY_WIDTH - uwStartX
		);

		// Copy to line buffer and draw
		memcpy(szLineBfr, &szText[uwTextOffs], ubCharsInLine);
		szLineBfr[ubCharsInLine] = '\0';
		if(szLineBfr[ubCharsInLine - 1] == '\n') {
			szLineBfr[ubCharsInLine - 1] = ' ';
		}
		commDrawText(uwStartX, uwCurrY, szLineBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);

		// Advance
		uwTextOffs += ubCharsInLine;
		++ubLinesWritten;
		uwCurrY += ubLineHeight;
	} while(szText[uwTextOffs] != '\0');

	return ubLinesWritten;
}

UBYTE commGetLineHeight(void) {
	return g_pFont->uwHeight + 1;
}

void commErase(UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight) {
#if defined(COMM_DEBUG)
	static UBYTE ubDebugColor = 0;
	const UBYTE ubClearColor = ubDebugColor;
	ubDebugColor = (ubDebugColor + 1) & 31;
#else
	const UBYTE ubClearColor = COMM_DISPLAY_COLOR_BG;
#endif
	const tUwCoordYX sPosDisplay = commGetOriginDisplay();
	blitRect(
		s_pBmDraw, sPosDisplay.uwX + uwX, sPosDisplay.uwY + uwY,
		uwWidth, uwHeight, ubClearColor
	);
}

void commEraseAll(void) {
	commErase(0, 0, COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT);
}

void commProgress(UBYTE ubPercent, const char *szDescription) {
	logWrite("Comm Progress: %hhu\n", ubPercent);
	if(!s_isShown) {
		return;
	}

	commErase(
		0, COMM_DISPLAY_HEIGHT / 2 - g_pFont->uwHeight,
		COMM_DISPLAY_WIDTH, g_pFont->uwHeight * 3
	);
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, COMM_DISPLAY_HEIGHT / 2, szDescription,
		FONT_HCENTER | FONT_BOTTOM | FONT_COOKIE | FONT_LAZY,
		COMM_DISPLAY_COLOR_TEXT
	);

	tUwCoordYX sBarPos = commGetOriginDisplay();
	const UBYTE ubDist = 2;
	const UBYTE ubBarWidth = 100;
	const UBYTE ubBarHeight = g_pFont->uwHeight;
	sBarPos.uwX += (COMM_DISPLAY_WIDTH - 100) / 2;
	sBarPos.uwY += COMM_DISPLAY_HEIGHT / 2 + 1 + ubDist;

	// X lines
	blitRect(
		s_pBmDraw, sBarPos.uwX - ubDist, sBarPos.uwY - ubDist,
		ubDist + ubBarWidth + ubDist, 1, COMM_DISPLAY_COLOR_TEXT
	);
	blitRect(
		s_pBmDraw, sBarPos.uwX - ubDist, sBarPos.uwY + ubBarHeight + ubDist - 1,
		ubDist + ubBarWidth + ubDist, 1, COMM_DISPLAY_COLOR_TEXT
	);

	// Y lines
	blitRect(
		s_pBmDraw, sBarPos.uwX - ubDist, sBarPos.uwY - ubDist,
		1, ubDist + ubBarHeight + ubDist, COMM_DISPLAY_COLOR_TEXT
	);
	blitRect(
		s_pBmDraw, sBarPos.uwX + ubBarWidth + ubDist - 1, sBarPos.uwY - ubDist,
		1, ubDist + ubBarHeight + ubDist, COMM_DISPLAY_COLOR_TEXT
	);

	// Fill
	blitRect(
		s_pBmDraw, sBarPos.uwX, sBarPos.uwY,
		ubBarWidth * ubPercent / 100, ubBarHeight, COMM_DISPLAY_COLOR_TEXT_DARK
	);

}

UBYTE commBreakTextToWidth(const char *szInput, UWORD uwMaxLineWidth) {
	UWORD uwLineWidth = 0;
	UBYTE ubCharsInLine = 0;
	UBYTE ubLastSpace = 0xFF; // next char, actually

	while(*szInput != '\0') {
		UBYTE ubCharWidth = fontGlyphWidth(g_pFont, *szInput) + 1;

		if(uwLineWidth + ubCharWidth >= uwMaxLineWidth) {
			if(ubLastSpace != 0xFF) {
				ubCharsInLine = ubLastSpace;
			}

			break;
		}

		uwLineWidth += ubCharWidth;
		ubCharsInLine++;

		if(*szInput == ' ') {
			ubLastSpace = ubCharsInLine;
		}
		else if(*szInput == '\r') {
			logWrite("ERR: CR character detected - use LF line endings!");
			break;
		}
		else if(*szInput == '\n') {
			break;
		}

		++szInput;
	}
	return ubCharsInLine;
}

tPageProcess s_pPageProcess = 0;
tPageCleanup s_pPageCleanup = 0;

void commRegisterPage(tPageProcess cbProcess, tPageCleanup cbCleanup) {
	logBlockBegin(
		"commRegisterPage(cbProcess: %p, cbCleanup: %p)", cbProcess, cbCleanup
	);
	if(s_pPageCleanup) {
		s_pPageCleanup();
	}
	commEraseAll();

	s_pPageProcess = cbProcess;
	s_pPageCleanup = cbCleanup;
	logBlockEnd("commRegisterPage()");
}

UBYTE commProcessPage(void) {
	if(s_pPageProcess) {
		s_pPageProcess();
		return 1;
	}
	return 0;
}
