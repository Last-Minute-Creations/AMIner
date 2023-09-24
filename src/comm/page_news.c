#include "page_news.h"
#include <stdio.h>
#include <ace/managers/system.h>
#include <comm/comm.h>
#include <json/utf8.h>
#include "../defs.h"
#include "../assets.h"
#include "../language.h"
#include "menu.h"

#define SCROLL_SPEED_SLOW 1
#define SCROLL_SPEED_FAST 8
#define SCROLL_WIDTH_VISIBLE 160
#define SCROLL_WIDTH_BUFFER 176

static char *s_szScroll;
static UWORD s_uwScrollBufferLength; ///< Size of allocated buffer with UTF-8 chars
static tTextBitMap *s_pNewsTextBitmap;
static const char *s_pCurrentChar;
static WORD s_wDrawnTextEnd;
static WORD s_wClearedBgEnd;
static UBYTE s_isScrollDone;

static const char *s_pNewsFileNames[NEWS_KIND_COUNT] = {
	[NEWS_KIND_ACCOLADES] = "accolades",
	[NEWS_KIND_INTRO_1] = "intro_1",
	[NEWS_KIND_INTRO_2] = "intro_2",
	[NEWS_KIND_INTRO_3] = "intro_3",
};

/**
 * @brief Draw characters to fill the void on the right.
 */
static void pageNewsFillScrollWithText(void) {
	if(s_wClearedBgEnd > 0 && s_wClearedBgEnd < SCROLL_WIDTH_BUFFER) {
		blitRect(
			s_pNewsTextBitmap->pBitMap, s_wClearedBgEnd, 0,
			SCROLL_WIDTH_BUFFER - s_wClearedBgEnd, g_pFont->uwHeight, 0
		);
		s_wClearedBgEnd = SCROLL_WIDTH_BUFFER;
	}

	char szCurrentChar[2] = {0};
	while(*s_pCurrentChar) {
		UBYTE ubDrawWidth = fontGlyphWidth(g_pFont, *s_pCurrentChar) + 1;
		if(s_wDrawnTextEnd + ubDrawWidth >= SCROLL_WIDTH_BUFFER) {
			break;
		}

		szCurrentChar[0] = *s_pCurrentChar;
		++s_pCurrentChar;
		fontDrawStr1bpp(
			g_pFont, s_pNewsTextBitmap->pBitMap, s_wDrawnTextEnd, 0, szCurrentChar
		);
		s_wDrawnTextEnd += ubDrawWidth;
	}
}

static void pageNewsProcess(void) {
	UBYTE ubSpeed = SCROLL_SPEED_SLOW; //commNavCheck(DIRECTION_FIRE) ? SCROLL_SPEED_FAST : SCROLL_SPEED_SLOW;

	// Shift scroll contents right
	tBitMap *pBitmap = s_pNewsTextBitmap->pBitMap;
	UBYTE *pStart = &pBitmap->Planes[0][pBitmap->BytesPerRow * pBitmap->Rows - sizeof(UWORD)];
	UWORD uwBlitSize = (g_pFont->uwHeight << HSIZEBITS) | (SCROLL_WIDTH_BUFFER / 16);
	UWORD uwBltCon0 = (ubSpeed << ASHIFTSHIFT) | USEA|USED | MINTERM_A;
	blitWait();
	g_pCustom->bltcon0 = uwBltCon0;
	g_pCustom->bltcon1 = BC1F_DESC;
	g_pCustom->bltafwm = 0xFFFF;
	g_pCustom->bltalwm = 0xFFFF;
	g_pCustom->bltamod = 0;
	g_pCustom->bltdmod = 0;
	g_pCustom->bltapt = pStart;
	g_pCustom->bltdpt = pStart;
	g_pCustom->bltsize = uwBlitSize;

	if(s_wDrawnTextEnd > 0) {
		s_wDrawnTextEnd -= ubSpeed;
		s_wClearedBgEnd -= ubSpeed;
		pageNewsFillScrollWithText();

		blitRect(
			commGetDisplayBuffer(),
			commGetOriginDisplay().uwX + 2,
			commGetOriginDisplay().uwY + COMM_DISPLAY_HEIGHT - g_pFont->uwHeight - 4,
			SCROLL_WIDTH_VISIBLE,
			g_pFont->uwHeight, 10
		);
		fontDrawTextBitMap(
			commGetDisplayBuffer(), s_pNewsTextBitmap,
			commGetOriginDisplay().uwX + 2,
			commGetOriginDisplay().uwY + COMM_DISPLAY_HEIGHT - g_pFont->uwHeight - 4,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);
	}
	else {
		s_wDrawnTextEnd = 0;
		s_isScrollDone = 1;
		if(!commIsIntro()) {
			commRegisterPage(0, 0);
			menuGsEnter(0);
		}
	}
}

UBYTE pageNewsIsDone(void) {
	return s_isScrollDone;
}

void pageNewsDestroy(void) {
	fontDestroyTextBitMap(s_pNewsTextBitmap);
	memFree(s_szScroll, s_uwScrollBufferLength);
}

void pageNewsCreate(tNewsKind eNewsKind) {
	logBlockBegin("pageNewsCreate(eNewsKind: %d)", eNewsKind);
	commRegisterPage(pageNewsProcess, pageNewsDestroy);

	char szPath[50];
	sprintf(szPath, "data/txt_%s/news_%s.txt", languageGetPrefix(), s_pNewsFileNames[eNewsKind]);
	s_szScroll = remapFile(szPath, g_pRemap, &s_uwScrollBufferLength, 0);
	s_isScrollDone = 0;

	tUwCoordYX sOrigin = commGetOrigin();
	tBitMap *pBitmapNews = bitmapCreateFromFile("data/comm_news.bm", 0);
	blitCopy(
		pBitmapNews, 0, 0, commGetDisplayBuffer(), sOrigin.uwX + 25, sOrigin.uwY + 28,
		bitmapGetByteWidth(pBitmapNews) * 8, pBitmapNews->Rows, MINTERM_COOKIE
	);
	bitmapDestroy(pBitmapNews);

	s_pNewsTextBitmap = fontCreateTextBitMap(SCROLL_WIDTH_BUFFER, g_pFont->uwHeight);
	s_pNewsTextBitmap->uwActualWidth = SCROLL_WIDTH_VISIBLE;
	s_pNewsTextBitmap->uwActualHeight = g_pFont->uwHeight;
	s_pCurrentChar = s_szScroll;
	s_wDrawnTextEnd = SCROLL_WIDTH_VISIBLE;
	s_wClearedBgEnd = SCROLL_WIDTH_BUFFER;
	logBlockEnd("pageNewsCreate()");
}
