/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_workshop.h"
#include <comm/comm.h>
#include <comm/button.h>
#include <comm/gs_shop.h>
#include "../string_array.h"
#include "../vehicle.h"
#include "../inventory.h"
#include "../defs.h"
#include "../hud.h"

#define WORKSHOP_PART_ICON_WIDTH 64
#define WORKSHOP_PART_ICON_HEIGHT 56

typedef enum tWorkshopRow {
	WORKSHOP_ROW_BUY,
	WORKSHOP_ROW_EXIT,
	WORKSHOP_ROW_NAV,
} tWorkshopRow;

char **g_pShopNames;

static tPartKind s_eSelectedPart = 0;
static tWorkshopRow s_eWorkshopRow;

static UBYTE pageWorkshopIsPartAcquirable(tPartKind ePart) {
	UBYTE isAcquirable = (
		ePart == INVENTORY_PART_TELEPORT || ePart == INVENTORY_PART_TNT
	);
	return isAcquirable;
}

static void pageWorkshopUpdateText(void) {
	UBYTE isActive = (s_eWorkshopRow == WORKSHOP_ROW_BUY);
	const UBYTE ubRowSize = commGetLineHeight() + 1;
	const UBYTE ubFontFlags = FONT_COOKIE | FONT_SHADOW;
	const UBYTE ubColor = (
		isActive ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);

	static const char szCaption[] = "KRTEK 2600";
	UWORD uwOffsY = 0;
	commDrawText(0, uwOffsY, szCaption, ubFontFlags, ubColor);
	uwOffsY += ubRowSize;

	char szBfr[50];
	UBYTE isAcquirable = pageWorkshopIsPartAcquirable(s_eSelectedPart);
	UBYTE ubLevel = inventoryGetPartDef(s_eSelectedPart)->ubLevel;
	UBYTE ubDisplayLevel = ubLevel + (isAcquirable ? 0 : 1);
	if(!isAcquirable || ubLevel > 0) {
		sprintf(szBfr, "%s %s%hhu", g_pShopNames[s_eSelectedPart], g_pMsgs[MSG_COMM_MK], ubDisplayLevel);
	}
	else {
		strcpy(szBfr, g_pShopNames[s_eSelectedPart]);
	}
	commDrawText(0, uwOffsY, szBfr, ubFontFlags, ubColor);
	uwOffsY += ubRowSize;

	if(ubLevel < g_ubUpgradeLevels) {
		sprintf(szBfr, "%s%hhu: %lu\x1F", g_pMsgs[MSG_COMM_UPGRADE_TO_MK], ubDisplayLevel + 1, g_pUpgradeCosts[ubLevel]);
		commDrawText(0, uwOffsY, szBfr, ubFontFlags, ubColor);
	}
	uwOffsY += 2 * ubRowSize;

	const char *szDescription = 0;
	// TODO: load from json
	if(s_eSelectedPart == INVENTORY_PART_TNT && ubLevel < UPGRADE_LEVEL_COUNT) {
		static const char *pDescriptions[UPGRADE_LEVEL_COUNT] = {
			"Pojedynczy ladunek pozwalajacy na zniszczenie prostej przeszkody terenowej.\nNiszczy surowce zawarte w terenie.",
			"Dwa ladunki pozwalajace drazyc dluzszy tunel lub zniszczyc pojedyncza skale.\nNiszczy surowce zawarte w terenie.",
			"Trzy ladunki jeszcze bardziej zwieksza Twoj zasieg.\nNiszczy surowce zawarte w terenie.",
			"Ulepszona formula materialu wybuchowego pozwoli zachowac surowce w detonowanym terenie."
		};
		szDescription = pDescriptions[ubLevel];
	}

	if(szDescription) {
		commDrawMultilineText(szDescription, 0, uwOffsY);
	}

	tUwCoordYX sButtonBuyPosition = buttonGetPosition(0);
	UBYTE ubArrowColor = (buttonGetSelected() == 0) ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK;
	commDrawText(sButtonBuyPosition.uwX - 4, sButtonBuyPosition.uwY + 2, "<", FONT_RIGHT | FONT_VCENTER | FONT_COOKIE, ubArrowColor);
	commDrawText(sButtonBuyPosition.uwX + 5 + buttonGetWidth(0), sButtonBuyPosition.uwY + 2, ">", FONT_LEFT | FONT_VCENTER | FONT_COOKIE, ubArrowColor);

	buttonDrawAll(commGetDisplayBuffer());
}

