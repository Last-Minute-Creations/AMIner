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
static tTextBitMap *s_pTextBitmap;

static char *s_pLines[LINES_MAX];
UWORD s_uwLineCount;
UBYTE s_ubCurrPage;
UBYTE s_ubPageCount;

static tUwCoordYX s_sOrigin;

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
		UWORD uwWordPxLength = fontMeasureText(g_pFont, szWordBfr).sUwCoord.uwX;
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

static void commMsgDrawPage(
	UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight, UBYTE ubPage
) {
	UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	UBYTE ubLinesPerPage = uwHeight / ubLineHeight;
	UWORD uwLineStart = ubPage * ubLinesPerPage;
	blitRect(
		s_pBuffer, s_sOrigin.sUwCoord.uwX + uwX, s_sOrigin.sUwCoord.uwY + uwY,
		uwWidth, uwHeight, COMM_DISPLAY_COLOR_BG
	);
	for(
		UWORD i = uwLineStart;
		i < uwLineStart + ubLinesPerPage && i < s_uwLineCount; ++i
	) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, s_pLines[i]);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap, s_sOrigin.sUwCoord.uwX + uwX,
			s_sOrigin.sUwCoord.uwY + uwY, COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_SHADOW
		);
		uwY += ubLineHeight;
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
	s_pTextBitmap = fontCreateTextBitMap(
		COMM_WIDTH, g_pFont->uwHeight
	);
	s_sOrigin = commGetOrigin();

	commMsgDrawPage(COMM_DISPLAY_X, COMM_DISPLAY_Y, COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT, 0);

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
		commMsgDrawPage(
			COMM_DISPLAY_X, COMM_DISPLAY_Y, COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT, s_ubCurrPage
		);
	}
	else if(s_ubCurrPage < s_ubPageCount - 1 && commNavUse(COMM_NAV_DOWN)) {
		++s_ubCurrPage;
		commMsgDrawPage(
			COMM_DISPLAY_X, COMM_DISPLAY_Y, COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT, s_ubCurrPage
		);
	}
}

void commMsgGsDestroy(void) {
	if(!s_isShown) {
		return;
	}

	systemUse();
	freeLines();
	fontDestroyTextBitMap(s_pTextBitmap);
	systemUnuse();
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}
