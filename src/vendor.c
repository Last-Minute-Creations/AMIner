/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vendor.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include "window.h"
#include "game.h"
#include "plan.h"
#include "mineral.h"
#include "color.h"

static UBYTE s_isShown;
static tTextBitMap *s_pTextBitmap;
tBitMap *s_pBuffer;

void vendorDrawPlan(void) {
	tUwCoordYX sOrigin = windowGetOrigin();
	char *szColNames[3] = {"Mineral", "Required", "Gathered"};
	UBYTE pColOffs[3] = {16, 120, 200};
	UBYTE ubRowOffsY = sOrigin.sUwCoord.uwY + 16;
	for(UBYTE i = 0; i < 3; ++i) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szColNames[i]);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + pColOffs[i], sOrigin.sUwCoord.uwY + 16,
			14, FONT_COOKIE
		);
	}

	ubRowOffsY += g_pFont->uwHeight + 1;

	blitRect(
		s_pBuffer, sOrigin.sUwCoord.uwX + pColOffs[0], ubRowOffsY,
		WINDOW_WIDTH - 2 * pColOffs[0], 1, 14
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
			s_pBuffer, s_pTextBitmap, sOrigin.sUwCoord.uwX + pColOffs[0], ubRowOffsY,
			pMineralDef->ubTitleColor, FONT_COOKIE
		);

		sprintf(szBfr, "%hhu", pPlan->pMinerals[i].ubTargetCount);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap, sOrigin.sUwCoord.uwX + pColOffs[1], ubRowOffsY,
			COLOR_GOLD, FONT_COOKIE
		);

		sprintf(szBfr, "%hhu", pPlan->pMinerals[i].ubCurrentCount);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		UBYTE ubColor;
		if(pPlan->pMinerals[i].ubTargetCount) {
			if(pPlan->pMinerals[i].ubCurrentCount >= pPlan->pMinerals[i].ubTargetCount) {
				ubColor = COLOR_GREEN;
			}
			else {
				ubColor = COLOR_RED;
			}
		}
		else {
			ubColor = COLOR_GOLD;
		}
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap, sOrigin.sUwCoord.uwX + pColOffs[2], ubRowOffsY,
			ubColor, FONT_COOKIE
		);
		ubRowOffsY += 10;
	}

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Days remaining:");
	fontDrawTextBitMap(
		s_pBuffer, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + 150, sOrigin.sUwCoord.uwY + 160,
		COLOR_GOLD, FONT_COOKIE
	);
	sprintf(szBfr, "%d", 0);
	fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
	fontDrawTextBitMap(
		s_pBuffer, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + 240, sOrigin.sUwCoord.uwY + 160,
		COLOR_GOLD, FONT_COOKIE | FONT_RIGHT
	);

	if(planIsFulfilled()) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, "Plan fulfilled!");
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + WINDOW_WIDTH/2,
			sOrigin.sUwCoord.uwY + WINDOW_HEIGHT - 20,
			COLOR_GREEN, FONT_COOKIE | FONT_CENTER
		);
		planGenerateNew();
	}

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Exit: enter/fire");
	fontDrawTextBitMap(
		s_pBuffer, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + WINDOW_WIDTH/2,
		sOrigin.sUwCoord.uwY + WINDOW_HEIGHT - 10,
		COLOR_GOLD, FONT_COOKIE | FONT_CENTER
	);
}

void vendorAlloc(void) {
	s_pTextBitmap = fontCreateTextBitMap(
		WINDOW_WIDTH, g_pFont->uwHeight
	);
}

void vendorDealloc(void) {
	fontDestroyTextBitMap(s_pTextBitmap);
}

void vendorGsCreate(void) {
	s_isShown = windowShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_pBuffer = g_pMainBuffer->pScroll->pBack;

	vendorDrawPlan();

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void vendorGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}
	if(keyUse(KEY_RETURN) || keyUse(KEY_SPACE)) {
		gamePopState();
		return;
	}
}

void vendorGsDestroy(void) {
	if(!s_isShown) {
		return;
	}
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	windowHide();
}
