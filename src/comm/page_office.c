/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_office.h"
#include <comm/page_favor.h>
#include <comm/page_bribe.h>
#include <comm/page_accounting.h>
#include <comm/gs_shop.h>
#include "save.h"

#define PPL_PER_ROW 4
#define SUBPAGES_PER_PERSON 8

typedef enum _tOfficeControls {
	OFFICE_CONTROLS_ACCEPT_DECLINE,
	OFFICE_CONTROLS_OK,
} tOfficeControls;

//----------------------------------------------------------------- PRIVATE VARS

static tFaceId s_pActivePpl[FACE_ID_COUNT]; // Key: pos in office, val: ppl
static tCommShopPage s_pOfficePages[FACE_ID_COUNT][SUBPAGES_PER_PERSON];
static BYTE s_bSelectionCurr;
static UBYTE s_ubUnlockedPplCount;

//------------------------------------------------------------------ PRIVATE FNS

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

	if(commNavUse(DIRECTION_LEFT)) {
		--s_bSelectionCurr;
	}
	else if(commNavUse(DIRECTION_RIGHT)) {
		++s_bSelectionCurr;
	}
	else if(commNavUse(DIRECTION_DOWN)) {
		s_bSelectionCurr += PPL_PER_ROW;
	}
	else if(commNavUse(DIRECTION_UP)){
		s_bSelectionCurr -= PPL_PER_ROW;
	}

	while(s_bSelectionCurr < 0) {
		s_bSelectionCurr += s_ubUnlockedPplCount;
	}

	while(s_bSelectionCurr >= s_ubUnlockedPplCount) {
		s_bSelectionCurr -= s_ubUnlockedPplCount;
	}

	if(s_bSelectionCurr != bOldSelection) {
		officeDrawFaceAtPos(bOldSelection);
		officeDrawFaceAtPos(s_bSelectionCurr);
	}

	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		commShopChangePage(
			COMM_SHOP_PAGE_OFFICE_MAIN,
			COMM_SHOP_PAGE_OFFICE_LIST_MIETEK + s_pActivePpl[s_bSelectionCurr] - FACE_ID_MIETEK
		);
	}
}

//------------------------------------------------------------------- PUBLIC FNS

void pageOfficeShow(void) {
	commRegisterPage(pageOfficeProcess, 0);
	for(tFaceId i = 0; i < s_ubUnlockedPplCount; ++i) {
		officeDrawFaceAtPos(i);
	}
}

void pageOfficeReset(void) {
	// Make all ppl locked
	for(tFaceId ePos = 0; ePos < FACE_ID_COUNT; ++ePos) {
		s_pActivePpl[ePos] = FACE_ID_COUNT;
	}
	s_ubUnlockedPplCount = 0;

	// Lock all dialogue options
	for(tFaceId ePerson = 0; ePerson < FACE_ID_COUNT; ++ePerson) {
		s_pOfficePages[ePerson][0] = COMM_SHOP_PAGE_OFFICE_MAIN; // Serves as list terminator
	}

	// Unlock select characters
	pageOfficeUnlockPerson(FACE_ID_MIETEK);
	pageOfficeUnlockPerson(FACE_ID_KRYSTYNA);

	// Unlock select pages
	pageOfficeTryUnlockPersonSubpage(FACE_ID_KRYSTYNA, COMM_SHOP_PAGE_OFFICE_KRYSTYNA_DOSSIER);
	pageOfficeTryUnlockPersonSubpage(FACE_ID_KRYSTYNA, COMM_SHOP_PAGE_OFFICE_KRYSTYNA_ACCOUNTING);

	// Reset counters
	pageFavorReset();
	pageBribeReset();
	pageAccountingReset();
}

void pageOfficeSave(tFile *pFile) {
	saveWriteHeader(pFile, "OFFC");
	fileWrite(pFile, s_pActivePpl, sizeof(s_pActivePpl[0]) * FACE_ID_COUNT);
	fileWrite(pFile, s_pOfficePages, sizeof(s_pOfficePages[0][0]) * FACE_ID_COUNT * SUBPAGES_PER_PERSON);
	fileWrite(pFile, &s_bSelectionCurr, sizeof(s_bSelectionCurr));
	fileWrite(pFile, &s_ubUnlockedPplCount, sizeof(s_ubUnlockedPplCount));
	pageFavorSave(pFile);
	pageBribeSave(pFile);
	pageAccountingSave(pFile);
}

UBYTE pageOfficeLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "OFFC")) {
		return 0;
	}

	fileRead(pFile, s_pActivePpl, sizeof(s_pActivePpl[0]) * FACE_ID_COUNT);
	fileRead(pFile, s_pOfficePages, sizeof(s_pOfficePages[0][0]) * FACE_ID_COUNT * SUBPAGES_PER_PERSON);
	fileRead(pFile, &s_bSelectionCurr, sizeof(s_bSelectionCurr));
	fileRead(pFile, &s_ubUnlockedPplCount, sizeof(s_ubUnlockedPplCount));
	return pageFavorLoad(pFile) &&
		pageBribeLoad(pFile) &&
		pageAccountingLoad(pFile);
}

void pageOfficeUnlockPerson(tFaceId ePerson) {
	s_pActivePpl[s_ubUnlockedPplCount++] = ePerson;
}

UBYTE pageOfficeTryUnlockPersonSubpage(tFaceId ePerson, tCommShopPage eSubpage) {
	for(UBYTE i = 0; i < SUBPAGES_PER_PERSON - 1; ++i) {
		if(s_pOfficePages[ePerson][i] == eSubpage) {
			// Already unlocked
			return 0;
		}
		if(s_pOfficePages[ePerson][i] == COMM_SHOP_PAGE_OFFICE_MAIN) {
			s_pOfficePages[ePerson][i] = eSubpage;
			s_pOfficePages[ePerson][i + 1] = COMM_SHOP_PAGE_OFFICE_MAIN;
			return 1;
		}
	}

	logWrite(
		"ERR: Can't add subpage %d to person %d - no more space\n",
		eSubpage, ePerson
	);
	return 0;
}

const tCommShopPage *officeGetPagesForFace(tFaceId eFace) {
	return s_pOfficePages[eFace];
}