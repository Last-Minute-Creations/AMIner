/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_office.h"
#include <comm/page_list.h>
#include <comm/page_msg.h>
#include <comm/page_favor.h>
#include <comm/page_bribe.h>
#include <comm/page_news.h>
#include <comm/page_accounting.h>

#define PPL_PER_ROW 4
#define SUBPAGES_PER_PERSON 5

typedef enum _tOfficeControls {
	OFFICE_CONTROLS_ACCEPT_DECLINE,
	OFFICE_CONTROLS_OK,
} tOfficeControls;

//---------------------------------------------------------------- OFFICE COMMON

static tCommFace s_pActivePpl[COMM_FACE_COUNT]; // Key: pos in office, val: ppl
static tOfficePage s_pOfficePages[COMM_FACE_COUNT][SUBPAGES_PER_PERSON];
static BYTE s_bSelectionCurr, s_bSelectionCount;
static UBYTE s_ubUnlockedPplCount;
static tOfficePage s_eCameFrom;

//------------------------------------------------------------- OFFICE PAGE MAIN

static void officeDrawFaceAtPos(BYTE bPos) {
	const UBYTE ubSpaceX = (COMM_DISPLAY_WIDTH - 2*2 - PPL_PER_ROW * 32) / 3;
	const UBYTE ubSpaceY = 10;
	const tUwCoordYX sOrigin = commGetOriginDisplay();

	UWORD uwRelativeX = 2 + (bPos % PPL_PER_ROW) * (32 + ubSpaceX);
	UWORD uwRelativeY = 2 + (bPos / PPL_PER_ROW) * (32 + ubSpaceY);
	UWORD uwX = sOrigin.uwX + uwRelativeX;
	UWORD uwY = sOrigin.uwY + uwRelativeY;

	tBitMap *pBmDraw = commGetDisplayBuffer();
	blitRect(pBmDraw, uwX - 2, uwY - 2, 36, 36, COMM_DISPLAY_COLOR_BG);

	// Draw selection
	if(bPos == s_bSelectionCurr) {
		blitCopy(g_pCommBmSelection, 0, 0,  pBmDraw, uwX - 2, uwY - 2, 16, 9, MINTERM_COOKIE);
		blitCopy(g_pCommBmSelection, 0, 9,  pBmDraw, uwX - 2 + 36 - 16, uwY - 2, 16, 9, MINTERM_COOKIE);
		blitCopy(g_pCommBmSelection, 0, 18, pBmDraw, uwX - 2, uwY - 2 + 36 - 9, 16, 9, MINTERM_COOKIE);
		blitCopy(g_pCommBmSelection, 0, 27, pBmDraw, uwX - 2 + 36 - 16, uwY - 2 + 36 - 9, 16, 9, MINTERM_COOKIE);
	}

	commDrawFaceAt(s_pActivePpl[bPos], uwRelativeX, uwRelativeY);
}

static void pageOfficeProcess(void) {
	BYTE bOldSelection = s_bSelectionCurr;

	if(commNavUse(COMM_NAV_LEFT)) {
		--s_bSelectionCurr;
	}
	else if(commNavUse(COMM_NAV_RIGHT)) {
		++s_bSelectionCurr;
	}
	else if(commNavUse(COMM_NAV_DOWN)) {
		s_bSelectionCurr += PPL_PER_ROW;
	}
	else if(commNavUse(COMM_NAV_UP)){
		s_bSelectionCurr -= PPL_PER_ROW;
	}

	while(s_bSelectionCurr < 0) {
		s_bSelectionCurr += s_bSelectionCount;
	}

	while(s_bSelectionCurr >= s_bSelectionCount) {
		s_bSelectionCurr -= s_bSelectionCount;
	}

	if(s_bSelectionCurr != bOldSelection) {
		officeDrawFaceAtPos(bOldSelection);
		officeDrawFaceAtPos(s_bSelectionCurr);
	}

	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		pageListCreate(s_pActivePpl[s_bSelectionCurr], s_pOfficePages[s_pActivePpl[s_bSelectionCurr]]);
	}
}

static void onBackFromLastRebuke(void) {
	pageNewsCreate(ENDING_REBUKES);
}

//------------------------------------------------------------------- PUBLIC FNS

void pageOfficeCreate(void) {
	commRegisterPage(pageOfficeProcess, 0);
	s_bSelectionCount = 0;
	s_eCameFrom = OFFICE_PAGE_COUNT;
	for(tCommFace i = 0; i < COMM_FACE_COUNT; ++i) {
		if(s_pActivePpl[i] == COMM_FACE_COUNT) {
			break;
		}
		officeDrawFaceAtPos(i);
		++s_bSelectionCount;
	}
}

