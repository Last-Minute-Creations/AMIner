#include "page_warehouse.h"
#include <ace/utils/string.h>
#include <ace/managers/key.h>
#include <comm/comm.h>
#include <comm/button.h>
#include <comm/page_accounting.h>
#include <comm/page_news.h>
#include <comm/page_office.h>
#include <comm/inbox.h>
#include "../warehouse.h"
#include "../assets.h"
#include "../defs.h"
#include "../hud.h"
#include "../game.h"
#include "../vehicle.h"

#define WAREHOUSE_COL_COUNT 4

char **g_pWarehouseColNames;

static UBYTE s_ubPosCurr = 0, s_ubPosCount = 0;
static const UBYTE s_pColOffs[WAREHOUSE_COL_COUNT] = {0,  50, 85, 130};
static UBYTE s_pMineralsOnList[MINERAL_TYPE_COUNT];

static UWORD s_pTmpSell[MINERAL_TYPE_COUNT];
static UWORD s_pTmpPlan[MINERAL_TYPE_COUNT];
static UWORD s_pTmpStock[MINERAL_TYPE_COUNT];
static UBYTE s_ubButtonCurrent;

static UBYTE getMineralsOnList(UBYTE *pMineralsOnList) {
	UBYTE ubCount = 0;
	tPlan *pPlan = planGetCurrent();
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		// Omit minerals not in plan
		if(pPlan->pMineralsRequired[i] || warehouseGetStock(i)) {
			pMineralsOnList[ubCount] = i;
			s_pTmpStock[i] = warehouseGetStock(i);
			s_pTmpPlan[i] = planManagerGet()->pMineralsSpent[i];
			s_pTmpSell[i] = 0;
			++ubCount;
		}
	}
	return ubCount;
}

static void drawRow(UBYTE ubPos) {
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
		s_pColOffs[0], uwRowOffsY, g_pMineralNames[ubMineral],
		FONT_COOKIE | FONT_SHADOW, ubColor
	);

	// Sell
	char szBfr[10];
	UWORD uwMineralReward = s_pTmpSell[ubMineral] * g_pMinerals[ubMineral].ubReward;
	sprintf(szBfr, "%hu\x1F", uwMineralReward);
	commDrawText(s_pColOffs[1], uwRowOffsY, szBfr, FONT_COOKIE | FONT_SHADOW, ubColor);

	// Stock
	UBYTE ubStockCenter = fontMeasureText(g_pFont, g_pWarehouseColNames[2]).uwX / 2;
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
			5, commGetLineHeight()
		);
		commErase(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf - 3 - 5, uwRowOffsY,
			5, commGetLineHeight()
		);
	}
	commDrawText(
		s_pColOffs[2] + ubStockCenter - ubValWidthHalf, uwRowOffsY, szBfr,
		FONT_COOKIE | FONT_SHADOW, ubColor
	);

	// Plan
	if(planManagerGet()->isPlanActive) {
		sprintf(
			szBfr, "%hu/%hu",
			s_pTmpPlan[ubMineral], planGetCurrent()->pMineralsRequired[ubMineral]
		);
	}
	else {
		stringCopy("-", szBfr);
	}
	commDrawText(s_pColOffs[3], uwRowOffsY, szBfr, FONT_COOKIE | FONT_SHADOW, ubColor);
}

static void redraw(void) {
	for(UBYTE ubCol = 0; ubCol < 4; ++ubCol) {
		commDrawText(
			s_pColOffs[ubCol], 0, g_pWarehouseColNames[ubCol],
			FONT_COOKIE | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
		);
	}

	tBitMap *pBmDraw = commGetDisplayBuffer();
	const tUwCoordYX sPosDisplay = commGetOriginDisplay();
	const UBYTE ubLineHeight = commGetLineHeight();
	blitRect(
		pBmDraw, sPosDisplay.uwX, sPosDisplay.uwY + ubLineHeight,
		COMM_DISPLAY_WIDTH, 1, COMM_DISPLAY_COLOR_TEXT
	);

	s_ubPosCount = getMineralsOnList(s_pMineralsOnList);
	s_ubPosCurr = s_ubPosCount; // move to buttons on start
	for(UBYTE i = 0; i < s_ubPosCount; ++i) {
		drawRow(i);
	}

	// Buttons
	UWORD uwBtnY = COMM_DISPLAY_HEIGHT - 2 * ubLineHeight - buttonGetHeight() + 2;
	s_ubButtonCurrent = 0;
	buttonReset(BUTTON_LAYOUT_HORIZONTAL, uwBtnY);
	buttonAdd(g_pMsgs[MSG_COMM_CONFIRM]);
	buttonAdd(g_pMsgs[MSG_COMM_MARKET]);
	buttonAdd(g_pMsgs[MSG_COMM_EXIT]);
	buttonSelect(s_ubButtonCurrent);
	buttonRowApply();
	buttonDrawAll(pBmDraw);

	char szBfr[40];

	// Time remaining
	if(planManagerGet()->isPlanActive) {
		sprintf(
			szBfr, g_pMsgs[MSG_COMM_TIME_REMAINING],
			planGetRemainingDays()
		);
		commDrawText(
			COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT - ubLineHeight, szBfr,
			FONT_COOKIE | FONT_SHADOW | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT
		);
	}

	// Accolades
	sprintf(
		szBfr, "%s %hhu",
		g_pMsgs[MSG_COMM_ACCOLADES], gameGetAccolades()
	);
	commDrawText(
		0, COMM_DISPLAY_HEIGHT - 2 * ubLineHeight, szBfr,
		FONT_COOKIE | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
	);

	// Rebukes
	sprintf(
		szBfr, "%s %hhu", g_pMsgs[MSG_COMM_REBUKES], gameGetRebukes()
	);
	commDrawText(
		0, COMM_DISPLAY_HEIGHT - ubLineHeight, szBfr,
		FONT_COOKIE | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
	);
}

