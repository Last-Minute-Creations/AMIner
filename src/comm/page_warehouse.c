#include "page_warehouse.h"
#include "comm/base.h"
#include "comm/button.h"
#include "comm/page_accounting.h"
#include "comm/page_news.h"
#include "../warehouse.h"
#include "../core.h"
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

static void drawRow(UBYTE ubPos, const tPlan *pPlan) {
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
	sprintf(
		szBfr, "%hu/%hu",
		s_pTmpPlan[ubMineral], pPlan->pMinerals[ubMineral].uwTargetCount
	);
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

	const tPlan *pPlan = warehouseGetCurrentPlan();
	s_ubPosCount = getMineralsOnList(pPlan, s_pMineralsOnList);
	s_ubPosCurr = s_ubPosCount; // move to buttons on start
	for(UBYTE i = 0; i < s_ubPosCount; ++i) {
		drawRow(i, pPlan);
	}

	// Buttons
	UWORD uwBtnX = COMM_DISPLAY_WIDTH / 3;
	UWORD uwBtnY = COMM_DISPLAY_HEIGHT - 4 * ubLineHeight;
	buttonRmAll();
	buttonAdd(g_pMsgs[MSG_COMM_CONFIRM], uwBtnX, uwBtnY);
	buttonAdd(g_pMsgs[MSG_COMM_EXIT], uwBtnX * 2, uwBtnY);
	buttonSelect(0);
	buttonDrawAll(pBmDraw);

	char szBfr[40];

	// Time remaining
	sprintf(
		szBfr, g_pMsgs[MSG_COMM_TIME_REMAINING],
		planGetRemainingDays(pPlan)
	);
	commDrawText(
		COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT - ubLineHeight, szBfr,
		FONT_COOKIE | FONT_SHADOW | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT
	);

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
			drawRow(ubPosPrev, warehouseGetCurrentPlan());
		}
		// Select new pos
		if(s_ubPosCurr < s_ubPosCount) {
			drawRow(s_ubPosCurr, warehouseGetCurrentPlan());
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
			drawRow(ubPosPrev, warehouseGetCurrentPlan());
		}
		else if(commNavUse(COMM_NAV_RIGHT) && s_pTmpStock[ubMineral]) {
			++s_pTmpPlan[ubMineral];
			--s_pTmpStock[ubMineral];
			drawRow(ubPosPrev, warehouseGetCurrentPlan());
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
		buttonDrawAll(commGetDisplayBuffer());
	}

	// Process button press
	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
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
				if(planIsFulfilled(warehouseGetCurrentPlan())) {
					warehouseAdvancePlan();
					tEnding eEnding = gameGetEnding();
					if(eEnding) {
						pageNewsCreate(eEnding);
						return;
					}
				}
				commEraseAll();
				redraw();
				break;
			case 1:
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