static void pageWorkshopNavigateToPart(tPartKind ePart) {
	s_eSelectedPart = ePart;

	commEraseAll();
	tUwCoordYX sOrigin = commGetOriginDisplay();
	blitCopy(
		g_pCommWorkshopIcons, 0, WORKSHOP_PART_ICON_HEIGHT * ePart,
		commGetDisplayBuffer(),
		sOrigin.uwX + COMM_DISPLAY_WIDTH - WORKSHOP_PART_ICON_WIDTH, sOrigin.uwY,
		WORKSHOP_PART_ICON_WIDTH, WORKSHOP_PART_ICON_HEIGHT, MINTERM_COOKIE
	);

	pageWorkshopUpdateText();
}

static UBYTE pageWorkshopBuyFor(LONG lCost) {
	if(g_pVehicles[0].lCash >= lCost) {
		g_pVehicles[0].lCash -= lCost;
		hudSetCash(0, g_pVehicles[0].lCash);
		return 1;
	}
	logWrite("pageWorkshopBuyFor: not enough cash\n");
	// TODO: msg "you don't have enough cash"
	return 0;
}

static UBYTE pageWorkshopBuyIsFull(UBYTE ubGot, UBYTE ubMax, const char *szMsg) {
	if(ubGot < ubMax) {
		return 0;
	}
	// TODO: msg szMsg
	logWrite("pageWorkshopBuyIsFull: '%s'\n", szMsg);
	return 1;
}

static void pageWorkshopProcess(void) {
	tWorkshopRow ePrevRow = s_eWorkshopRow;

	if(commNavUse(DIRECTION_UP) || commShopGetTabNavigationState() == TAB_NAVIGATION_STATE_DISABLING) {
		if(s_eWorkshopRow > WORKSHOP_ROW_BUY) {
			--s_eWorkshopRow;
		}
	}
	if(commNavUse(DIRECTION_DOWN)) {
		if(s_eWorkshopRow < WORKSHOP_ROW_NAV) {
			++s_eWorkshopRow;
		}
	}

	if(ePrevRow != s_eWorkshopRow) {
		if(s_eWorkshopRow == WORKSHOP_ROW_NAV) {
			commShopFocusOnTabNavigation();
			buttonDeselectAll();
		}
		else {
			buttonSelect(s_eWorkshopRow);
		}
		pageWorkshopUpdateText();
	}

	if(s_eWorkshopRow == WORKSHOP_ROW_BUY) {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			if(s_eSelectedPart < INVENTORY_PART_COUNT) {
				const tPartDef *pPart = inventoryGetPartDef(s_eSelectedPart);
				UBYTE ubLevel = pPart->ubLevel;
				if(!pageWorkshopBuyIsFull(
					ubLevel, g_ubUpgradeLevels, g_pMsgs[MSG_COMM_ALREADY_MAX]
				) && pageWorkshopBuyFor(g_pUpgradeCosts[ubLevel])) {
					inventorySetPartLevel(s_eSelectedPart, ubLevel+1);
					// text have changed, so draw everything again
					pageWorkshopNavigateToPart(s_eSelectedPart);
				}
			}
		}
		else if(commNavUse(DIRECTION_RIGHT)) {
			BYTE bNewPos = s_eSelectedPart + 1;
			if(bNewPos >= WORKSHOP_ITEM_COUNT) {
				bNewPos = 0;
			}
			pageWorkshopNavigateToPart(bNewPos);
		}
		else if(commNavUse(DIRECTION_LEFT)) {
			BYTE bNewPos = s_eSelectedPart - 1;
			if(bNewPos < 0) {
				bNewPos = WORKSHOP_ITEM_COUNT - 1;
			}
			pageWorkshopNavigateToPart(bNewPos);
		}
	}
	else if(s_eWorkshopRow == WORKSHOP_ROW_EXIT) {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			// Exit
			commRegisterPage(0, 0);
		}
	}
}

void pageWorkshopCreate(void) {
	commRegisterPage(pageWorkshopProcess, 0);

	// Buttons
	UWORD uwBtnY = COMM_DISPLAY_HEIGHT - 2 * (buttonGetHeight() + 2);
	buttonReset(BUTTON_LAYOUT_VERTICAL, uwBtnY);
	buttonAdd(g_pMsgs[MSG_COMM_BUY]);
	buttonAdd(g_pMsgs[MSG_COMM_EXIT]);
	if(commShopGetTabNavigationState() == TAB_NAVIGATION_STATE_ENABLED) {
		buttonDeselectAll();
		s_eWorkshopRow = WORKSHOP_ROW_NAV;
	}
	else {
		buttonSelect(0);
		s_eWorkshopRow = WORKSHOP_ROW_BUY;
	}
	buttonRowApply();

	pageWorkshopNavigateToPart(0);
}
