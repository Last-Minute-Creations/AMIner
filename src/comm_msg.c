/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comm_msg.h"
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/utils/font.h>
#include "game.h"
#include "color.h"
#include "comm.h"

#define LINES_MAX 100

UBYTE s_isShown;
tBitMap *s_pBuffer;

static char *s_pLines[LINES_MAX];
UWORD s_uwLineCount;
UBYTE s_ubCurrPage;
UBYTE s_ubPageCount;

static void freeLines(void) {
	while(s_uwLineCount--) {
		memFree(s_pLines[s_uwLineCount], strlen(s_pLines[s_uwLineCount]) + 1);
	}
	s_uwLineCount = 0;
	s_ubCurrPage = 0;
	s_ubPageCount = 0;
}

static void readLines(const char *szFilePath, UWORD uwMaxLength) {
	systemUse();
	freeLines();
	tFile *pFileLines = fileOpen(szFilePath, "r");
	if(!pFileLines) {
		logWrite("ERR: Couldn't read lines from '%s'\n", szFilePath);
		return;
	}
	char szWordBfr[50];
	UBYTE ubWordBytes = 0;
	char szLineBfr[200] = "";
	UWORD uwLinePxLength = 0;
	UBYTE ubSpacePxLength = fontGlyphWidth(g_pFont, ' ') + 1;

	while(!fileIsEof(pFileLines)) {
		// Read one word and measure it when complete
		char c;
		fileRead(pFileLines, &c, 1);
		if(c > ' ') {
			szWordBfr[ubWordBytes] = c;
			++ubWordBytes;
			continue;
		}

		// It it's end of line start new line
		szWordBfr[ubWordBytes] = '\0';
		UWORD uwWordPxLength = fontMeasureText(g_pFont, szWordBfr).uwX;
		if(uwLinePxLength + ubSpacePxLength + uwWordPxLength > uwMaxLength) {
			// If it's too long, save current line and write that word at the beginning of new line
			s_pLines[s_uwLineCount] = memAllocFast(strlen(szLineBfr) + 1);
			strcpy(s_pLines[s_uwLineCount], szLineBfr);
			++s_uwLineCount;
			strcpy(szLineBfr, szWordBfr);
			uwLinePxLength = uwWordPxLength;
		}
		else {
			// If it's not, append to current line
			if(uwLinePxLength) {
				// Add space before word if there's already something in line
				strcat(szLineBfr, " ");
			}
			strcat(szLineBfr, szWordBfr);
			uwLinePxLength += ubSpacePxLength + uwWordPxLength;
		}
		ubWordBytes = 0;
		if(c == '\n') {
			// If it's end of line, save current line and start new one
			s_pLines[s_uwLineCount] = memAllocFast(strlen(szLineBfr) + 1);
			strcpy(s_pLines[s_uwLineCount], szLineBfr);
			++s_uwLineCount;
			szLineBfr[0] = '\0';
			uwLinePxLength = 0;
		}
	}
	fileClose(pFileLines);

	if(s_uwLineCount >= LINES_MAX) {
		logWrite("ERR: line count %hu >= LINES MAX %d", s_uwLineCount, LINES_MAX);
	}

	systemUnuse();

	UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	UBYTE ubLinesPerPage = COMM_DISPLAY_HEIGHT / ubLineHeight;
	s_ubPageCount = (s_uwLineCount + (ubLinesPerPage - 1)) / ubLinesPerPage;
}

static void commMsgDrawPage(UBYTE ubPage) {
	UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	UBYTE ubLinesPerPage = COMM_DISPLAY_HEIGHT / ubLineHeight;
	UWORD uwLineStart = ubPage * ubLinesPerPage;
	commEraseAll();
	UWORD uwLineY = 0;
	for(
		UWORD i = uwLineStart;
		i < uwLineStart + ubLinesPerPage && i < s_uwLineCount; ++i
	) {
		commDrawText(
			0, uwLineY, s_pLines[i], FONT_COOKIE | FONT_SHADOW,
			COMM_DISPLAY_COLOR_TEXT
		);
		uwLineY += ubLineHeight;
	}
}

void commMsgGsCreate(void) {
	s_isShown = commShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_uwLineCount = 0;
	readLines("data/intro.txt", COMM_DISPLAY_WIDTH);

	s_pBuffer = g_pMainBuffer->pScroll->pBack;

	commMsgDrawPage(0);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now on!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void commMsgGsLoop(void) {

	commProcess();

	if(commNavUse(COMM_NAV_BTN)) {
		blitWait();
		gamePopState();
		return;
	}

	if(s_ubCurrPage > 0 && commNavUse(COMM_NAV_UP)) {
		--s_ubCurrPage;
		commMsgDrawPage(s_ubCurrPage);
	}
	else if(s_ubCurrPage < s_ubPageCount - 1 && commNavUse(COMM_NAV_DOWN)) {
		++s_ubCurrPage;
		commMsgDrawPage(s_ubCurrPage);
	}
}

void commMsgGsDestroy(void) {
	if(!s_isShown) {
		return;
	}

	systemUse();
	freeLines();
	systemUnuse();
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}
