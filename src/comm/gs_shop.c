/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gs_shop.h"
#include <ace/managers/system.h>
#include <comm/base.h>
#include <comm/page_warehouse.h>
#include <comm/page_workshop.h>
#include <comm/page_office.h>
#include <comm/page_news.h>
#include <comm/page_msg.h>
#include <comm/page_accounting.h>
#include <comm/page_bribe.h>
#include <comm/page_favor.h>
#include <comm/page_list.h>
#include "core.h"
#include "hud.h"
#include "tutorial.h"
#include "game.h"
#include "inbox.h"

static tCommTab s_eTab;
static UBYTE s_isShown;
static tCommShopPage s_eCurrentPage;
static tCommShopPage s_eCameFrom;

//------------------------------------------------------------------ PRIVATE FNS

static tCommTab commShopPageToTab(tCommShopPage ePage) {
	switch(ePage) {
		case COMM_SHOP_PAGE_OFFICE_MAIN:
			return COMM_TAB_OFFICE;
		case COMM_SHOP_PAGE_WAREHOUSE:
			return COMM_TAB_WAREHOUSE;
		case COMM_SHOP_PAGE_WORKSHOP:
			return COMM_TAB_WORKSHOP;
		default:
			return COMM_TAB_COUNT;
	}
}

static tCommShopPage commShopTabToPage(tCommTab eTab) {
	switch(eTab) {
		case COMM_TAB_OFFICE:
			return COMM_SHOP_PAGE_OFFICE_MAIN;
		case COMM_TAB_WAREHOUSE:
			return COMM_SHOP_PAGE_WAREHOUSE;
		case COMM_TAB_WORKSHOP:
			return COMM_SHOP_PAGE_WORKSHOP;
		default:
			return COMM_SHOP_PAGE_COUNT;
	}
}

static void onBack(void) {
	tCommShopPage ePage;
	if(inboxTryPopBack(&ePage)) {
		commShopChangePage(s_eCameFrom, ePage);
	}
	else {
		commShopChangePage(COMM_SHOP_PAGE_COUNT, s_eCameFrom);
	}
}

static void onBackFromLastRebuke(void) {
	commShopChangePage(COMM_SHOP_PAGE_COUNT, COMM_SHOP_PAGE_NEWS_REBUKES);
}

static void commGsShopCreate(void) {
	s_isShown = commTryShow();
	if(!s_isShown) {
		// Camera not placed properly
		statePop(g_pGameStateManager);
		return;
	}

	tCommShopPage ePage;
	if(!inboxTryPopBack(&ePage)) {
		ePage = COMM_SHOP_PAGE_OFFICE_MAIN;
	}
	commShopChangePage(COMM_SHOP_PAGE_OFFICE_MAIN, ePage);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void commGsShopLoop(void) {
	commProcess();
	tutorialProcess();

	hudUpdate();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	if(commShopPageToTab(s_eCurrentPage) != COMM_TAB_COUNT) {
		tCommTab eOldTab = s_eTab;
		// Tab nav using shift+left / shift+right
		if(commNavExUse(COMM_NAV_EX_SHIFT_LEFT)) {
			if(s_eTab) {
				--s_eTab;
			}
			else {
				s_eTab = COMM_TAB_COUNT - 1;
			}
		}
		else if(commNavExUse(COMM_NAV_EX_SHIFT_RIGHT)) {
			if(s_eTab < COMM_TAB_COUNT - 1) {
				++s_eTab;
			}
			else {
				s_eTab = 0;
			}
		}
		if(s_eTab != eOldTab) {
			tCommShopPage ePage = commShopTabToPage(s_eTab);
			commShopChangePage(s_eCurrentPage, ePage);
			return;
		}
	}

	if(!commProcessPage()) {
		logWrite("shop quit\n");
		statePop(g_pGameStateManager);
		return;
	}
}

static void commGsShopDestroy(void) {
	if(!s_isShown) {
		return;
	}

	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}

//------------------------------------------------------------------- PUBLIC FNS

void commShopChangePage(tCommShopPage eCameFrom, tCommShopPage ePage) {
	s_eCurrentPage = ePage;
	s_eTab = commShopPageToTab(ePage);

	commSetActiveLed(s_eTab);
	commEraseAll();

	s_eCameFrom = eCameFrom;
	switch(ePage) {
		case COMM_SHOP_PAGE_WORKSHOP:
			pageWorkshopCreate();
			break;
		case COMM_SHOP_PAGE_WAREHOUSE:
			pageWarehouseCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_KRYSTYNA_DOSSIER:
			pageMsgCreate("dossier_krystyna", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KRYSTYNA_ACCOUNTING:
			pageAccountingCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_DOSSIER:
			pageMsgCreate("dossier_urzedas", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_BRIBE:
			pageBribeCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_FAVOR:
			pageFavorCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_DOSSIER:
			pageMsgCreate("dossier_komisarz", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_WELCOME:
			pageMsgCreate("komisarz_welcome", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_1:
			pageMsgCreate("komisarz_rebuke_1", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_2:
			pageMsgCreate("komisarz_rebuke_2", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_3:
			pageMsgCreate("komisarz_rebuke_3", onBackFromLastRebuke);
			break;
		case COMM_SHOP_PAGE_OFFICE_LIST_MIETEK:
		case COMM_SHOP_PAGE_OFFICE_LIST_KRYSTYNA:
		case COMM_SHOP_PAGE_OFFICE_LIST_KOMISARZ:
		case COMM_SHOP_PAGE_OFFICE_LIST_URZEDAS: {
			tCommFace eFace = ePage - COMM_SHOP_PAGE_OFFICE_LIST_MIETEK + COMM_FACE_MIETEK;
			pageListCreate(eFace);
		} break;
		case COMM_SHOP_PAGE_NEWS_REBUKES:
			pageNewsCreate(NEWS_KIND_REBUKES);
			break;
		case COMM_SHOP_PAGE_NEWS_ACCOLADES:
			pageNewsCreate(NEWS_KIND_ACCOLADES);
			break;
		case COMM_SHOP_PAGE_OFFICE_MAIN:
		default:
			pageOfficeCreate();
			break;
	}
}

UBYTE commShopIsActive(void) {
	return s_isShown;
}

void commShopGoBack(void) {
	onBack();
}

tState g_sStateShop = {
	.cbCreate = commGsShopCreate, .cbLoop = commGsShopLoop,
	.cbDestroy = commGsShopDestroy
};
