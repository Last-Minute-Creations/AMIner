/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_office.h"
#include <comm/base.h>
#include <comm/page_list.h>
#include <comm/page_favor.h>
#include <comm/page_bribe.h>
#include <comm/page_accounting.h>

#define PPL_PER_ROW 4

/**
 * @brief List of people in the office.
 * Must be the exact same order as in office page list!
 */
typedef enum _tOfficePpl {
	OFFICE_PPL_MIETEK,
	OFFICE_PPL_KRYSTYNA,
	OFFICE_PPL_PUTIN,
	OFFICE_PPL_URZEDAS,
	OFFICE_PPL_COUNT,
} tOfficePpl;

typedef enum _tOfficeControls {
	OFFICE_CONTROLS_ACCEPT_DECLINE,
	OFFICE_CONTROLS_OK,
} tOfficeControls;

static const tOfficePage s_pOfficePages[OFFICE_PPL_COUNT][4] = {
	[OFFICE_PPL_MIETEK] = {
		OFFICE_PAGE_MAIN
	},
	[OFFICE_PPL_KRYSTYNA] = {
		OFFICE_PAGE_DOSSIER_KRYSTYNA, OFFICE_PAGE_ACCOUNTING, OFFICE_PAGE_MAIN
	},
	[OFFICE_PPL_PUTIN] = {
		OFFICE_PAGE_MAIN
	},
	[OFFICE_PPL_URZEDAS] = {
		OFFICE_PAGE_DOSSIER_URZEDAS, OFFICE_PAGE_FAVOR, OFFICE_PAGE_BRIBE, OFFICE_PAGE_MAIN
	},
};

//---------------------------------------------------------------- OFFICE COMMON

static tOfficePpl s_pActivePpl[OFFICE_PPL_COUNT]; // Key: pos in office, val: ppl
static BYTE s_bSelectionCurr, s_bSelectionCount;

//------------------------------------------------------------- OFFICE PAGE MAIN

static void officeDrawFaceAtPos(BYTE bPos) {
	const UBYTE ubSpaceX = (COMM_DISPLAY_WIDTH - 2*2 - PPL_PER_ROW * 32) / 3;
	const UBYTE ubSpaceY = 10;
	const tUwCoordYX sOrigin = commGetOriginDisplay();

	UWORD uwX = sOrigin.uwX + 2 + (bPos % PPL_PER_ROW) * (32 + ubSpaceX);
	UWORD uwY = sOrigin.uwY + 2 + (bPos / PPL_PER_ROW) * (32 + ubSpaceY);

	tBitMap *pBmDraw = commGetDisplayBuffer();
	blitRect(pBmDraw, uwX - 2, uwY - 2, 36, 36, COMM_DISPLAY_COLOR_BG);

	// Draw selection
	if(bPos == s_bSelectionCurr) {
		blitCopy(g_pCommBmSelection, 0, 0,  pBmDraw, uwX - 2, uwY - 2, 16, 9, MINTERM_COOKIE);
		blitCopy(g_pCommBmSelection, 0, 9,  pBmDraw, uwX - 2 + 36 - 16, uwY - 2, 16, 9, MINTERM_COOKIE);
		blitCopy(g_pCommBmSelection, 0, 18, pBmDraw, uwX - 2, uwY - 2 + 36 - 9, 16, 9, MINTERM_COOKIE);
		blitCopy(g_pCommBmSelection, 0, 27, pBmDraw, uwX - 2 + 36 - 16, uwY - 2 + 36 - 9, 16, 9, MINTERM_COOKIE);
	}

	blitCopy(
		g_pCommBmFaces, 0, s_pActivePpl[bPos] * 32, pBmDraw, uwX, uwY,
		32, 32, MINTERM_COOKIE
	);
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
		pageListCreate(s_pOfficePages[s_pActivePpl[s_bSelectionCurr]]);
	}
}

void pageOfficeCreate(void) {
	commRegisterPage(pageOfficeProcess, 0);
	s_bSelectionCount = 0;
	for(tOfficePpl i = 0; i < OFFICE_PPL_COUNT; ++i) {
		if(s_pActivePpl[i] == OFFICE_PPL_COUNT) {
			break;
		}
		officeDrawFaceAtPos(i);
		++s_bSelectionCount;
	}
}

void pageOfficeReset(void) {
	for(tOfficePpl i = 0; i < OFFICE_PPL_COUNT; ++i) {
		s_pActivePpl[i] = OFFICE_PPL_COUNT;
	}
	UBYTE ubPos = 0;
	// s_pActivePpl[ubPos++] = OFFICE_PPL_MIETEK;
	s_pActivePpl[ubPos++] = OFFICE_PPL_KRYSTYNA;
	s_pActivePpl[ubPos++] = OFFICE_PPL_URZEDAS;

	// Reset counters
	pageFavorReset();
	pageBribeReset();
	pageAccountingReset();
}
