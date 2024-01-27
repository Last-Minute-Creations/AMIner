/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gs_shop.h"
#include <ace/managers/system.h>
#include <comm/comm.h>
#include <comm/page_warehouse.h>
#include <comm/page_workshop.h>
#include <comm/page_office.h>
#include <comm/page_news.h>
#include <comm/page_msg.h>
#include <comm/page_accounting.h>
#include <comm/page_bribe.h>
#include <comm/page_favor.h>
#include <comm/page_gate_dilemma.h>
#include <comm/page_questioning.h>
#include <comm/page_list.h>
#include <comm/page_sokoban.h>
#include <comm/page_market.h>
#include "core.h"
#include "dino.h"
#include "game.h"
#include "hud.h"
#include "inbox.h"
#include "menu.h"
#include "tutorial.h"

static tCommTab s_eTab;
static tCommShopPage s_eCurrentPage;
static tCommShopPage s_eCameFrom;

//------------------------------------------------------------------ PRIVATE FNS

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
	if(inboxTryPopFront(&ePage)) {
		commShopChangePage(s_eCameFrom, ePage);
	}
	else {
		hudClearInboxNotification();
		commShopChangePage(COMM_SHOP_PAGE_COUNT, s_eCameFrom);
	}
}

static void onBackFromLastRebuke(void) {
	menuGsEnter(0);
}

