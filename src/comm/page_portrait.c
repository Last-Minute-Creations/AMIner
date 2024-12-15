#include "page_portrait.h"
#include "assets.h"
#include <comm/page_office.h>

#define SCROLL_SPEED_SLOW 1
#define SCROLL_SPEED_FAST 8
#define SCROLL_WIDTH_VISIBLE 160
#define SCROLL_WIDTH_BUFFER 176

static void pagePortraitProcess(void) {
	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		commShopChangePage(
			COMM_SHOP_PAGE_OFFICE_LIST_SCI,
			COMM_SHOP_PAGE_OFFICE_SCIENTIST_MINER_TEXT
		);
	}
}

void pagePortraitCreate(void) {
	logBlockBegin("pagePortraitCreate()");
	commRegisterPage(pagePortraitProcess, 0);

	tUwCoordYX sOrigin = commGetOrigin();
	tBitMap *pPortrait = bitmapCreateFromFile("data/portrait.bm", 0);
	UWORD uwPortraitWidth = bitmapGetByteWidth(pPortrait) * 8;
	UWORD uwPortraitHeight = pPortrait->Rows;
	blitCopy(
		pPortrait, 0, 0, commGetDisplayBuffer(),
		sOrigin.uwX + COMM_DISPLAY_X + (COMM_DISPLAY_WIDTH - uwPortraitWidth) / 2,
		sOrigin.uwY + COMM_DISPLAY_Y + (COMM_DISPLAY_HEIGHT - uwPortraitHeight) / 2,
		uwPortraitWidth, uwPortraitHeight, MINTERM_COOKIE
	);
	bitmapDestroy(pPortrait);

	// TODO: play sound
	ptplayerLoadMod(g_pMenuMod, g_pModSampleData, 0);
	ptplayerEnableMusic(1);

	logBlockEnd("pagePortraitCreate()");
}
