#include "page_msg.h"
#include <ace/managers/system.h>
#include <comm/base.h>
#include <json/utf8.h>
#include "../defs.h"
#include "../core.h"

#define LINES_MAX 100
#define LINES_OCCUPIED_BY_FACE 3

static char *s_pLines[LINES_MAX] = {0};
static const char *s_szTitle;
static UWORD s_uwLineCount;
static UBYTE s_ubCurrPage, s_ubPageCount;
static tFaceId s_eFace;
static tOnClose s_cbOnClose;

static void freeLines(void) {
	while(s_uwLineCount--) {
		memFree(s_pLines[s_uwLineCount], strlen(s_pLines[s_uwLineCount]) + 1);
	}
	s_uwLineCount = 0;
	s_ubCurrPage = 0;
	s_ubPageCount = 0;
}

static void readLines(
	const tCodeRemap *pRemap, const char *szFilePath, UWORD uwMaxLineWidth
) {
	systemUse();
	logBlockBegin(
		"readLines(pRemap: %p, szFilePath: '%s', uwMaxLineWidth: %hu)",
		pRemap, szFilePath, uwMaxLineWidth
	);
	freeLines();

	// Read whole file to plain buffer
	tFile *pFileLines = fileOpen(szFilePath, "r");

	if(!pFileLines) {
		logWrite("ERR: Couldn't read lines from '%s'\n", szFilePath);
		logBlockEnd("readLines()");
		systemUnuse();
		return;
	}

	UWORD uwTextLength = 0;
	ULONG ulCodepoint, ulState = 0;
	UBYTE ubCharCode;
	UWORD uwFileContentsBufferSize = fileGetSize(szFilePath) + 1;
	char *szFileContents = memAllocFast(uwFileContentsBufferSize);

	while(fileRead(pFileLines, &ubCharCode, 1)) {
		if(decode(&ulState, &ulCodepoint, ubCharCode) != UTF8_ACCEPT) {
			continue;
		}

		if(pRemap) {
			ubCharCode = remapChar(pRemap, ulCodepoint);
		}
		else {
			ubCharCode = ulCodepoint;
		}

		szFileContents[uwTextLength++] = ubCharCode;
	}
	szFileContents[uwTextLength] = '\0';

	fileClose(pFileLines);

	// Split text into lines
	UWORD uwPos = 0;
	do {
		UBYTE ubLineLength = commBreakTextToWidth(
			&szFileContents[uwPos], uwMaxLineWidth
		);
		s_pLines[s_uwLineCount] = memAllocFast(ubLineLength + 1);
		memcpy(s_pLines[s_uwLineCount], &szFileContents[uwPos], ubLineLength);
		s_pLines[s_uwLineCount][ubLineLength] = '\0';

		if(s_pLines[s_uwLineCount][ubLineLength - 1] == '\n') {
			s_pLines[s_uwLineCount][ubLineLength - 1] = ' ';
		}

		++s_uwLineCount;
		uwPos += ubLineLength;
	} while(uwPos < uwTextLength);

	if(s_uwLineCount >= LINES_MAX) {
		logWrite("ERR: line count %hu >= LINES MAX %d", s_uwLineCount, LINES_MAX);
	}

	memFree(szFileContents, uwFileContentsBufferSize);
	UBYTE ubLinesPerPage = COMM_DISPLAY_HEIGHT / commGetLineHeight() - 1;
	WORD wLinesInNextPages = s_uwLineCount - LINES_OCCUPIED_BY_FACE;
	wLinesInNextPages = MAX(0, wLinesInNextPages);
	s_ubPageCount = 1 + ((UWORD)wLinesInNextPages + ubLinesPerPage - 1) / ubLinesPerPage;

	logBlockEnd("readLines()");
	systemUnuse();
}

static void commMsgDrawCurrentPage(void) {
	UBYTE ubLineHeight = commGetLineHeight();
	UBYTE ubLinesPerPage = COMM_DISPLAY_HEIGHT / ubLineHeight - 1;
	UWORD uwLineStart = MAX(0, s_ubCurrPage * ubLinesPerPage - 3);
	commEraseAll();
	UWORD uwLineY = 0;

	if(s_ubCurrPage == 0) {
		commDrawFaceAt(s_eFace, 0, 0);
		commDrawTitle(48, 0, g_pMsgs[MSG_PAGE_LIST_MIETEK + s_eFace]);
		commDrawText(48, ubLineHeight, s_szTitle, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
		uwLineY = LINES_OCCUPIED_BY_FACE * ubLineHeight;
		ubLinesPerPage -= LINES_OCCUPIED_BY_FACE;
	}

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
	char szPageCnt[8];
	sprintf(szPageCnt, "%hhu/%hhu", s_ubCurrPage + 1, s_ubPageCount);
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, COMM_DISPLAY_HEIGHT, szPageCnt,
		FONT_HCENTER | FONT_BOTTOM | FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK
	);
}

static void pageMsgProcess(void) {
	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		if(s_ubCurrPage < s_ubPageCount - 1) {
			++s_ubCurrPage;
			commMsgDrawCurrentPage();
		}
		else {
			if(s_cbOnClose) {
				s_cbOnClose();
			}

			return;
		}
	}

	if(s_ubCurrPage > 0 && commNavUse(DIRECTION_UP)) {
		--s_ubCurrPage;
		commMsgDrawCurrentPage();
	}
	else if(s_ubCurrPage < s_ubPageCount - 1 && commNavUse(DIRECTION_DOWN)) {
		++s_ubCurrPage;
		commMsgDrawCurrentPage();
	}
}

static void pageMsgDestroy(void) {
	systemUse();
	freeLines();
	systemUnuse();
}

void pageMsgCreate(
	tFaceId eFace, const char *szTitle, const char *szFile, tOnClose cbOnClose
) {
	logBlockBegin(
		"pageMsgCreate(eFace: %d, szTitle: '%s', szfile: '%s', cbOnClose: %p)",
		eFace, szTitle, szFile, cbOnClose
	);
	commRegisterPage(pageMsgProcess, pageMsgDestroy);
	s_eFace = eFace;
	s_szTitle = szTitle;
	s_cbOnClose = cbOnClose;
	s_uwLineCount = 0;
	char szPath[100];
	sprintf(szPath, "data/txt_%s/%s.txt", coreGetLangPrefix(), szFile);
	readLines(g_pRemap, szPath, COMM_DISPLAY_WIDTH);

	s_ubCurrPage = 0;
	commMsgDrawCurrentPage();
	logBlockEnd("pageMsgCreate()");
}