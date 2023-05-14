/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_workshop.h"
#include <comm/comm.h>
#include <comm/button.h>
#include "../string_array.h"
#include "../vehicle.h"
#include "../inventory.h"
#include "../defs.h"
#include "../hud.h"

char **g_pShopNames;

static UBYTE s_ubWorkshopPos = 0;
static UBYTE s_isOnExitBtn = 0;

static void commShopSelectWorkshopPos(UBYTE ubPart, UBYTE isActive) {
	s_ubWorkshopPos = ubPart;
	static const char szCaption[] = "KRTEK 2600";

	const UBYTE ubRowSize = commGetLineHeight() + 1;
	const UBYTE ubFontFlags = FONT_COOKIE | FONT_SHADOW;
	const UBYTE ubColor = (
		isActive ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);

	commErase(0, 0, COMM_DISPLAY_WIDTH, 5 * ubRowSize);
	UWORD uwOffs = 0;
	commDrawText(0, uwOffs, szCaption, ubFontFlags, ubColor);
	uwOffs += ubRowSize;
	commDrawText(0, uwOffs, g_pShopNames[ubPart], ubFontFlags, ubColor);
	uwOffs += 2 * ubRowSize;
	char szBfr[50];
	if(ubPart < INVENTORY_PART_COUNT) {
		UBYTE ubLevel = inventoryGetPartDef(s_ubWorkshopPos)->ubLevel;
		sprintf(szBfr, "%s%hhu", g_pMsgs[MSG_COMM_MK], ubLevel + 1);
		commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
		if(ubLevel < g_ubUpgradeLevels) {
			uwOffs += ubRowSize;
			sprintf(szBfr, "%s%hhu: %lu\x1F", g_pMsgs[MSG_COMM_UPGRADE_TO_MK], ubLevel + 2, g_pUpgradeCosts[ubLevel]);
			commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
		}
	}
	else {
		const tItem *pItem = inventoryGetItemDef(s_ubWorkshopPos - INVENTORY_PART_COUNT);
		sprintf(szBfr, "%s: %hu/%hu", g_pMsgs[MSG_COMM_STOCK], pItem->ubCount, pItem->ubMax);
		commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
		uwOffs += ubRowSize;
		sprintf(szBfr, "%s: %hu\x1F", g_pMsgs[MSG_COMM_BUY], pItem->uwPrice);
		commDrawText(0, uwOffs, szBfr, ubFontFlags, ubColor);
	}
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
	if(s_isOnExitBtn) {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			// Exit
			commRegisterPage(0, 0);
		}
		else if(commNavUse(DIRECTION_UP)) {
			buttonSelect(0);
			buttonDrawAll(commGetDisplayBuffer());
			s_isOnExitBtn = 0;
			commShopSelectWorkshopPos(s_ubWorkshopPos, 1);
		}
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			if(s_ubWorkshopPos < INVENTORY_PART_COUNT) {
				const tPart *pPart = inventoryGetPartDef(s_ubWorkshopPos);
				UBYTE ubLevel = pPart->ubLevel;
				if(!commShopWorkshopBuyIsFull(
					ubLevel, g_ubUpgradeLevels, g_pMsgs[MSG_COMM_ALREADY_MAX]
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
					pItem->ubCount, pItem->ubMax, g_pMsgs[MSG_COMM_ALREADY_FULL]
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
		else if(commNavUse(DIRECTION_DOWN)) {
			buttonSelect(1);
			buttonDrawAll(commGetDisplayBuffer());
			s_isOnExitBtn = 1;
			commShopSelectWorkshopPos(s_ubWorkshopPos, 0);
		}
		else if(commNavUse(DIRECTION_RIGHT)) {
			BYTE bNewPos = s_ubWorkshopPos + 1;
			if(bNewPos >= WORKSHOP_ITEM_COUNT) {
				bNewPos = 0;
			}
			commShopSelectWorkshopPos(bNewPos, 1);
		}
		else if(commNavUse(DIRECTION_LEFT)) {
			BYTE bNewPos = s_ubWorkshopPos - 1;
			if(bNewPos < 0) {
				bNewPos = WORKSHOP_ITEM_COUNT - 1;
			}
			commShopSelectWorkshopPos(bNewPos, 1);
		}
	}
}

void pageWorkshopCreate(void) {
	commRegisterPage(pageWorkshopProcess, 0);
	commShopSelectWorkshopPos(0, 1);

	// Buttons
	UWORD uwBtnX = COMM_DISPLAY_WIDTH / 2;
	UWORD uwBtnY1 = COMM_DISPLAY_HEIGHT - 3 * buttonGetHeight();
	UWORD uwBtnY2 = COMM_DISPLAY_HEIGHT - 1 * buttonGetHeight();
	buttonRmAll();
	buttonAdd(g_pMsgs[MSG_COMM_BUY], uwBtnX, uwBtnY1);
	buttonAdd(g_pMsgs[MSG_COMM_EXIT], uwBtnX, uwBtnY2);
	buttonSelect(0);
	buttonDrawAll(commGetDisplayBuffer());
	s_isOnExitBtn = 0;
}
