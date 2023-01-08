#include "page_news.h"
#include <ace/managers/system.h>
#include <comm/base.h>
#include <json/utf8.h>
#include "../defs.h"
#include "../core.h"
#include "menu.h"

#define SCROLL_SPEED_SLOW 1
#define SCROLL_SPEED_FAST 8
#define SCROLL_WIDTH_VISIBLE 160
#define SCROLL_WIDTH_BUFFER 176

static const char *s_szScroll = "Dzisiaj w godzinach popoludniowych w siedzibie Ministerstwa na uroczystej Gali zostaly wreczone odznaczenia, awanse oraz nominacje na nowe stanowiska w Resorcie. Dyrektorzy kopaln otrzymali odznaki: Zasluzony Przodownik Pracy Socjalistycznej, Zasluzony dla Gornictwa LRD. Wszyscy rowniez otrzymali nowe przydzialy na odcinkach wymagajacych jeszcze wiekszego poswiecenia i zaangazowania. Minister dziekowal zebranym za ofiarna prace i trud w realizacji postawionego celu przekroczenia normy wydobycia o 600%. Celu, ktory udalo sie zrealizowac z nawiazka. Minister wyrazil rowniez nadzieje iz na nowych stanowiskach i w nowych zakladach czlonkowie Resortu stana na wysokosci zadania tak jak robili to dotychczas. Wszystkich nagrodzono gromkimi brawami a nastepnie rozpoczela sie czesc konsumpcyjno - artystyczna. Zabawy i tance w rytm nowoczesnej muzyki granej przez specjalnie w tym celu zaproszone slawy estrady trwaly do bialego rana. Gratulujemy Wam Towarzysze. Jestescie wzorem do nasladowania. Narod jest z Was dumny.";
static tBitMap *s_pBitmapScroll;
static const char *s_pCurrentChar;
static WORD s_wDrawnTextEnd;
static WORD s_wClearedBgEnd;

/**
 * @brief Draw characters to fill the void on the right.
 */
static void pageNewsFillScrollWithText(void) {
	if(s_wClearedBgEnd > 0 && s_wClearedBgEnd < SCROLL_WIDTH_BUFFER) {
		blitRect(s_pBitmapScroll, s_wClearedBgEnd, 0, SCROLL_WIDTH_BUFFER - s_wClearedBgEnd, g_pFont->uwHeight, 0);
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
		fontDrawStr1bpp(g_pFont, s_pBitmapScroll, s_wDrawnTextEnd, 0, szCurrentChar);
		s_wDrawnTextEnd += ubDrawWidth;
	}
}

static void pageNewsProcess(void) {
	UBYTE ubSpeed = commNavCheck(COMM_NAV_BTN) ? SCROLL_SPEED_FAST : SCROLL_SPEED_SLOW;

	// Shift scroll contents right
	UBYTE *pStart = &s_pBitmapScroll->Planes[0][s_pBitmapScroll->BytesPerRow * s_pBitmapScroll->Rows - sizeof(UWORD)];
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

		blitCopy(
			s_pBitmapScroll, 0, 0,
			commGetDisplayBuffer(), commGetOriginDisplay().uwX + 2,
			commGetOriginDisplay().uwY + COMM_DISPLAY_HEIGHT - g_pFont->uwHeight - 4,
			SCROLL_WIDTH_VISIBLE, g_pFont->uwHeight, MINTERM_COOKIE
		);
	}
	else {
		menuGsEnter(0);
	}
}

static void pageNewsDestroy(void) {
	bitmapDestroy(s_pBitmapScroll);
}

void pageNewsCreate(tNewsKind eEnding) {
	logBlockBegin("pageNewsCreate(eEnding: %d)", eEnding);
	commRegisterPage(pageNewsProcess, pageNewsDestroy);

	tUwCoordYX sOrigin = commGetOrigin();
	tBitMap *pBitmapNews = bitmapCreateFromFile("data/comm_news.bm", 0);
	blitCopy(
		pBitmapNews, 0, 0, commGetDisplayBuffer(), sOrigin.uwX + 25, sOrigin.uwY + 28,
		bitmapGetByteWidth(pBitmapNews) * 8, pBitmapNews->Rows, MINTERM_COOKIE
	);
	bitmapDestroy(pBitmapNews);

	s_pBitmapScroll = bitmapCreate(SCROLL_WIDTH_BUFFER, g_pFont->uwHeight, 1, BMF_CLEAR);
	s_pCurrentChar = s_szScroll;
	s_wDrawnTextEnd = SCROLL_WIDTH_VISIBLE;
	s_wClearedBgEnd = SCROLL_WIDTH_BUFFER;
	logBlockEnd("pageNewsCreate()");
}