static void commGsShopCreate(void) {
	UBYTE isShopShown = commTryShow(gameGetSteers(), 2, 0);
	if(!isShopShown) {
		// Camera not placed properly
		statePop(g_pGameStateManager);
		return;
	}

	pageOfficeResetSelection();

	tCommShopPage ePage;
	if(!inboxTryPopFront(&ePage)) {
		ePage = COMM_SHOP_PAGE_OFFICE_MAIN;
		hudClearInboxNotification();
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
	dinoProcess();

	hudUpdate();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	if(commShopPageToTab(s_eCurrentPage) != COMM_TAB_COUNT) {
		// Process inbox when on main tab page, e.g. after fulfilling plan
		tCommShopPage ePage;
		if(inboxTryPopFront(&ePage)) {
			commShopChangePage(s_eCameFrom, ePage);
			return;
		}
		hudClearInboxNotification();

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
		gameTriggerSave();
		logWrite("shop quit\n");
		statePop(g_pGameStateManager);
		return;
	}
}

static void commGsShopDestroy(void) {
	if(!commIsShown()) {
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
	const char *szTitle = g_pMsgs[commShopPageToTitle(ePage)];
	switch(ePage) {
		case COMM_SHOP_PAGE_WORKSHOP:
			pageWorkshopCreate();
			break;
		case COMM_SHOP_PAGE_WAREHOUSE:
			pageWarehouseCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_MIETEK_WELCOME:
			pageMsgCreate(FACE_ID_MIETEK, szTitle, "mietek_welcome", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KRYSTYNA_DOSSIER:
			pageMsgCreate(FACE_ID_KRYSTYNA, szTitle, "krystyna_dossier", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KRYSTYNA_ACCOUNTING:
			pageAccountingCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_DOSSIER:
			pageMsgCreate(FACE_ID_URZEDAS, szTitle, "urzedas_dossier", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_WELCOME:
			pageMsgCreate(FACE_ID_URZEDAS, szTitle, "urzedas_welcome", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_FIRST_PLAN:
			pageMsgCreate(FACE_ID_URZEDAS, szTitle, "urzedas_first_plan", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_COMPLETE:
			pageMsgCreate(FACE_ID_URZEDAS, szTitle, "urzedas_plan_complete", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_DELAYED:
			pageMsgCreate(FACE_ID_URZEDAS, szTitle, "urzedas_plan_delayed", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_BRIBE:
			pageBribeCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_URZEDAS_FAVOR:
			pageFavorCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_DOSSIER:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_dossier", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_WELCOME:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_welcome", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_DINO_INTRO:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_dino_intro", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_1:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_rebuke_1", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_2:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_rebuke_2", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_3:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_rebuke_3", onBackFromLastRebuke);
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_QUESTIONING:
			pageQuestioningCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_KOMISARZ_ARCH_ACCOLADE:
			pageMsgCreate(FACE_ID_KOMISARZ, szTitle, "komisarz_arch_accolade", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_ARCH_WELCOME:
			pageMsgCreate(FACE_ID_ARCH, szTitle, "arch_welcome", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_ARCH_ACCOLADE:
			pageMsgCreate(FACE_ID_ARCH, szTitle, "arch_accolade", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_ARCH_PLAN_FAIL:
			pageMsgCreate(FACE_ID_ARCH, szTitle, "arch_plan_fail", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_ARCH_GATE_FAIL:
			pageMsgCreate(FACE_ID_ARCH, szTitle, "arch_gate_fail", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_PRISONER_DOSSIER:
			pageMsgCreate(FACE_ID_PRISONER, szTitle, "prisoner_dossier", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_PRISONER_WELCOME:
			pageMsgCreate(FACE_ID_PRISONER, szTitle, "prisoner_welcome", onBack);
			break;
		case COMM_SHOP_PAGE_OFFICE_LIST_MIETEK:
		case COMM_SHOP_PAGE_OFFICE_LIST_KRYSTYNA:
		case COMM_SHOP_PAGE_OFFICE_LIST_KOMISARZ:
		case COMM_SHOP_PAGE_OFFICE_LIST_ARCH:
		case COMM_SHOP_PAGE_OFFICE_LIST_PRISONER:
		case COMM_SHOP_PAGE_OFFICE_LIST_AGENT:
		case COMM_SHOP_PAGE_OFFICE_LIST_URZEDAS: {
			tFaceId eFace = ePage - COMM_SHOP_PAGE_OFFICE_LIST_MIETEK + FACE_ID_MIETEK;
			pageListCreate(eFace);
		} break;
		case COMM_SHOP_PAGE_NEWS_ACCOLADES:
			pageNewsCreate(NEWS_KIND_ACCOLADES);
			break;
		case COMM_SHOP_PAGE_NEWS_GATE_ENEMY:
			pageNewsCreate(NEWS_KIND_GATE_ENEMY);
			break;
		case COMM_SHOP_PAGE_NEWS_GATE_RED:
			pageNewsCreate(NEWS_KIND_GATE_RED);
			break;
		case COMM_SHOP_PAGE_SOKOBAN:
			pageSokobanCreate();
			break;
		case COMM_SHOP_PAGE_MARKET:
			pageMarketCreate();
			break;
		case COMM_SHOP_PAGE_ARCH_GATE_PLEA:
			pageMsgCreate(FACE_ID_ARCH, "", "arch_gate_plea", onBack);
			break;
		case COMM_SHOP_PAGE_PRISONER_GATE_PLEA:
			pageMsgCreate(FACE_ID_PRISONER, "", "prisoner_gate_plea", onBack);
			break;
		case COMM_SHOP_PAGE_GATE_DILEMMA:
			pageGateDilemmaCreate();
			break;
		case COMM_SHOP_PAGE_OFFICE_MAIN:
		default:
			pageOfficeShow();
			break;
	}
}

UBYTE commShopIsActive(void) {
	return commIsShown();
}

tCommShopPage commShopGetCurrentPage(void) {
	return s_eCurrentPage;
}

tCommTab commShopPageToTab(tCommShopPage ePage) {
	switch(ePage) {
		case COMM_SHOP_PAGE_OFFICE_MAIN:
			return COMM_TAB_OFFICE;
		case COMM_SHOP_PAGE_WAREHOUSE:
		case COMM_SHOP_PAGE_SOKOBAN:
			return COMM_TAB_WAREHOUSE;
		case COMM_SHOP_PAGE_WORKSHOP:
			return COMM_TAB_WORKSHOP;
		default:
			return COMM_TAB_COUNT;
	}
}

tMsg commShopPageToTitle(tCommShopPage ePage) {
	return MSG_PAGE_LIST_MIETEK + ePage - COMM_SHOP_PAGE_OFFICE_LIST_MIETEK;
}

void commShopGoBack(void) {
	onBack();
}

tState g_sStateShop = {
	.cbCreate = commGsShopCreate, .cbLoop = commGsShopLoop,
	.cbDestroy = commGsShopDestroy
};
