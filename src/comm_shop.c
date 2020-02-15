/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comm_shop.h"
#include <ace/managers/game.h>
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

static UBYTE s_isShown;
static UBYTE s_isBtnPress = 0;
tBitMap *s_pBmDraw;
tCommLed s_eTab;

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

//----------------------------------------------------------------------- OFFICE

static void commShopDrawOffice(void) {

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
		szBfr, "%s %d",
		g_sShopMsgs.pStrings[SHOP_MSG_TIME_REMAINING],
		(pPlan->wTimeRemaining + (140-1)) / 140
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
					gameAddAccolade();
					warehouseNewPlan(1, g_is2pPlaying);
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
			commShopDrawOffice();
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