void pageOfficeReset(void) {
	// Make all ppl locked
	for(tCommFace ePos = 0; ePos < COMM_FACE_COUNT; ++ePos) {
		s_pActivePpl[ePos] = COMM_FACE_COUNT;
	}
	s_ubUnlockedPplCount = 0;

	// Lock all dialogue options
	for(tCommFace ePerson = 0; ePerson < COMM_FACE_COUNT; ++ePerson) {
		s_pOfficePages[ePerson][0] = OFFICE_PAGE_MAIN; // Serves as list terminator
	}

	// Unlock select characters
	pageOfficeUnlockPerson(COMM_FACE_KRYSTYNA);
	pageOfficeUnlockPerson(COMM_FACE_URZEDAS);
	pageOfficeUnlockPerson(COMM_FACE_KOMISARZ);

	// Unlock select pages
	pageOfficeUnlockPersonSubpage(COMM_FACE_KRYSTYNA, OFFICE_PAGE_KRYSTYNA_DOSSIER);
	pageOfficeUnlockPersonSubpage(COMM_FACE_KRYSTYNA, OFFICE_PAGE_KRYSTYNA_ACCOUNTING);
	pageOfficeUnlockPersonSubpage(COMM_FACE_URZEDAS, OFFICE_PAGE_URZEDAS_DOSSIER);
	pageOfficeUnlockPersonSubpage(COMM_FACE_URZEDAS, OFFICE_PAGE_URZEDAS_BRIBE);

	// TODO: Unlock later on
	pageOfficeUnlockPersonSubpage(COMM_FACE_URZEDAS, OFFICE_PAGE_URZEDAS_FAVOR);
	pageOfficeUnlockPersonSubpage(COMM_FACE_KOMISARZ, OFFICE_PAGE_KOMISARZ_DOSSIER);
	pageOfficeUnlockPersonSubpage(COMM_FACE_KOMISARZ, OFFICE_PAGE_KOMISARZ_REBUKE_1);
	pageOfficeUnlockPersonSubpage(COMM_FACE_KOMISARZ, OFFICE_PAGE_KOMISARZ_REBUKE_2);
	pageOfficeUnlockPersonSubpage(COMM_FACE_KOMISARZ, OFFICE_PAGE_KOMISARZ_REBUKE_3);

	// Reset counters
	pageFavorReset();
	pageBribeReset();
	pageAccountingReset();
}

void pageOfficeUnlockPerson(tCommFace ePerson) {
	s_pActivePpl[s_ubUnlockedPplCount++] = ePerson;
}

void pageOfficeUnlockPersonSubpage(tCommFace ePerson, tOfficePage eSubpage) {
	for(UBYTE i = 0; i < SUBPAGES_PER_PERSON - 1; ++i) {
		if(s_pOfficePages[ePerson][i] == OFFICE_PAGE_MAIN) {
			s_pOfficePages[ePerson][i] = eSubpage;
			s_pOfficePages[ePerson][i + 1] = OFFICE_PAGE_MAIN;
			return;
		}
	}

	logWrite(
		"ERR: Can't add subpage %d to person %d - no more space\n",
		eSubpage, ePerson
	);
}

void pageOfficeOpenSubpage(tOfficePage eCameFrom, tOfficePage eTarget) {
	s_eCameFrom = eCameFrom;
	switch(eTarget) {
		case OFFICE_PAGE_KRYSTYNA_DOSSIER:
			pageMsgCreate("dossier_krystyna", pageOfficeGoBack);
			break;
		case OFFICE_PAGE_KRYSTYNA_ACCOUNTING:
			pageAccountingCreate();
			break;
		case OFFICE_PAGE_URZEDAS_DOSSIER:
			pageMsgCreate("dossier_urzedas", pageOfficeGoBack);
			break;
		case OFFICE_PAGE_URZEDAS_BRIBE:
			pageBribeCreate();
			break;
		case OFFICE_PAGE_URZEDAS_FAVOR:
			pageFavorCreate();
			break;
		case OFFICE_PAGE_KOMISARZ_DOSSIER:
			pageMsgCreate("dossier_komisarz", pageOfficeGoBack);
			break;
		case OFFICE_PAGE_KOMISARZ_WELCOME:
			pageMsgCreate("komisarz_welcome", pageOfficeGoBack);
			break;
		case OFFICE_PAGE_KOMISARZ_REBUKE_1:
			pageMsgCreate("komisarz_rebuke_1", pageOfficeGoBack);
			break;
		case OFFICE_PAGE_KOMISARZ_REBUKE_2:
			pageMsgCreate("komisarz_rebuke_2", pageOfficeGoBack);
			break;
		case OFFICE_PAGE_KOMISARZ_REBUKE_3:
			pageMsgCreate("komisarz_rebuke_3", onBackFromLastRebuke);
			break;
		case OFFICE_PAGE_LIST_MIETEK:
		case OFFICE_PAGE_LIST_KRYSTYNA:
		case OFFICE_PAGE_LIST_KOMISARZ:
		case OFFICE_PAGE_LIST_URZEDAS: {
			tCommFace eFace = eTarget - OFFICE_PAGE_LIST_MIETEK + COMM_FACE_MIETEK;
			pageListCreate(eFace, s_pOfficePages[eFace]);
		} break;
		case OFFICE_PAGE_MAIN:
		default:
			pageOfficeCreate();
			break;
	}
}

void pageOfficeGoBack(void) {
	pageOfficeOpenSubpage(OFFICE_PAGE_COUNT, s_eCameFrom);
}
