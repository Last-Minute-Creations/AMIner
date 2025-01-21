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

static UBYTE workshopIsPartAcquirable(tPartKind ePart) {
	UBYTE isAcquirable = (
		ePart == INVENTORY_PART_TELEPORT || ePart == INVENTORY_PART_TNT
	);
	return isAcquirable;
}

static void commShopSelectWorkshopPart(tPartKind ePart, UBYTE isActive) {
	s_eSelectedPart = ePart;
	static const char szCaption[] = "KRTEK 2600";

	const UBYTE ubRowSize = commGetLineHeight() + 1;
	const UBYTE ubFontFlags = FONT_COOKIE | FONT_SHADOW;
	const UBYTE ubColor = (
		isActive ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);

	commEraseAll();
	UWORD uwOffsY = 0;
	commDrawText(0, uwOffsY, szCaption, ubFontFlags, ubColor);
	uwOffsY += ubRowSize;

	tUwCoordYX sOrigin = commGetOriginDisplay();
	blitCopy(
		g_pCommWorkshopIcons, 0, WORKSHOP_PART_ICON_HEIGHT * ePart,
		commGetDisplayBuffer(),
		sOrigin.uwX + COMM_DISPLAY_WIDTH - WORKSHOP_PART_ICON_WIDTH, sOrigin.uwY,
		WORKSHOP_PART_ICON_WIDTH, WORKSHOP_PART_ICON_HEIGHT, MINTERM_COOKIE
	);

	char szBfr[50];
	UBYTE isAcquirable = workshopIsPartAcquirable(s_eSelectedPart);
	UBYTE ubLevel = inventoryGetPartDef(s_eSelectedPart)->ubLevel;
	UBYTE ubDisplayLevel = ubLevel + (isAcquirable ? 0 : 1);
	if(!isAcquirable || ubLevel > 0) {
		sprintf(szBfr, "%s %s%hhu", g_pShopNames[ePart], g_pMsgs[MSG_COMM_MK], ubDisplayLevel);
	}
	else {
		strcpy(szBfr, g_pShopNames[ePart]);
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

	buttonDrawAll(commGetDisplayBuffer());
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
		commShopSelectWorkshopPart(s_eSelectedPart, s_eWorkshopRow == WORKSHOP_ROW_BUY);
	}

	if(s_eWorkshopRow == WORKSHOP_ROW_BUY) {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			if(s_eSelectedPart < INVENTORY_PART_COUNT) {
				const tPartDef *pPart = inventoryGetPartDef(s_eSelectedPart);
				UBYTE ubLevel = pPart->ubLevel;
				if(!commShopWorkshopBuyIsFull(
					ubLevel, g_ubUpgradeLevels, g_pMsgs[MSG_COMM_ALREADY_MAX]
				) && commShopWorkshopBuyFor(g_pUpgradeCosts[ubLevel])) {
					inventorySetPartLevel(s_eSelectedPart, ubLevel+1);
					commShopSelectWorkshopPart(s_eSelectedPart, 1);
				}
			}
		}
		else if(commNavUse(DIRECTION_RIGHT)) {
			BYTE bNewPos = s_eSelectedPart + 1;
			if(bNewPos >= WORKSHOP_ITEM_COUNT) {
				bNewPos = 0;
			}
			commShopSelectWorkshopPart(bNewPos, 1);
		}
		else if(commNavUse(DIRECTION_LEFT)) {
			BYTE bNewPos = s_eSelectedPart - 1;
			if(bNewPos < 0) {
				bNewPos = WORKSHOP_ITEM_COUNT - 1;
			}
			commShopSelectWorkshopPart(bNewPos, 1);
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

	commShopSelectWorkshopPart(0, 1);
}
