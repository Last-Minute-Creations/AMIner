/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_list.h"
#include "page_office.h"
#include <comm/base.h>
#include <comm/page_office.h>
#include <comm/page_msg.h>
#include <comm/page_bribe.h>
#include <comm/page_favor.h>
#include <comm/page_accounting.h>
#include "../defs.h"

static BYTE s_bPosCurr, s_bPosCount;
static const tOfficePage *s_pCurrentList;

static void cbOnMsgClose(void) {
	pageListCreate(s_pCurrentList);
}

static void officeDrawListPos(tOfficePage eListPage, UBYTE ubPos) {
	UBYTE ubColor = (
		ubPos == s_bPosCurr ?
		COMM_DISPLAY_COLOR_TEXT :
		COMM_DISPLAY_COLOR_TEXT_DARK
	);
	commDrawText(
		0, 10 * ubPos, g_pMsgs[MSG_PAGE_MAIN + eListPage], FONT_COOKIE, ubColor
	);
}

static void pageListProcess(void) {
	BYTE bPrevPos = s_bPosCurr;
	if(commNavUse(COMM_NAV_DOWN)) {
		if(++s_bPosCurr >= s_bPosCount) {
			s_bPosCurr = 0;
		}
	}
	else if(commNavUse(COMM_NAV_UP)) {
		if(--s_bPosCurr < 0) {
			s_bPosCurr = s_bPosCount - 1;
		}
	}

	if(bPrevPos != s_bPosCurr) {
		officeDrawListPos(s_pCurrentList[bPrevPos], bPrevPos);
		officeDrawListPos(s_pCurrentList[s_bPosCurr], s_bPosCurr);
	}
	else if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		switch(s_pCurrentList[s_bPosCurr]) {
			case OFFICE_PAGE_DOSSIER_KRYSTYNA:
				pageMsgCreate("dossier_krystyna", cbOnMsgClose);
				break;
			case OFFICE_PAGE_DOSSIER_URZEDAS:
				pageMsgCreate("dossier_urzedas", cbOnMsgClose);
				break;
			case OFFICE_PAGE_BRIBE:
				pageBribeCreate();
				break;
			case OFFICE_PAGE_FAVOR:
				pageFavorCreate();
				break;
			case OFFICE_PAGE_ACCOUNTING:
				pageAccountingCreate();
				break;
			case OFFICE_PAGE_LIST_MIETEK:
			case OFFICE_PAGE_LIST_KRYSTYNA:
			case OFFICE_PAGE_LIST_PUTIN:
			case OFFICE_PAGE_LIST_URZEDAS:
			case OFFICE_PAGE_MAIN:
			default:
				pageOfficeCreate();
				break;
		}
	}
}

void pageListCreate(const tOfficePage *pPages) {
	s_pCurrentList = pPages;
	commRegisterPage(pageListProcess, 0);
	tOfficePage eListPage;
	s_bPosCurr = 0;
	s_bPosCount = 0;
	do {
		eListPage = pPages[s_bPosCount];
		officeDrawListPos(eListPage, s_bPosCount);
		++s_bPosCount;
	} while(eListPage != OFFICE_PAGE_MAIN);
}
