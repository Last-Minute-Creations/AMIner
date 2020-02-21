/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comm_shop.h"
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/rand.h>
#include "comm.h"
#include "core.h"
#include "game.h"
#include "warehouse.h"
#include "mineral.h"
#include "button.h"
#include "vehicle.h"
#include "hud.h"
#include "tutorial.h"
#include "defs.h"
#include "inventory.h"

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

typedef enum _tOfficePage {
	OFFICE_PAGE_MAIN,
	OFFICE_PAGE_LIST_MIETEK,
	OFFICE_PAGE_LIST_KRYSTYNA,
	OFFICE_PAGE_LIST_PUTIN,
	OFFICE_PAGE_LIST_URZEDAS,
	OFFICE_PAGE_DOSSIER_KRYSTYNA,
	OFFICE_PAGE_DOSSIER_URZEDAS,
	OFFICE_PAGE_BRIBE,
	OFFICE_PAGE_FAVOR,
	OFFICE_PAGE_ACCOUNTING,
	OFFICE_PAGE_COUNT
} tOfficePage;

typedef enum _tShopMessageNames {
	SHOP_MSG_TIME_REMAINING,
	SHOP_MSG_ACCOLADES,
	SHOP_MSG_REBUKES,
	SHOP_MSG_MK,
	SHOP_MSG_UPGRADE_TO_MK,
	SHOP_MSG_STOCK,
	SHOP_MSG_BUY,
	SHOP_MSG_EXIT,
	SHOP_MSG_CONFIRM,
	SHOP_MSG_ALREADY_MAX,
	SHOP_MSG_ALREADY_FULL,
	SHOP_MSG_COUNT,
} tShopMessageNames;

typedef enum _tOfficeControls {
	OFFICE_CONTROLS_ACCEPT_DECLINE,
	OFFICE_CONTROLS_OK,
} tOfficeControls;

static UBYTE s_isShown;
static UBYTE s_isBtnPress = 0;
static tBitMap *s_pBmDraw;
static tCommLed s_eTab;

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

tStringArray g_sOfficePageNames;

//---------------------------------------------------------------- OFFICE COMMON

static tBitMap *s_pFaces, *s_pSelection;

static tOfficePpl s_pActivePpl[OFFICE_PPL_COUNT]; // Key: pos in office, val: ppl
static tOfficePage s_eOfficePage;
static UBYTE s_ubOfficeSelectionCount;
static BYTE s_bOfficeSelectionCurr;
static UBYTE s_ubFavorsLeft, s_ubBribeAccoladeCount, s_ubBribeRebukeCount;
static BYTE s_bBribeChanceFail, s_bAccountingChanceFail;
static tOfficeControls s_eControls;

static void officeMakePageCurrent(tOfficePage ePage);

static void drawAcceptDecline(void) {
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - 2 * g_pFont->uwHeight;
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, uwOffsY, "Accept", FONT_COOKIE | FONT_CENTER, (
			s_bOfficeSelectionCurr == 0 ?
			COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
		)
	);

	uwOffsY += g_pFont->uwHeight;
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, uwOffsY, "Decline", FONT_COOKIE | FONT_CENTER, (
			s_bOfficeSelectionCurr == 1 ?
			COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
		)
	);
	s_eControls = OFFICE_CONTROLS_ACCEPT_DECLINE;
	s_ubOfficeSelectionCount = 2;
}

static void drawOk(void) {
	UWORD uwOffsY = COMM_DISPLAY_HEIGHT - 2 * g_pFont->uwHeight;
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, uwOffsY, "OK", FONT_COOKIE | FONT_CENTER, (
			s_bOfficeSelectionCurr == 1 ?
			COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
		)
	);

	s_eControls = OFFICE_CONTROLS_OK;
	s_ubOfficeSelectionCount = 1;
}