static void pageWarehouseProcess(void) {
	if(keyUse(KEY_H)) {
		commShopChangePage(COMM_SHOP_PAGE_WAREHOUSE, COMM_SHOP_PAGE_SOKOBAN);
		return;
	}

	UBYTE isButtonRefresh = 0;
	UBYTE ubPosPrev = s_ubPosCurr;
	if(commNavUse(DIRECTION_UP) && s_ubPosCurr) {
		 s_ubPosCurr = MAX(0, s_ubPosCurr - 1);
	}
	else if(commNavUse(DIRECTION_DOWN) && s_ubPosCurr < s_ubPosCount) {
		s_ubPosCurr = MIN(s_ubPosCount, s_ubPosCurr + 1);
	}

	if(s_ubPosCurr != ubPosPrev) {
		// Deselect previous pos
		if(ubPosPrev < s_ubPosCount) {
			drawRow(ubPosPrev);
		}
		// Select new pos
		if(s_ubPosCurr < s_ubPosCount) {
			drawRow(s_ubPosCurr);
			if(ubPosPrev >= s_ubPosCount) {
				buttonDeselectAll();
				isButtonRefresh = 1;
			}
		}
		else {
			s_ubButtonCurrent = 0;
			buttonSelect(s_ubButtonCurrent);
			isButtonRefresh = 1;
		}
	}
	else if(s_ubPosCurr < s_ubPosCount) {
		UBYTE ubMineral = s_pMineralsOnList[s_ubPosCurr];
		// Process moving stock
		if(commNavUse(DIRECTION_LEFT) && s_pTmpStock[ubMineral]) {
			++s_pTmpSell[ubMineral];
			--s_pTmpStock[ubMineral];
			drawRow(ubPosPrev);
		}
		else if(
			commNavUse(DIRECTION_RIGHT) && s_pTmpStock[ubMineral] &&
			planManagerGet()->isPlanActive
		) {
			++s_pTmpPlan[ubMineral];
			--s_pTmpStock[ubMineral];
			drawRow(ubPosPrev);
		}
	}
	else {
		// Navigation between buttons
		if(commNavUse(DIRECTION_RIGHT)) {
			if(s_ubButtonCurrent < 2) {
				++s_ubButtonCurrent;
				buttonSelect(s_ubButtonCurrent);
				isButtonRefresh = 1;
			}
		}
		else if(commNavUse(DIRECTION_LEFT)) {
			if(s_ubButtonCurrent > 0) {
				--s_ubButtonCurrent;
				buttonSelect(s_ubButtonCurrent);
				isButtonRefresh = 1;
			}
		}
		else if(ubPosPrev < s_ubPosCount) {
			s_ubButtonCurrent = 0;
			buttonSelect(s_ubButtonCurrent);
			isButtonRefresh = 1;
		}
	}
	if(isButtonRefresh) {
		buttonDrawAll(commGetDisplayBuffer());
	}

	// Process button press
	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		switch(buttonGetSelected()) {
			case 0: {
				// Confirm
				for(UBYTE i = 0; i < s_ubPosCount; ++i) {
					UBYTE ubMineral = s_pMineralsOnList[i];
					warehouseSetStock(ubMineral, s_pTmpStock[ubMineral]);
					planSpendMinerals(ubMineral, s_pTmpPlan[ubMineral]);
					g_pVehicles[0].lCash += g_pMinerals[ubMineral].ubReward * s_pTmpSell[ubMineral];
					s_pTmpSell[ubMineral] = 0;
					s_pTmpPlan[ubMineral] = 0;
					s_pTmpStock[ubMineral] = 0;
					hudSetCash(0, g_pVehicles[0].lCash);
				}

				if(planIsCurrentFulfilled()) {
					UBYTE wasDelayed = (planManagerGet()->eProlongState == PLAN_PROLONG_CURRENT);
					warehouseNextPlan(NEXT_PLAN_REASON_FULFILLED);

					if(wasDelayed) {
						pageOfficeTryUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_DELAYED);
						inboxPushBack(COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_DELAYED, 0);
					}
				}
				commEraseAll();
				redraw();
			} break;
			case 1:
				// Market
				commShopChangePage(COMM_SHOP_PAGE_WAREHOUSE, COMM_SHOP_PAGE_MARKET);
				return;
			case 2:
				// Exit
				commRegisterPage(0, 0);
				return;
			default:
				break;
		};
	}
}

void pageWarehouseCreate(void) {
	commRegisterPage(pageWarehouseProcess, 0);
	redraw();
}
