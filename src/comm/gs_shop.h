/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_SHOP_H_
#define _COMM_SHOP_H_

#include "aminer.h"
#include <comm/base.h>

typedef enum _tCommShopPage {
	COMM_SHOP_PAGE_WAREHOUSE,
	COMM_SHOP_PAGE_WORKSHOP,
	COMM_SHOP_PAGE_OFFICE_MAIN,
	COMM_SHOP_PAGE_OFFICE_LIST_MIETEK,
	COMM_SHOP_PAGE_OFFICE_LIST_KRYSTYNA,
	COMM_SHOP_PAGE_OFFICE_LIST_KOMISARZ,
	COMM_SHOP_PAGE_OFFICE_LIST_URZEDAS,
	COMM_SHOP_PAGE_OFFICE_MIETEK_WELCOME,
	COMM_SHOP_PAGE_OFFICE_KRYSTYNA_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_KRYSTYNA_ACCOUNTING,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_WELCOME,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_FIRST_PLAN,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_BRIBE,
	COMM_SHOP_PAGE_OFFICE_URZEDAS_FAVOR,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_DOSSIER,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_WELCOME,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_1,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_2,
	COMM_SHOP_PAGE_OFFICE_KOMISARZ_REBUKE_3,
	COMM_SHOP_PAGE_NEWS_REBUKES,
	COMM_SHOP_PAGE_NEWS_ACCOLADES,
	COMM_SHOP_PAGE_COUNT
} tCommShopPage;

extern tState g_sStateShop;

UBYTE commShopIsActive(void);

tCommShopPage commShopGetCurrentPage(void);

tCommTab commShopPageToTab(tCommShopPage ePage);

void commShopChangePage(tCommShopPage eCameFrom, tCommShopPage ePage);

void commShopGoBack(void);

#endif // _COMM_SHOP_H_
