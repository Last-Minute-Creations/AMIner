/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "window.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include "game.h"

typedef enum _tBtnState {
	BTN_STATE_NACTIVE = 0,
	BTN_STATE_ACTIVE = 1
} tBtnState;

static tBitMap *s_pRestoreBuffer;
static tBitMap *s_pBg, *s_pButtons;
static UBYTE s_pNav[WINDOW_NAV_COUNT] = {BTN_STATE_NACTIVE};
static tBitMap *s_pDrawBfr;

void windowInit(void) {
	s_pRestoreBuffer = bitmapCreate(
		WINDOW_WIDTH, WINDOW_HEIGHT,
		g_pMainBuffer->sCommon.pVPort->ubBPP, BMF_INTERLEAVED
	);
	s_pBg = bitmapCreateFromFile("data/commrade_bg.bm", 0);
	s_pButtons = bitmapCreateFromFile("data/commrade_buttons.bm", 0);
}

void windowDeinit(void) {
	bitmapDestroy(s_pRestoreBuffer);
	bitmapDestroy(s_pBg);
	bitmapDestroy(s_pButtons);
}

UBYTE windowShow(void) {
	tUwCoordYX sOrigin = windowGetOrigin();
	if(g_pMainBuffer->uwMarginedHeight - sOrigin.sUwCoord.uwX < WINDOW_HEIGHT) {
		// Not positioned evenly
		return 0;
	}

	s_pDrawBfr = g_pMainBuffer->pScroll->pBack;

	// Store content beneath window
	blitCopyAligned(
		s_pDrawBfr,
		sOrigin.sUwCoord.uwX, sOrigin.sUwCoord.uwY,
		s_pRestoreBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT
	);

	// Draw window background
	blitCopyAligned(
		s_pBg, 0, 0,
		s_pDrawBfr, sOrigin.sUwCoord.uwX, sOrigin.sUwCoord.uwY,
		WINDOW_WIDTH, WINDOW_HEIGHT
	);
	return 1;
}

void windowProcess(void) {
	static const UWORD pCoords[WINDOW_NAV_COUNT][4] = {
		// dX, dY, sY, h
		{218, 114, 26, 14},
		{218, 143, 54, 14},
		{203, 128, 82, 15},
		{234, 128, 112, 15},
		{218, 129, 142, 13}
	};

	UBYTE pTests[WINDOW_NAV_COUNT] = {
		(
			keyCheck(KEY_W) || keyCheck(KEY_UP) ||
			joyCheck(JOY1 + JOY_UP) || joyCheck(JOY2 + JOY_UP)
		),
		(
			keyCheck(KEY_S) || keyCheck(KEY_DOWN) ||
			joyCheck(JOY1 + JOY_DOWN) || joyCheck(JOY2 + JOY_DOWN)
		),
		(
			keyCheck(KEY_A) || keyCheck(KEY_LEFT) ||
			joyCheck(JOY1 + JOY_LEFT) || joyCheck(JOY2 + JOY_LEFT)
		),
		(
			keyCheck(KEY_D) || keyCheck(KEY_RIGHT) ||
			joyCheck(JOY1 + JOY_RIGHT) || joyCheck(JOY2 + JOY_RIGHT)
		),
		(
		keyCheck(KEY_RETURN) || keyCheck(KEY_SPACE) || keyCheck(KEY_ESCAPE) ||
		joyCheck(JOY1 + JOY_FIRE) || joyCheck(JOY2 + JOY_FIRE)
		)
	};

	tUwCoordYX sOrigin = windowGetOrigin();

	for(UBYTE i = 0; i < WINDOW_NAV_COUNT; ++i) {
		if(pTests[i]) {
			if(s_pNav[i] == BTN_STATE_NACTIVE) {
				s_pNav[i] = BTN_STATE_ACTIVE;
				blitCopy(
					s_pButtons, 0, pCoords[i][2], s_pDrawBfr,
					sOrigin.sUwCoord.uwX + pCoords[i][0],
					sOrigin.sUwCoord.uwY + pCoords[i][1],
					16, pCoords[i][3], MINTERM_COOKIE, 0xFF
				);
			}
		}
		else if(s_pNav[i] != BTN_STATE_NACTIVE) {
			s_pNav[i] = BTN_STATE_NACTIVE;
			blitCopy(
				s_pButtons, 0, pCoords[i][2] + pCoords[i][3], s_pDrawBfr,
				sOrigin.sUwCoord.uwX + pCoords[i][0],
				sOrigin.sUwCoord.uwY + pCoords[i][1],
				16, pCoords[i][3], MINTERM_COOKIE, 0xFF
			);
		}
	}
}

UBYTE windowNavCheck(tWindowNav eNav) {
	return s_pNav[eNav];
}

UBYTE windowNavUse(tWindowNav eNav) {
	switch(eNav) {
		case WINDOW_NAV_UP:
			return (
				keyUse(KEY_W) || keyUse(KEY_UP) ||
				joyUse(JOY1 + JOY_UP) || joyUse(JOY2 + JOY_UP)
			);
		case WINDOW_NAV_DOWN:
			return (
				keyUse(KEY_S) || keyUse(KEY_DOWN) ||
				joyUse(JOY1 + JOY_DOWN) || joyUse(JOY2 + JOY_DOWN)
			);
		case WINDOW_NAV_LEFT:
			return (
				keyUse(KEY_A) || keyUse(KEY_LEFT) ||
				joyUse(JOY1 + JOY_LEFT) || joyUse(JOY2 + JOY_LEFT)
			);
		case WINDOW_NAV_RIGHT:
			return (
				keyUse(KEY_D) || keyUse(KEY_RIGHT) ||
				joyUse(JOY1 + JOY_RIGHT) || joyUse(JOY2 + JOY_RIGHT)
			);
		case WINDOW_NAV_BTN:
			return (
				keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || keyUse(KEY_ESCAPE) ||
				joyUse(JOY1 + JOY_FIRE) || joyUse(JOY2 + JOY_FIRE)
			);
		default:
			return 0;
	}
}

void windowHide(void) {
	tUwCoordYX sOrigin = windowGetOrigin();
	// Restore content beneath window
	blitCopyAligned(
		s_pRestoreBuffer, 0, 0,
		s_pDrawBfr, sOrigin.sUwCoord.uwX, sOrigin.sUwCoord.uwY,
		WINDOW_WIDTH, WINDOW_HEIGHT
	);
}

tUwCoordYX windowGetOrigin(void) {
	UWORD uwScrollX = g_pMainBuffer->pCamera->uPos.sUwCoord.uwX;
	UWORD uwScrollY = g_pMainBuffer->pCamera->uPos.sUwCoord.uwY;
	UWORD uwBufferHeight = g_pMainBuffer->uwMarginedHeight;
	UWORD uwFoldScrollY = uwScrollY & (uwBufferHeight-1);
	UWORD uwWindowOffsY = (g_pMainBuffer->sCommon.pVPort->uwHeight - WINDOW_HEIGHT) / 2;
	UWORD uwWindowOffsX = (g_pMainBuffer->sCommon.pVPort->uwWidth - WINDOW_WIDTH) / 2;

	tUwCoordYX sCoord;
	sCoord.sUwCoord.uwX = uwScrollX + uwWindowOffsX;
	sCoord.sUwCoord.uwY = uwFoldScrollY + uwWindowOffsY;

	return sCoord;
}
