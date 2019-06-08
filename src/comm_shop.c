/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comm_shop.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include "comm.h"
#include "game.h"
#include "plan.h"
#include "mineral.h"

static UBYTE s_isShown;
static tTextBitMap *s_pTextBitmap;
tBitMap *s_pBmDraw;
tCommLed s_eTab;

static void commShopDrawOffice(void) {
	tUwCoordYX sOrigin = commGetOrigin();
	char *szColNames[3] = {"Mineral", "Required", "Gathered"};
	UBYTE pColOffs[3] = {0, 70, 120};
	UBYTE ubRowOffsY = sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y;
	for(UBYTE i = 0; i < 3; ++i) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szColNames[i]);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[i],
			ubRowOffsY, 14, FONT_COOKIE
		);
	}

	ubRowOffsY += g_pFont->uwHeight + 1;

	blitRect(
		s_pBmDraw, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[0],
		ubRowOffsY, COMM_DISPLAY_WIDTH, 1, 14
	);

	ubRowOffsY += 10 - g_pFont->uwHeight;

	char szBfr[10];
	const tPlan *pPlan = planGetCurrent();
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		if(!pPlan->pMinerals[i].ubTargetCount && !pPlan->pMinerals[i].ubCurrentCount) {
			continue;
		}
		const tMineralDef *pMineralDef = &g_pMinerals[i];
		fontFillTextBitMap(
			g_pFont, s_pTextBitmap, pMineralDef->szName
		);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[0], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);

		sprintf(szBfr, "%hhu", pPlan->pMinerals[i].ubTargetCount);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[1], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);

		sprintf(szBfr, "%hhu", pPlan->pMinerals[i].ubCurrentCount);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[2], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);
		ubRowOffsY += 10;
	}

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Days remaining:");
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH - 20,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - 3*(g_pFont->uwHeight + 1),
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_RIGHT
	);
	sprintf(szBfr, "%d", 0);
	fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - 3*(g_pFont->uwHeight + 1),
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_RIGHT
	);

	if(planIsFulfilled()) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, "Plan fulfilled!");
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH / 2,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - 2*(g_pFont->uwHeight + 1),
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_HCENTER
		);
		planGenerateNew();
	}

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Exit: enter/fire");
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH / 2,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - (g_pFont->uwHeight + 1),
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_HCENTER
	);
}

static void commShopDrawWorkshop(void) {

}

static void commShopDrawWarehouse(void) {
	const UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	tUwCoordYX sOrigin = commGetOrigin();
	char *szColNames[4] = {"Mineral", "Plan", "Stock", "Black market"};
	UBYTE pColOffs[4] = {0, 50, 80, 110};
	UBYTE ubRowOffsY = sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y;
	for(UBYTE ubCol = 0; ubCol < 4; ++ubCol) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szColNames[ubCol]);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[ubCol],
			ubRowOffsY, 14, FONT_COOKIE
		);
	}

	ubRowOffsY += ubLineHeight;

	blitRect(
		s_pBmDraw, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[0],
		ubRowOffsY, COMM_DISPLAY_WIDTH, 1, 14
	);

	ubRowOffsY += 10 - g_pFont->uwHeight;

	char szBfr[30];
	const tPlan *pPlan = planGetCurrent();
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		// Omit minerals not in plan
		if(!pPlan->pMinerals[i].ubTargetCount && !pPlan->pMinerals[i].ubCurrentCount) {
			continue;
		}

		// Name
		const tMineralDef *pMineralDef = &g_pMinerals[i];
		fontFillTextBitMap(
			g_pFont, s_pTextBitmap, pMineralDef->szName
		);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[0], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);

		// Plan
		sprintf(
			szBfr, "%hhu/%hhu",
			pPlan->pMinerals[i].ubCurrentCount, pPlan->pMinerals[i].ubTargetCount
		);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[1], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);

		// Stock
		sprintf(szBfr, "%hhu", 0);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[2], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);

		// Black market
		sprintf(szBfr, "%hhu/%hhu\x1F", 0, 0);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + pColOffs[3], ubRowOffsY,
			COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
		);

		ubRowOffsY += 10;
	}
	ubRowOffsY += 10;

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Or any worth:");
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X, ubRowOffsY,
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
	);
	sprintf(szBfr, "%d/%d\x1F", pPlan->ulCurrentSum, pPlan->ulTargetSum);
	UWORD uwAdd = s_pTextBitmap->uwActualWidth;
	fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + uwAdd + 1, ubRowOffsY,
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
	);

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Days remaining:");
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH - 20,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - 3 * ubLineHeight,
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_RIGHT
	);
	sprintf(szBfr, "%d", 0);
	fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH - 20 + 5,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - 3 * ubLineHeight,
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE
	);

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "L/R: move stock to plan/market");
	fontDrawTextBitMap(
		s_pBmDraw, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + COMM_DISPLAY_X + COMM_DISPLAY_WIDTH / 2,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y + COMM_DISPLAY_HEIGHT - (g_pFont->uwHeight + 1),
		COMM_DISPLAY_COLOR_TEXT, FONT_COOKIE | FONT_HCENTER
	);
}

void commShopAlloc(void) {
	s_pTextBitmap = fontCreateTextBitMap(
		COMM_WIDTH, g_pFont->uwHeight
	);
}

void commShopDealloc(void) {
	fontDestroyTextBitMap(s_pTextBitmap);
}

static void commShopShowTab(tCommLed eTab) {
	s_eTab = eTab;
	commSetActiveLed(eTab);
	tUwCoordYX sOrigin = commGetOrigin();
	blitRect(
		s_pBmDraw, sOrigin.sUwCoord.uwX + COMM_DISPLAY_X,
		sOrigin.sUwCoord.uwY + COMM_DISPLAY_Y,
		COMM_DISPLAY_WIDTH, COMM_DISPLAY_HEIGHT, COMM_DISPLAY_COLOR_BG
	);
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

void commShopGsCreate(void) {
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

	tCommLed eOldTab = s_eTab;
	if(commNavCheck(COMM_NAV_BTN)) {
		isShift = 1;
	}
	else if(isShift) {
		isShift = 0;
		if(!wasShiftAction) {
			// Btn released and no other pressed in the meantime - quit commrade
			gamePopState();
			return;
		}
		else {
			wasShiftAction = 0;
		}
	}

	// Tab nav using shift+left / shift+right
	if(isShift && commNavUse(COMM_NAV_LEFT)) {
		if(s_eTab) {
			--s_eTab;
		}
		else {
			s_eTab = COMM_LED_COUNT-1;
		}
		wasShiftAction = 1;
	}
	else if(isShift && commNavUse(COMM_NAV_RIGHT)) {
		if(s_eTab < COMM_LED_COUNT - 1) {
			++s_eTab;
		}
		else {
			s_eTab = 0;
		}
		wasShiftAction = 1;
	}

	if(s_eTab != eOldTab) {
		commShopShowTab(s_eTab);
	}
	else {
		switch(s_eTab) {
			case COMM_LED_OFFICE:
				break;
			case COMM_LED_WORKSHOP:
				break;
			case COMM_LED_WAREHOUSE:
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
