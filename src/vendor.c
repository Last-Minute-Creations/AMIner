/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vendor.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include "window.h"
#include "game.h"
#include "tile.h"

static UBYTE s_isShown;
static tTextBitMap *s_pTextBitmap;
tBitMap *s_pBuffer;

typedef struct _tPlanMineral {
	UBYTE ubMineralType;
	UBYTE ubCount;
} tPlanMineral;

typedef struct _tPlan {
	tPlanMineral pMinerals[4];
	UBYTE ubMineralCount;
	ULONG ulAlternativeSum;
} tPlan;

static const char *s_pMineralNames[] = {0,
	[TILE_SILVER_1] = "Silver",
	[TILE_GOLD_1] = "Gold",
	[TILE_EMERALD_1] = "Emerald",
	[TILE_RUBY_1] = "Ruby",
	[TILE_MOONSTONE_1] = "Moonstone",
	[TILE_COAL_1] = "Coal"
};

void vendorDrawPlan(void) {
	tUwCoordYX sOrigin = windowGetOrigin();
	char *szColNames[3] = {"Mineral", "Required", "Gathered"};
	uint8_t pColOffs[3] = {16, 120, 200};
	UBYTE ubRowOffsY = sOrigin.sUwCoord.uwY + 16;
	for(UBYTE i = 0; i < 3; ++i) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szColNames[i]);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + pColOffs[i], sOrigin.sUwCoord.uwY + 16,
			14, FONT_COOKIE
		);
	}

	tPlan sPlan = {
		.pMinerals = {
			{TILE_SILVER_1, 10},
			{TILE_GOLD_1, 5}
		},
		.ubMineralCount = 2,
		.ulAlternativeSum = 100000
	};

	ubRowOffsY += g_pFont->uwHeight + 1;

	blitRect(
		s_pBuffer, sOrigin.sUwCoord.uwX + pColOffs[0], ubRowOffsY,
		WINDOW_WIDTH - 2 * pColOffs[0], 1, 14
	);

	ubRowOffsY += 10 - g_pFont->uwHeight;

	char szBfr[10];
	for(UBYTE i = 0; i < sPlan.ubMineralCount; ++i) {
		fontFillTextBitMap(
			g_pFont, s_pTextBitmap, s_pMineralNames[sPlan.pMinerals[i].ubMineralType]
		);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + pColOffs[0], ubRowOffsY, 14, FONT_COOKIE
		);

		sprintf(szBfr, "%hhu", sPlan.pMinerals[i].ubCount);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + pColOffs[1], ubRowOffsY, 14, FONT_COOKIE
		);

		sprintf(szBfr, "%hhu", 0);
		fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap,
			sOrigin.sUwCoord.uwX + pColOffs[2], ubRowOffsY, 14, FONT_COOKIE
		);
		ubRowOffsY += 10;
	}

	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Days remaining:");
	fontDrawTextBitMap(
		s_pBuffer, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + 150, sOrigin.sUwCoord.uwY + 160, 14, FONT_COOKIE
	);
	sprintf(szBfr, "%d", 0);
	fontFillTextBitMap(g_pFont, s_pTextBitmap, szBfr);
	fontDrawTextBitMap(
		s_pBuffer, s_pTextBitmap,
		sOrigin.sUwCoord.uwX + 240, sOrigin.sUwCoord.uwY + 160,
		14, FONT_COOKIE | FONT_RIGHT
	);

}

void vendorGsCreate(void) {
	s_isShown = windowShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_pBuffer = g_pMainBuffer->pScroll->pBack;

	s_pTextBitmap = fontCreateTextBitMap(
		WINDOW_WIDTH, g_pFont->uwHeight
	);

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
	fontDestroyTextBitMap(s_pTextBitmap);
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	windowHide();
}
