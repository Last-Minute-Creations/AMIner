#include "page_portrait.h"
#include "assets.h"
#include "settings.h"
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

	g_sSettings.ubSokoUnlock = SETTINGS_SOKO_UNLOCK_ON;

	// TODO: play sound
	ptplayerLoadMod(g_pMenuMod, g_pModSampleData, 0);
	ptplayerEnableMusic(1);

	logBlockEnd("pagePortraitCreate()");
}
