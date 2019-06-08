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
		// UBYTE ubColor;
		// if(pPlan->pMinerals[i].ubTargetCount) {
		// 	if(pPlan->pMinerals[i].ubCurrentCount >= pPlan->pMinerals[i].ubTargetCount) {
		// 		ubColor = COLOR_GREEN;
		// 	}
		// 	else {
		// 		ubColor = COLOR_RED;
		// 	}
		// }
		// else {
		// 	ubColor = COMM_DISPLAY_COLOR_TEXT;
		// }
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

	s_eTab = COMM_LED_OFFICE;
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
	if(isShift && commNavUse(COMM_NAV_LEFT) && s_eTab) {
		--s_eTab;
		wasShiftAction = 1;
	}
	else if(isShift && commNavUse(COMM_NAV_RIGHT) && s_eTab < COMM_LED_COUNT - 1) {
		++s_eTab;
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
