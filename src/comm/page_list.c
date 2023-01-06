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

#define PORTRAIT_X 0
#define PORTRAIT_Y 0
#define PORTRAIT_HEIGHT 32
#define LIST_X 40
#define LIST_Y 40
#define TITLE_HEIGHT 8
#define TITLE_X LIST_X
#define TITLE_Y (PORTRAIT_HEIGHT - TITLE_HEIGHT)
#define LIST_SPACING_Y 10

static BYTE s_bPosCurr, s_bPosCount;
static const tOfficePage *s_pCurrentList;
static tCommFace s_eFace;

static void officeDrawListPos(tOfficePage eListPage, UBYTE ubPos) {
	UBYTE ubColor = (
		ubPos == s_bPosCurr ?
		COMM_DISPLAY_COLOR_TEXT :
		COMM_DISPLAY_COLOR_TEXT_DARK
	);
	commDrawText(
		LIST_X, LIST_Y + LIST_SPACING_Y * ubPos, g_pMsgs[MSG_PAGE_MAIN + eListPage],
		FONT_COOKIE, ubColor
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
		pageOfficeOpenSubpage(
			OFFICE_PAGE_LIST_MIETEK + s_eFace - COMM_FACE_MIETEK, s_pCurrentList[s_bPosCurr]
		);
	}
}

void pageListCreate(tCommFace eFace, const tOfficePage *pPages) {
	s_eFace = eFace;
	s_pCurrentList = pPages;
	commRegisterPage(pageListProcess, 0);
	s_bPosCurr = 0;

	commDrawTitle(TITLE_X, TITLE_Y, g_pMsgs[MSG_PAGE_LIST_MIETEK + s_eFace]);
	commDrawFaceAt(s_eFace, PORTRAIT_X, PORTRAIT_Y);

	tOfficePage eListPage;
	s_bPosCount = 0;
	do {
		eListPage = pPages[s_bPosCount];
		officeDrawListPos(eListPage, s_bPosCount);
		++s_bPosCount;
	} while(eListPage != OFFICE_PAGE_MAIN);
}