static UBYTE drawLongText(
	const char *szText, UBYTE ubLineHeight, UWORD uwStartX, UWORD uwStartY
) {
	UBYTE ubLinesWritten = 1;

	UBYTE ubTextLength = strlen(szText);
	UWORD uwLineWidth = 0;
	UWORD uwCurrY = uwStartY;
	UBYTE ubCharsInLine = 0;
	UBYTE ubLastSpace = 0xFF; // next char, actually
	UWORD uwLastSpaceWidth = 0;
	char szLineBfr[50];
	for(uint8_t i = 0; i < ubTextLength; ++i) {
		UBYTE ubCharWidth = fontGlyphWidth(g_pFont, szText[i]) + 1;
		if(uwLineWidth + ubCharWidth >= (COMM_DISPLAY_WIDTH - uwStartX)) {
			if(ubLastSpace != 0xFF) {
				szLineBfr[ubLastSpace - 1] = '\0';
			}
			else {
				szLineBfr[ubCharsInLine] = '\0';
			}
			commDrawText(uwStartX, uwCurrY, szLineBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
			if(ubLastSpace != 0xFF) {
				ubCharsInLine -= ubLastSpace;
				for(UBYTE j = 0; j < ubCharsInLine; ++j) {
					szLineBfr[j] = szLineBfr[ubLastSpace + j];
				}
				uwLineWidth -= uwLastSpaceWidth;
			}
			else {
				uwLineWidth = 0;
				ubCharsInLine = 0;
			}
			ubLastSpace = 0xFF;
			uwLastSpaceWidth = 0;
			++ubLinesWritten;
			uwCurrY += ubLineHeight;
		}

		uwLineWidth += ubCharWidth;
		szLineBfr[ubCharsInLine++] = szText[i];
		if(szText[i] == ' ') {
			ubLastSpace = ubCharsInLine;
			uwLastSpaceWidth = uwLineWidth;
		}
	}

	if(ubCharsInLine) {
		szLineBfr[ubCharsInLine] = '\0';
		commDrawText(uwStartX, uwCurrY, szLineBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
	}

	return ubLinesWritten;
}

//------------------------------------------------------- OFFICE PAGE ACCOUNTING

static void officePreparePageAccounting(void) {
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = g_pFont->uwHeight + 1;

	UWORD uwCost = warehouseGetPlanRemainingCost(warehouseGetPlan()) / 2;

	uwPosY += drawLongText(
		"I can do some Creative Acccounting for you and fulfill your plan instantly."
		" For a price, of course.", ubLineHeight, 0, uwPosY
	) * ubLineHeight;
	char szBfr[150];
	uwPosY += ubLineHeight / 2;
	sprintf(
		szBfr, "There is %hhu%% chance that we will get caught, which would result in instantly getting a rebuke.",
		s_bAccountingChanceFail
	);
	uwPosY += drawLongText(szBfr,  ubLineHeight, 0, uwPosY) * ubLineHeight;
	sprintf(szBfr, "It will cost you %hu\x1F.", uwCost);
	uwPosY += drawLongText(szBfr, ubLineHeight, 0, uwPosY) * ubLineHeight;

	drawAcceptDecline();
}

static void officeProcessPageAccounting(void) {
	BYTE bPrevPos = s_bOfficeSelectionCurr;
	if(commNavUse(COMM_NAV_DOWN)) {
		if(++s_bOfficeSelectionCurr >= s_ubOfficeSelectionCount) {
			s_bOfficeSelectionCurr = 0;
		}
	}
	else if(commNavUse(COMM_NAV_UP)) {
		if(--s_bOfficeSelectionCurr < 0) {
			s_bOfficeSelectionCurr = s_ubOfficeSelectionCount - 1;
		}
	}

	if(bPrevPos != s_bOfficeSelectionCurr) {
		drawAcceptDecline();
	}

	if(s_isBtnPress) {
		if(s_bOfficeSelectionCurr == 0) {
			if(ubRandMinMax(1, 100) > s_bAccountingChanceFail) {
				warehouseNewPlan(1, g_is2pPlaying);
			}
			else {
				gameAddRebuke();
			}

			s_bAccountingChanceFail = MIN(s_bAccountingChanceFail + 6, 100);
		}
		officeMakePageCurrent(OFFICE_PAGE_MAIN);
	}
}

void officeReduceAccountingChanceFail(void) {
	s_bAccountingChanceFail = MAX(0, s_bAccountingChanceFail - 2);
}

//------------------------------------------------------------ OFFICE PAGE FAVOR

static void officePreparePageFavor(void) {
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	WORD wDays = warehouseGetRemainingDays(warehouseGetPlan());
	if(s_ubFavorsLeft > 0 && wDays >= 25) {

		uwPosY += drawLongText(
			"I like working with you Comrade, I really do."
			" I heard that current plan is tough for you. If you want,"
			" I can make some calls and try to do something about it.",
			ubLineHeight, 0, uwPosY
		) * ubLineHeight;

		uwPosY += ubLineHeight / 2;
		uwPosY += drawLongText("Urz\x84""das can replace current plan with another one.", ubLineHeight, 0, uwPosY) * ubLineHeight;
		char szBfr[100];
		sprintf(szBfr, "You have %hhu favors left.", s_ubFavorsLeft);
		uwPosY += drawLongText(szBfr, ubLineHeight, 0, uwPosY) * ubLineHeight;

		drawAcceptDecline();
	}
	else {
		uwPosY += drawLongText("You ask me for too much, Comrade. Do some real work, will you?", ubLineHeight, 0, uwPosY) * ubLineHeight;
		drawOk();
	}
}

static void officeProcessPageFavor(void) {
	if(s_eControls == OFFICE_CONTROLS_ACCEPT_DECLINE) {
		BYTE bPrevPos = s_bOfficeSelectionCurr;
		if(commNavUse(COMM_NAV_DOWN)) {
			if(++s_bOfficeSelectionCurr >= s_ubOfficeSelectionCount) {
				s_bOfficeSelectionCurr = 0;
			}
		}
		else if(commNavUse(COMM_NAV_UP)) {
			if(--s_bOfficeSelectionCurr < 0) {
				s_bOfficeSelectionCurr = s_ubOfficeSelectionCount - 1;
			}
		}

		if(bPrevPos != s_bOfficeSelectionCurr) {
			drawAcceptDecline();
		}

		if(s_isBtnPress) {
			if(s_bOfficeSelectionCurr == 0) {
				--s_ubFavorsLeft;
				warehouseNewPlan(0, g_is2pPlaying);
			}
			officeMakePageCurrent(OFFICE_PAGE_MAIN);
		}
	}
	else {
		if(s_isBtnPress) {
			officeMakePageCurrent(OFFICE_PAGE_MAIN);
		}
	}
}

//------------------------------------------------------------ OFFICE PAGE BRIBE

static void officePreparePageBribe(void) {
	const UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	const tPlan *pPlan = warehouseGetPlan();
	char szBfr[150];
	UWORD uwPosY = 0;
	UWORD uwCost;

	if(!pPlan->isExtendedTime) {
		if(!pPlan->isPenaltyCountdownStarted) {
			sprintf(szBfr, "Bribe for extra %hhu days for finishing plan in time.", 14);
			uwCost = 100;
			for(UBYTE i = s_ubBribeAccoladeCount; i--;) {
				uwCost = (uwCost * 120 / 100);
			}
		}
		else {
			sprintf(szBfr, "Bribe for extra %hhu days before getting a penalty.", 14);
			uwCost = 200;
			for(UBYTE i = s_ubBribeRebukeCount; i--;) {
				uwCost = (uwCost * 120 / 100);
			}
		}
		uwPosY += drawLongText(szBfr, ubLineHeight, 0, uwPosY) * ubLineHeight;

		uwPosY += ubLineHeight / 2;
		sprintf(
			szBfr, "There is %hhu%% chance that we will get caught, which would result in instantly getting a rebuke.",
			s_bBribeChanceFail
		);
		uwPosY += drawLongText(szBfr,  ubLineHeight, 0, uwPosY) * ubLineHeight;
		sprintf(szBfr, "It will cost you %hu\x1F.", uwCost);
		uwPosY += drawLongText(szBfr,  ubLineHeight, 0, uwPosY) * ubLineHeight;

		drawAcceptDecline();
	}
	else {
		uwPosY += drawLongText("Comrade, not now... there's too much heat!",  ubLineHeight, 0, uwPosY) * ubLineHeight;
		drawOk();
	}
}

static void officeProcessPageBribe(void) {
// #error take cash for a bribe!

	if(s_eControls == OFFICE_CONTROLS_ACCEPT_DECLINE) {
		BYTE bPrevPos = s_bOfficeSelectionCurr;
		if(commNavUse(COMM_NAV_DOWN)) {
			if(++s_bOfficeSelectionCurr >= s_ubOfficeSelectionCount) {
				s_bOfficeSelectionCurr = 0;
			}
		}
		else if(commNavUse(COMM_NAV_UP)) {
			if(--s_bOfficeSelectionCurr < 0) {
				s_bOfficeSelectionCurr = s_ubOfficeSelectionCount - 1;
			}
		}

		if(bPrevPos != s_bOfficeSelectionCurr) {
			drawAcceptDecline();
		}

		if(s_isBtnPress) {
			if(s_bOfficeSelectionCurr == 0) {
				if(ubRandMinMax(1, 100) > s_bBribeChanceFail) {
					// Success
					const tPlan *pPlan = warehouseGetPlan();
					if(!pPlan->isPenaltyCountdownStarted) {
						// accolade bribe
						++s_ubBribeAccoladeCount;
						s_bBribeChanceFail = MIN(s_bBribeChanceFail + 2, 100);
					}
					else {
						// rebuke bribe
						++s_ubBribeRebukeCount;
						s_bBribeChanceFail = MIN(s_bBribeChanceFail + 5, 100);
					}
					warehouseAddDaysToPlan(14, 1);
				}
				else {
					gameAddRebuke();
				}
			}
			officeMakePageCurrent(OFFICE_PAGE_MAIN);
		}
	}
	else {
		if(s_isBtnPress) {
			officeMakePageCurrent(OFFICE_PAGE_MAIN);
		}
	}
}

//------------------------------------------------------------- OFFICE PAGE MAIN

static void officeDrawFaceAtPos(BYTE bPos) {
	const UBYTE ubSpaceX = (COMM_DISPLAY_WIDTH - 2*2 - 4 * 32) / 3;
	const UBYTE ubSpaceY = 10;
	const tUwCoordYX sOrigin = commGetOriginDisplay();

	UWORD uwX = sOrigin.uwX + 2 + (bPos % 4) * (32 + ubSpaceX);
	UWORD uwY = sOrigin.uwY + 2 + (bPos / 4) * (32 + ubSpaceY);

	blitRect(s_pBmDraw, uwX - 2, uwY - 2, 36, 36, COMM_DISPLAY_COLOR_BG);

	// Draw selection
	if(bPos == s_bOfficeSelectionCurr) {
		blitCopy(s_pSelection, 0, 0,  s_pBmDraw, uwX - 2, uwY - 2, 16, 9, MINTERM_COOKIE, 0xFF);
		blitCopy(s_pSelection, 0, 9,  s_pBmDraw, uwX - 2 + 36 - 16, uwY - 2, 16, 9, MINTERM_COOKIE, 0xFF);
		blitCopy(s_pSelection, 0, 18, s_pBmDraw, uwX - 2, uwY - 2 + 36 - 9, 16, 9, MINTERM_COOKIE, 0xFF);
		blitCopy(s_pSelection, 0, 27, s_pBmDraw, uwX - 2 + 36 - 16, uwY - 2 + 36 - 9, 16, 9, MINTERM_COOKIE, 0xFF);
	}

	blitCopy(
		s_pFaces, 0, s_pActivePpl[bPos] * 32, s_pBmDraw, uwX, uwY,
		32, 32, MINTERM_COOKIE, 0xFF
	);
}

static void officePreparePageMain(void) {
	s_eOfficePage = OFFICE_PAGE_MAIN;
	for(tOfficePpl i = 0; i < OFFICE_PPL_COUNT; ++i) {
		if(s_pActivePpl[i] == OFFICE_PPL_COUNT) {
			break;
		}
		officeDrawFaceAtPos(i);
		++s_ubOfficeSelectionCount;
	}
}

static void officeProcessPageMain(void) {
	BYTE bOldSelection = s_bOfficeSelectionCurr;
	if(commNavUse(COMM_NAV_LEFT)) {
		--s_bOfficeSelectionCurr;
	}
	else if(commNavUse(COMM_NAV_RIGHT)) {
		++s_bOfficeSelectionCurr;
	}
	else if(commNavUse(COMM_NAV_DOWN)) {
		s_bOfficeSelectionCurr += 4;
	}
	else if(commNavUse(COMM_NAV_UP)) {
		s_bOfficeSelectionCurr -= 4;
	}
	while(s_bOfficeSelectionCurr < 0) {
		s_bOfficeSelectionCurr += s_ubOfficeSelectionCount;
	}
	while(s_bOfficeSelectionCurr >= s_ubOfficeSelectionCount) {
		s_bOfficeSelectionCurr -= s_ubOfficeSelectionCount;
	}
	if(s_bOfficeSelectionCurr != bOldSelection) {
		officeDrawFaceAtPos(bOldSelection);
		officeDrawFaceAtPos(s_bOfficeSelectionCurr);
	}

	if(s_isBtnPress) {
		officeMakePageCurrent(
			OFFICE_PAGE_LIST_MIETEK + s_pActivePpl[s_bOfficeSelectionCurr]
		);
	}
}

//------------------------------------------------------------- OFFICE PAGE LIST

static void officeDrawListPos(tOfficePage eListPage, UBYTE ubPos) {
	UBYTE ubColor = (
		ubPos == s_bOfficeSelectionCurr ?
		COMM_DISPLAY_COLOR_TEXT :
		COMM_DISPLAY_COLOR_TEXT_DARK
	);
	commDrawText(
		0, 10 * ubPos, g_sOfficePageNames.pStrings[eListPage], FONT_COOKIE, ubColor
	);
}

static void officePreparePageList(tOfficePage ePage) {
	tOfficePpl ePpl = ePage - OFFICE_PAGE_LIST_MIETEK;

	tOfficePage eListPage;
	do {
		eListPage = s_pOfficePages[ePpl][s_ubOfficeSelectionCount];
		officeDrawListPos(eListPage, s_ubOfficeSelectionCount);
		++s_ubOfficeSelectionCount;
	} while(eListPage != OFFICE_PAGE_MAIN);
}

static void officeProcessPageList(const tOfficePage *pPages) {
	BYTE bPrevPos = s_bOfficeSelectionCurr;
	if(commNavUse(COMM_NAV_DOWN)) {
		if(++s_bOfficeSelectionCurr >= s_ubOfficeSelectionCount) {
			s_bOfficeSelectionCurr = 0;
		}
	}
	else if(commNavUse(COMM_NAV_UP)) {
		if(--s_bOfficeSelectionCurr < 0) {
			s_bOfficeSelectionCurr = s_ubOfficeSelectionCount - 1;
		}
	}

	if(bPrevPos != s_bOfficeSelectionCurr) {
		officeDrawListPos(pPages[bPrevPos], bPrevPos);
		officeDrawListPos(pPages[s_bOfficeSelectionCurr], s_bOfficeSelectionCurr);
	}
	else if(s_isBtnPress) {
		officeMakePageCurrent(pPages[s_bOfficeSelectionCurr]);
	}
}

static void commShopProcessOffice(void) {
	switch(s_eOfficePage) {
		case OFFICE_PAGE_LIST_MIETEK:
		case OFFICE_PAGE_LIST_KRYSTYNA:
		case OFFICE_PAGE_LIST_PUTIN:
		case OFFICE_PAGE_LIST_URZEDAS:
			officeProcessPageList(s_pOfficePages[s_eOfficePage - OFFICE_PAGE_LIST_MIETEK]);
			break;
		case OFFICE_PAGE_BRIBE:
			officeProcessPageBribe();
			break;
		case OFFICE_PAGE_FAVOR:
			officeProcessPageFavor();
			break;
		case OFFICE_PAGE_ACCOUNTING:
			officeProcessPageAccounting();
			break;
		case OFFICE_PAGE_MAIN:
		default:
			officeProcessPageMain();
			break;
	}
}

//---------------------------------------------------------- OFFICE PAGE CURRENT

static void officeMakePageCurrent(tOfficePage ePage) {
	logWrite("Make current page: %d\n", ePage);
	commEraseAll();
	s_bOfficeSelectionCurr = 0;
	s_ubOfficeSelectionCount = 0;
	s_eOfficePage = ePage;
	switch(ePage) {
		case OFFICE_PAGE_LIST_MIETEK:
		case OFFICE_PAGE_LIST_KRYSTYNA:
		case OFFICE_PAGE_LIST_PUTIN:
		case OFFICE_PAGE_LIST_URZEDAS:
			officePreparePageList(ePage);
			break;
		case OFFICE_PAGE_BRIBE:
			officePreparePageBribe();
			break;
		case OFFICE_PAGE_FAVOR:
			officePreparePageFavor();
			break;
		case OFFICE_PAGE_ACCOUNTING:
			officePreparePageAccounting();
			break;
		case OFFICE_PAGE_MAIN:
		default:
			officePreparePageMain();
			break;
	}
}

void officeResetPpl(void) {
	for(tOfficePpl i = 0; i < OFFICE_PPL_COUNT; ++i) {
		s_pActivePpl[i] = OFFICE_PPL_COUNT;
	}
	UBYTE ubPos = 0;
	// s_pActivePpl[ubPos++] = OFFICE_PPL_MIETEK;
	s_pActivePpl[ubPos++] = OFFICE_PPL_KRYSTYNA;
	s_pActivePpl[ubPos++] = OFFICE_PPL_URZEDAS;

	// Reset counters
	s_ubFavorsLeft = 10;
	s_bBribeChanceFail = 0;
	s_bAccountingChanceFail = 5;
	s_ubBribeAccoladeCount = 0;
	s_ubBribeRebukeCount = 0;
}

//--------------------------------------------------------------------- WORKSHOP

tStringArray g_sShopNames;
tStringArray g_sShopMsgs;

static UBYTE s_ubWorkshopPos = 0;
static UBYTE s_isOnExitBtn = 0;

static void commShopSelectWorkshopPos(UBYTE ubPart, UBYTE isActive) {
	s_ubWorkshopPos = ubPart;
	static const char szCaption[] = "KRTEK 2600";

	const UBYTE ubRowSize = g_pFont->uwHeight + 2;
	const UBYTE ubFontFlags = FONT_COOKIE | FONT_SHADOW;
	const UBYTE ubColor = (
		isActive ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);

	commErase(0, 0, COMM_DISPLAY_WIDTH, 5 * ubRowSize);
	UWORD uwOffs = 0;
	commDrawText(0, uwOffs, szCaption, ubFontFlags, ubColor);
	uwOffs += ubRowSize;
	commDrawText(0, uwOffs, g_sShopNames.pStrings[ubPart], ubFontFlags, ubColor);
	uwOffs += 2 * ubRowSize;
	char szBfr[50];
	if(ubPart < INVENTORY_PART_COUNT) {
		UBYTE ubLevel = inventoryGetPartDef(s_ubWorkshopPos)->ubLevel;
		sprintf(szBfr, "%s%hhu", g_sShopMsgs.pStrings[SHOP_MSG_MK], ubLevel + 1);
		commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
		if(ubLevel < g_ubUpgradeLevels) {
			uwOffs += ubRowSize;
			sprintf(szBfr, "%s%hhu: %lu\x1F", g_sShopMsgs.pStrings[SHOP_MSG_UPGRADE_TO_MK], ubLevel + 2, g_pUpgradeCosts[ubLevel]);
			commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
		}
	}
	else {
		const tItem *pItem = inventoryGetItemDef(s_ubWorkshopPos - INVENTORY_PART_COUNT);
		sprintf(szBfr, "%s: %hu/%hu", g_sShopMsgs.pStrings[SHOP_MSG_STOCK], pItem->ubCount, pItem->ubMax);
		commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
		uwOffs += ubRowSize;
		sprintf(szBfr, "%s: %hu\x1F", g_sShopMsgs.pStrings[SHOP_MSG_BUY], pItem->uwPrice);
		commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
	}
}

static void commShopDrawWorkshop(void) {
	commShopSelectWorkshopPos(0, 1);

	// Buttons
	UWORD uwBtnX = COMM_DISPLAY_WIDTH / 2;
	UWORD uwBtnY1 = COMM_DISPLAY_HEIGHT - 4 * g_pFont->uwHeight;
	UWORD uwBtnY2 = COMM_DISPLAY_HEIGHT - 2 * g_pFont->uwHeight;
	buttonRmAll();
	buttonAdd(g_sShopMsgs.pStrings[SHOP_MSG_BUY], uwBtnX, uwBtnY1);
	buttonAdd(g_sShopMsgs.pStrings[SHOP_MSG_EXIT], uwBtnX, uwBtnY2);
	buttonSelect(0);
	buttonDrawAll(s_pBmDraw);
	s_isOnExitBtn = 0;
}

static UBYTE commShopWorkshopBuyFor(LONG lCost) {
	if(g_pVehicles[0].lCash >= lCost) {
		g_pVehicles[0].lCash -= lCost;
		hudSetCash(0, g_pVehicles[0].lCash);
		return 1;
	}
	logWrite("commShopWorkshopBuyFor: not enough cash\n");
	// TODO: msg "you don't have enough cash"
	return 0;
}

static UBYTE commShopWorkshopBuyIsFull(UBYTE ubGot, UBYTE ubMax, const char *szMsg) {
	if(ubGot < ubMax) {
		return 0;
	}
	// TODO: msg szMsg
	logWrite("commShopWorkshopBuyIsFull: '%s'\n", szMsg);
	return 1;
}

static void commShopProcessWorkshop(void) {
	if(s_isOnExitBtn) {
		if(s_isBtnPress) {
			// Exit
			gamePopState();
		}
		else if(commNavUse(COMM_NAV_UP)) {
			buttonSelect(0);
			buttonDrawAll(s_pBmDraw);
			s_isOnExitBtn = 0;
			commShopSelectWorkshopPos(s_ubWorkshopPos, 1);
		}
	}
	else {
		if(s_isBtnPress) {
			if(s_ubWorkshopPos < INVENTORY_PART_COUNT) {
				const tPart *pPart = inventoryGetPartDef(s_ubWorkshopPos);
				UBYTE ubLevel = pPart->ubLevel;
				if(!commShopWorkshopBuyIsFull(
					ubLevel, g_ubUpgradeLevels, g_sShopMsgs.pStrings[SHOP_MSG_ALREADY_MAX]
				) && commShopWorkshopBuyFor(g_pUpgradeCosts[ubLevel])) {
					inventorySetPartLevel(s_ubWorkshopPos, ubLevel+1);
					commShopSelectWorkshopPos(s_ubWorkshopPos, 1);
				}
			}
			else {
				tItemName eItemName = s_ubWorkshopPos - INVENTORY_PART_COUNT;
				const tItem *pItem = inventoryGetItemDef(eItemName);
				logWrite("try to buy item %p, %hhu/%hhu\n", pItem, pItem->ubCount, pItem->ubMax);
				// item - TNT, nuke, teleport
				if(!commShopWorkshopBuyIsFull(
					pItem->ubCount, pItem->ubMax, g_sShopMsgs.pStrings[SHOP_MSG_ALREADY_FULL]
				) && commShopWorkshopBuyFor(pItem->uwPrice)) {
					inventorySetItemCount(eItemName, pItem->ubCount + 1);
					commShopSelectWorkshopPos(s_ubWorkshopPos, 1);
					static const tMode pItemToMode[INVENTORY_ITEM_COUNT] = {
						[INVENTORY_ITEM_TNT] = MODE_TNT,
						[INVENTORY_ITEM_NUKE] = MODE_NUKE,
						[INVENTORY_ITEM_TELEPORT] = MODE_TELEPORT
					};
					hudSetModeCounter(pItemToMode[eItemName], pItem->ubCount);
				}
			}
		}
		else if(commNavUse(COMM_NAV_DOWN)) {
			buttonSelect(1);
			buttonDrawAll(s_pBmDraw);
			s_isOnExitBtn = 1;
			commShopSelectWorkshopPos(s_ubWorkshopPos, 0);
		}
		else if(commNavUse(COMM_NAV_RIGHT)) {
			BYTE bNewPos = s_ubWorkshopPos + 1;
			if(bNewPos >= WORKSHOP_ITEM_COUNT) {
				bNewPos = 0;
			}
			commShopSelectWorkshopPos(bNewPos, 1);
		}
		else if(commNavUse(COMM_NAV_LEFT)) {
			BYTE bNewPos = s_ubWorkshopPos - 1;
			if(bNewPos < 0) {
				bNewPos = WORKSHOP_ITEM_COUNT - 1;
			}
			commShopSelectWorkshopPos(bNewPos, 1);
		}
	}
}

//-------------------------------------------------------------------- WAREHOUSE

tStringArray g_sWarehouseColNames;

static UBYTE s_ubPosCurr = 0, s_ubPosCount = 0;
static const UBYTE s_pColOffs[WAREHOUSE_COL_COUNT] = {0,  50, 85, 130};
static UBYTE s_pMineralsOnList[MINERAL_TYPE_COUNT];

static UWORD s_pTmpSell[MINERAL_TYPE_COUNT];
static UWORD s_pTmpPlan[MINERAL_TYPE_COUNT];
static UWORD s_pTmpStock[MINERAL_TYPE_COUNT];

static UBYTE getMineralsOnList(const tPlan *pPlan, UBYTE *pMineralsOnList) {
	UBYTE ubCount = 0;
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		// Omit minerals not in plan
		if(pPlan->pMinerals[i].uwTargetCount || warehouseGetStock(i)) {
			pMineralsOnList[ubCount] = i;
			s_pTmpStock[i] = warehouseGetStock(i);
			s_pTmpPlan[i] = pPlan->pMinerals[i].uwCurrentCount;
			s_pTmpSell[i] = 0;
			++ubCount;
		}
	}
	return ubCount;
}

static void commShopDrawWarehouseRow(UBYTE ubPos, const tPlan *pPlan) {
	UBYTE ubMineral = s_pMineralsOnList[ubPos];
	UBYTE ubColor = (
		ubPos == s_ubPosCurr ?
		COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);

	UWORD uwRowOffsY = 11 + ubPos * 10;

	// Erase
	commErase(0, uwRowOffsY, COMM_DISPLAY_WIDTH, 10);

	// Name
	commDrawText(
		s_pColOffs[0], uwRowOffsY, g_sMineralNames.pStrings[ubMineral],
		FONT_COOKIE | FONT_SHADOW, ubColor
	);

	// Sell
	char szBfr[10];
	UWORD uwMineralReward = s_pTmpSell[ubMineral] * g_pMinerals[ubMineral].ubReward;
	sprintf(szBfr, "%hu\x1F", uwMineralReward);
	commDrawText(s_pColOffs[1], uwRowOffsY, szBfr, FONT_COOKIE | FONT_SHADOW, ubColor);

	// Stock
	UBYTE ubStockCenter = fontMeasureText(g_pFont, g_sWarehouseColNames.pStrings[2]).uwX / 2;
	sprintf(szBfr, "%hu", s_pTmpStock[ubMineral]);
	UBYTE ubValWidthHalf = fontMeasureText(g_pFont, szBfr).uwX / 2;

	if(ubPos == s_ubPosCurr) {
		commDrawText(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf + 3, uwRowOffsY, ">",
			FONT_COOKIE | FONT_SHADOW | FONT_LEFT, ubColor
		);
		commDrawText(
			s_pColOffs[2] + ubStockCenter - ubValWidthHalf - 3, uwRowOffsY, "<",
			FONT_COOKIE | FONT_SHADOW | FONT_RIGHT, ubColor
		);
	}
	else {
		commErase(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf + 3, uwRowOffsY,
			5, g_pFont->uwHeight + 1
		);
		commErase(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf - 3 - 5, uwRowOffsY,
			5, g_pFont->uwHeight + 1
		);
	}
	commDrawText(
		s_pColOffs[2] + ubStockCenter - ubValWidthHalf, uwRowOffsY, szBfr,
		FONT_COOKIE | FONT_SHADOW, ubColor
	);

	// Plan
	sprintf(
		szBfr, "%hu/%hu",
		s_pTmpPlan[ubMineral], pPlan->pMinerals[ubMineral].uwTargetCount
	);
	commDrawText(s_pColOffs[3], uwRowOffsY, szBfr, FONT_COOKIE | FONT_SHADOW, ubColor);
}

static void commShopDrawWarehouse(void) {
	for(UBYTE ubCol = 0; ubCol < 4; ++ubCol) {
		commDrawText(
			s_pColOffs[ubCol], 0, g_sWarehouseColNames.pStrings[ubCol],
			FONT_COOKIE | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
		);
	}

	const tUwCoordYX sPosDisplay = commGetOriginDisplay();
	const UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	blitRect(
		s_pBmDraw, sPosDisplay.uwX, sPosDisplay.uwY + ubLineHeight,
		COMM_DISPLAY_WIDTH, 1, COMM_DISPLAY_COLOR_TEXT
	);

	const tPlan *pPlan = warehouseGetPlan();
	s_ubPosCount = getMineralsOnList(pPlan, s_pMineralsOnList);
	s_ubPosCurr = s_ubPosCount; // move to buttons on start
	for(UBYTE i = 0; i < s_ubPosCount; ++i) {
		commShopDrawWarehouseRow(i, pPlan);
	}

	// Buttons
	UWORD uwBtnX = COMM_DISPLAY_WIDTH / 3;
	UWORD uwBtnY = COMM_DISPLAY_HEIGHT - 4 * ubLineHeight;
	buttonRmAll();
	buttonAdd(g_sShopMsgs.pStrings[SHOP_MSG_CONFIRM], uwBtnX, uwBtnY);
	buttonAdd(g_sShopMsgs.pStrings[SHOP_MSG_EXIT], uwBtnX * 2, uwBtnY);
	buttonSelect(0);
	buttonDrawAll(s_pBmDraw);

	char szBfr[40];

	// Time remaining
	sprintf(
		szBfr, g_sShopMsgs.pStrings[SHOP_MSG_TIME_REMAINING],
		warehouseGetRemainingDays(pPlan)
	);
	commDrawText(
		COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT - ubLineHeight, szBfr,
		FONT_COOKIE | FONT_SHADOW | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT
	);

	// Accolades
	sprintf(
		szBfr, "%s %hhu",
		g_sShopMsgs.pStrings[SHOP_MSG_ACCOLADES], gameGetAccolades()
	);
	commDrawText(
		0, COMM_DISPLAY_HEIGHT - 2 * ubLineHeight, szBfr,
		FONT_COOKIE | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
	);

	// Rebukes
	sprintf(
		szBfr, "%s %hhu", g_sShopMsgs.pStrings[SHOP_MSG_REBUKES], gameGetRebukes()
	);
	commDrawText(
		0, COMM_DISPLAY_HEIGHT - ubLineHeight, szBfr,
		FONT_COOKIE | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
	);
}

static void commShopProcessWarehouse() {
	UBYTE isButtonRefresh = 0;
	UBYTE ubPosPrev = s_ubPosCurr;
	if(commNavUse(COMM_NAV_UP) && s_ubPosCurr) {
		 s_ubPosCurr = MAX(0, s_ubPosCurr - 1);
	}
	else if(commNavUse(COMM_NAV_DOWN) && s_ubPosCurr < s_ubPosCount) {
		s_ubPosCurr = MIN(s_ubPosCount, s_ubPosCurr + 1);
	}

	if(s_ubPosCurr != ubPosPrev) {
		// Deselect previous pos
		if(ubPosPrev < s_ubPosCount) {
			commShopDrawWarehouseRow(ubPosPrev, warehouseGetPlan());
		}
		// Select new pos
		if(s_ubPosCurr < s_ubPosCount) {
			commShopDrawWarehouseRow(s_ubPosCurr, warehouseGetPlan());
			if(ubPosPrev >= s_ubPosCount) {
				buttonSelect(BUTTON_INVALID);
				isButtonRefresh = 1;
			}
		}
		else {
			buttonSelect(0);
			isButtonRefresh = 1;
		}
	}
	else if(s_ubPosCurr < s_ubPosCount) {
		UBYTE ubMineral = s_pMineralsOnList[s_ubPosCurr];
		// Process moving stock
		if(commNavUse(COMM_NAV_LEFT) && s_pTmpStock[ubMineral]) {
			++s_pTmpSell[ubMineral];
			--s_pTmpStock[ubMineral];
			commShopDrawWarehouseRow(ubPosPrev, warehouseGetPlan());
		}
		else if(commNavUse(COMM_NAV_RIGHT) && s_pTmpStock[ubMineral]) {
			++s_pTmpPlan[ubMineral];
			--s_pTmpStock[ubMineral];
			commShopDrawWarehouseRow(ubPosPrev, warehouseGetPlan());
		}
	}
	else {
		// Navigation between buttons
		if(commNavUse(COMM_NAV_RIGHT)) {
			buttonSelect(1);
			isButtonRefresh = 1;
		}
		else if(commNavUse(COMM_NAV_LEFT)) {
			buttonSelect(0);
			isButtonRefresh = 1;
		}
		else if(ubPosPrev < s_ubPosCount) {
			buttonSelect(0);
			isButtonRefresh = 1;
		}
	}
	if(isButtonRefresh) {
		buttonDrawAll(s_pBmDraw);
	}

	// Process button press
	if(s_isBtnPress) {
		switch(buttonGetSelected()) {
			case 0:
				// Confirm
				for(UBYTE i = 0; i < s_ubPosCount; ++i) {
					UBYTE ubMineral = s_pMineralsOnList[i];
					warehouseSetStock(ubMineral, s_pTmpStock[ubMineral]);
					warehouseReserveMineralsForPlan(ubMineral, s_pTmpPlan[ubMineral]);
					g_pVehicles[0].lCash += g_pMinerals[ubMineral].ubReward * s_pTmpSell[ubMineral];
					s_pTmpSell[ubMineral] = 0;
					s_pTmpPlan[ubMineral] = 0;
					s_pTmpStock[ubMineral] = 0;
					hudSetCash(0, g_pVehicles[0].lCash);
				}
				if(warehouseIsPlanFulfilled()) {
					warehouseNewPlan(1, g_is2pPlaying);
					officeReduceAccountingChanceFail();
				}
				commEraseAll();
				commShopDrawWarehouse();
				break;
			case 1:
				// Exit
				gamePopState();
				return;
			default:
				break;
		};
	}
}

static void commShopShowTab(tCommLed eTab) {
	s_eTab = eTab;
	commSetActiveLed(eTab);
	commEraseAll();
	switch(eTab) {
		case COMM_LED_OFFICE:
			officeMakePageCurrent(OFFICE_PAGE_MAIN);
			break;
		case COMM_LED_WORKSHOP:
			commShopDrawWorkshop();
			break;
		case COMM_LED_WAREHOUSE:
			commShopDrawWarehouse();
			break;
		default:
		break;
	}
}

//------------------------------------------------------------------------ ALLOC

void commShopAlloc(void) {
	systemUse();
	s_pFaces = bitmapCreateFromFile("data/comm_faces_office.bm", 0);
	s_pSelection = bitmapCreateFromFile("data/comm_office_selection.bm", 0);
	systemUnuse();
}

void commShopDealloc(void) {
	systemUse();
	bitmapDestroy(s_pFaces);
	bitmapDestroy(s_pSelection);
	systemUnuse();
}

//-------------------------------------------------------------------- GAMESTATE

void commShopGsCreate(void) {
	s_isBtnPress = 0;
	s_isShown = commShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_pBmDraw = g_pMainBuffer->pScroll->pBack;

	s_eTab = COMM_LED_WAREHOUSE;
	commShopShowTab(s_eTab);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void commShopGsLoop(void) {
	static UBYTE isShift = 0;
	static UBYTE wasShiftAction = 0;
	commProcess();

	tutorialProcess();

	tCommLed eOldTab = s_eTab;
	s_isBtnPress = 0;
	if(commNavCheck(COMM_NAV_BTN)) {
		isShift = 1;
	}
	else if(isShift) {
		if(!wasShiftAction) {
			// Btn released and no other pressed in the meantime
			s_isBtnPress = 1;
		}
		isShift = 0;
		wasShiftAction = 0;
	}

	// Tab nav using shift+left / shift+right
	if(isShift) {
		if(commNavUse(COMM_NAV_LEFT)) {
			if(s_eTab) {
				--s_eTab;
			}
			else {
				s_eTab = COMM_LED_COUNT - 1;
			}
			wasShiftAction = 1;
		}
		else if(commNavUse(COMM_NAV_RIGHT)) {
			if(s_eTab < COMM_LED_COUNT - 1) {
				++s_eTab;
			}
			else {
				s_eTab = 0;
			}
			wasShiftAction = 1;
		}
	}

	hudUpdate();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	if(s_eTab != eOldTab) {
		commShopShowTab(s_eTab);
	}
	else {
		switch(s_eTab) {
			case COMM_LED_OFFICE:
				commShopProcessOffice();
				break;
			case COMM_LED_WORKSHOP:
				commShopProcessWorkshop();
				break;
			case COMM_LED_WAREHOUSE:
				commShopProcessWarehouse();
				break;
			default:
				break;
		}
	}
}

void commShopGsDestroy(void) {
	if(!s_isShown) {
		return;
	}

	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}

UBYTE commShopIsActive(void) {
	return s_isShown;
}
