/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_SHOP_H_
#define _COMM_SHOP_H_

#include "aminer.h"
#include <comm/comm.h>
#include "defs.h"

typedef enum _tCommShopPage {
	COMM_SHOP_PAGE_WAREHOUSE,
	COMM_SHOP_PAGE_WORKSHOP,
	// Office - must be same order as MSG_PAGE_* in defs.h
	COMM_SHOP_PAGE_OFFICE_MAIN,
	COMM_SHOP_PAGE_OFFICE_LIST_MIETEK,
	COMM_SHOP_PAGE_OFFICE_LIST_KRYSTYNA,
	COMM_SHOP_PAGE_OFFICE_LIST_KOMISARZ,
	COMM_SHOP_PAGE_OFFICE_LIST_URZEDAS,
	COMM_SHOP_PAGE_OFFICE_LIST_ARCH,
	COMM_SHOP_PAGE_OFFICE_LIST_PRISONER,
	COMM_SHOP_PAGE_OFFICE_LIST_AGENT,

	COMM_SHOP_PAGE_OFFICE_MIETEK_WELCOME,
	COMM_SHOP_PAGE_OFFICE_MIETEK_FIRST_PLAN,
	COMM_SHOP_PAGE_OFFICE_MIETEK_PLAN_COMPLETE,

	COMM_SHOP_PAGE_OFFICE_KRYSTYNA_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_KRYSTYNA_ACCOUNTING,

	COMM_SHOP_PAGE_OFFICE_URZEDAS_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_WELCOME,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_FIRST_PLAN,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_COMPLETE,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_DELAYED,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_BRIBE,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_FAVOR,

	COMM_SHOP_PAGE_OFFICE_KOMISARZ_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_WELCOME,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_DINO_INTRO,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_1,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_2,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_3,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_ARCH_ACCOLADE,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_QUESTIONING,

	COMM_SHOP_PAGE_OFFICE_ARCH_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_ARCH_WELCOME,
	COMM_SHOP_PAGE_OFFICE_ARCH_PLAN_FAIL,
	COMM_SHOP_PAGE_OFFICE_ARCH_ACCOLADE,
	COMM_SHOP_PAGE_OFFICE_ARCH_GATE_OPENED,
	COMM_SHOP_PAGE_OFFICE_ARCH_GATE_DESTROYED,

	COMM_SHOP_PAGE_OFFICE_PRISONER_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_PRISONER_WELCOME,
	COMM_SHOP_PAGE_OFFICE_PRISONER_GATE_DESTROYED,
	COMM_SHOP_PAGE_OFFICE_PRISONER_RADIO_1,
	COMM_SHOP_PAGE_OFFICE_PRISONER_RADIO_2,
	COMM_SHOP_PAGE_OFFICE_PRISONER_RADIO_3,
	// Intro & Outro
	COMM_SHOP_PAGE_NEWS_REBUKES,
	COMM_SHOP_PAGE_NEWS_ACCOLADES,
	COMM_SHOP_PAGE_NEWS_GATE_ENEMY,
	COMM_SHOP_PAGE_NEWS_GATE_RED,
	// Subpages
	COMM_SHOP_PAGE_SOKOBAN,
	COMM_SHOP_PAGE_MARKET,
	// Gate cutscene
	COMM_SHOP_PAGE_ARCH_GATE_PLEA,
	COMM_SHOP_PAGE_PRISONER_GATE_PLEA,
	COMM_SHOP_PAGE_GATE_DILEMMA,

	COMM_SHOP_PAGE_COUNT
} tCommShopPage;

typedef enum tTabNavigationState {
	TAB_NAVIGATION_STATE_DISABLED,
	TAB_NAVIGATION_STATE_ENABLED,
	TAB_NAVIGATION_STATE_DISABLING,
} tTabNavigationState;

extern tState g_sStateShop;

UBYTE commShopIsActive(void);

tCommShopPage commShopGetCurrentPage(void);

tCommTab commShopPageToTab(tCommShopPage ePage);

tMsg commShopPageToTitle(tCommShopPage ePage);

void commShopChangePage(tCommShopPage eCameFrom, tCommShopPage ePage);

void commShopGoBack(void);

tTabNavigationState commShopGetTabNavigationState(void);

void commShopFocusOnTabNavigation(void);

#endif // _COMM_SHOP_H_
