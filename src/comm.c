/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comm.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include "game.h"

typedef enum _tBtnState {
	BTN_STATE_NACTIVE = 0,
	BTN_STATE_ACTIVE = 1
} tBtnState;

static tBitMap *s_pBmRestore;
static tBitMap *s_pBg, *s_pButtons;
static UBYTE s_pNav[COMM_NAV_COUNT] = {BTN_STATE_NACTIVE};
static tBitMap *s_pBmDraw;

void commCreate(void) {
	s_pBmRestore = bitmapCreate(
		COMM_WIDTH, COMM_HEIGHT,
		g_pMainBuffer->sCommon.pVPort->ubBPP, BMF_INTERLEAVED
	);
	s_pBg = bitmapCreateFromFile("data/comm_bg.bm", 0);
	s_pButtons = bitmapCreateFromFile("data/comm_buttons.bm", 0);
}

void commDestroy(void) {
	bitmapDestroy(s_pBmRestore);
	bitmapDestroy(s_pBg);
	bitmapDestroy(s_pButtons);
}

void commSetActiveLed(tCommLed eLed) {
	const UBYTE ubLedWidth = 16;
	const UBYTE ubLedHeight = 13;
	const UBYTE ubGrnLedY = ubLedHeight;
	static const UBYTE pLedX[] = {19, 95, 181};
	const UBYTE ubLedY = 169;

	tUwCoordYX sOrigin = commGetOrigin();
	for(UBYTE i = 0; i < COMM_LED_COUNT; ++i) {
		blitCopy(
			s_pButtons, 0, (i == eLed ? ubGrnLedY : 0),
			s_pBmDraw, sOrigin.uwX + pLedX[i], sOrigin.uwY + ubLedY,
			ubLedWidth, ubLedHeight, MINTERM_COOKIE, 0xFF
		);
	}
}

UBYTE commShow(void) {
	tUwCoordYX sOrigin = commGetOrigin();
	if(g_pMainBuffer->uwMarginedHeight - sOrigin.uwX < COMM_HEIGHT) {
		// Not positioned evenly
		return 0;
	}

	s_pBmDraw = g_pMainBuffer->pScroll->pBack;

	// Store content beneath commrade
	blitCopyAligned(
		s_pBmDraw, sOrigin.uwX, sOrigin.uwY,
		s_pBmRestore, 0, 0, COMM_WIDTH, COMM_HEIGHT
	);

	// Draw commrade background
	blitCopyAligned(
		s_pBg, 0, 0, s_pBmDraw, sOrigin.uwX, sOrigin.uwY,
		COMM_WIDTH, COMM_HEIGHT
	);
	return 1;
}

void commProcess(void) {
	static const UWORD pBtnPos[COMM_NAV_COUNT][4] = {
		// dX, dY, sY, h
		{218, 114, 26, 14},
		{218, 143, 54, 14},
		{203, 128, 82, 15},
		{234, 128, 112, 15},
		{218, 129, 142, 13}
	};

	UBYTE pTests[COMM_NAV_COUNT] = {
		(
			keyCheck(KEY_W) || joyCheck(JOY1 + JOY_UP) ||
			(g_is2pPlaying && (keyCheck(KEY_UP) || joyCheck(JOY2 + JOY_UP)))
		),
		(
			keyCheck(KEY_S) || joyCheck(JOY1 + JOY_DOWN) ||
			(g_is2pPlaying && (keyCheck(KEY_DOWN) || joyCheck(JOY2 + JOY_DOWN)))
		),
		(
			keyCheck(KEY_A) || joyCheck(JOY1 + JOY_LEFT) ||
			(g_is2pPlaying && (keyCheck(KEY_LEFT) || joyCheck(JOY2 + JOY_LEFT)))
		),
		(
			keyCheck(KEY_D) || joyCheck(JOY1 + JOY_RIGHT) ||
			(g_is2pPlaying && (keyCheck(KEY_RIGHT) || joyCheck(JOY2 + JOY_RIGHT)))
		),
		(
			keyCheck(KEY_RETURN) || keyCheck(KEY_SPACE) || keyCheck(KEY_ESCAPE) ||
			joyCheck(JOY1 + JOY_FIRE) || (g_is2pPlaying && (joyCheck(JOY2 + JOY_FIRE)))
		)
	};

	tUwCoordYX sOrigin = commGetOrigin();

	for(UBYTE i = 0; i < COMM_NAV_COUNT; ++i) {
		if(pTests[i]) {
			if(s_pNav[i] == BTN_STATE_NACTIVE) {
				s_pNav[i] = BTN_STATE_ACTIVE;
				blitCopy(
					s_pButtons, 0, pBtnPos[i][2], s_pBmDraw,
					sOrigin.uwX + pBtnPos[i][0],
					sOrigin.uwY + pBtnPos[i][1],
					16, pBtnPos[i][3], MINTERM_COOKIE, 0xFF
				);
			}
		}
		else if(s_pNav[i] != BTN_STATE_NACTIVE) {
			s_pNav[i] = BTN_STATE_NACTIVE;
			blitCopy(
				s_pButtons, 0, pBtnPos[i][2] + pBtnPos[i][3], s_pBmDraw,
				sOrigin.uwX + pBtnPos[i][0],
				sOrigin.uwY + pBtnPos[i][1],
				16, pBtnPos[i][3], MINTERM_COOKIE, 0xFF
			);
		}
	}
}

UBYTE commNavCheck(tCommNav eNav) {
	return s_pNav[eNav];
}

UBYTE commNavUse(tCommNav eNav) {
	switch(eNav) {
		case COMM_NAV_UP:
			return (
				keyUse(KEY_W) || keyUse(KEY_UP) ||
				joyUse(JOY1 + JOY_UP) || joyUse(JOY2 + JOY_UP)
			);
		case COMM_NAV_DOWN:
			return (
				keyUse(KEY_S) || keyUse(KEY_DOWN) ||
				joyUse(JOY1 + JOY_DOWN) || joyUse(JOY2 + JOY_DOWN)
			);
		case COMM_NAV_LEFT:
			return (
				keyUse(KEY_A) || keyUse(KEY_LEFT) ||
				joyUse(JOY1 + JOY_LEFT) || joyUse(JOY2 + JOY_LEFT)
			);
		case COMM_NAV_RIGHT:
			return (
				keyUse(KEY_D) || keyUse(KEY_RIGHT) ||
				joyUse(JOY1 + JOY_RIGHT) || joyUse(JOY2 + JOY_RIGHT)
			);
		case COMM_NAV_BTN:
			return (
				keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || keyUse(KEY_ESCAPE) ||
				joyUse(JOY1 + JOY_FIRE) || joyUse(JOY2 + JOY_FIRE)
			);
		default:
			return 0;
	}
}

void commHide(void) {
	tUwCoordYX sOrigin = commGetOrigin();
	// Restore content beneath commrade
	blitCopyAligned(
		s_pBmRestore, 0, 0, s_pBmDraw, sOrigin.uwX, sOrigin.uwY,
		COMM_WIDTH, COMM_HEIGHT
	);
}

tUwCoordYX commGetOrigin(void) {
	UWORD uwScrollX = g_pMainBuffer->pCamera->uPos.uwX;
	UWORD uwScrollY = g_pMainBuffer->pCamera->uPos.uwY;
	UWORD uwBufferHeight = g_pMainBuffer->uwMarginedHeight;
	UWORD uwFoldScrollY = uwScrollY & (uwBufferHeight-1);
	UWORD uwCommOffsY = (g_pMainBuffer->sCommon.pVPort->uwHeight - COMM_HEIGHT) / 2;
	UWORD uwCommOffsX = (g_pMainBuffer->sCommon.pVPort->uwWidth - COMM_WIDTH) / 2;

	tUwCoordYX sOrigin;
	sOrigin.uwX = uwScrollX + uwCommOffsX;
	sOrigin.uwY = uwFoldScrollY + uwCommOffsY;

	return sOrigin;
}

tUwCoordYX commGetOriginDisplay(void) {
	tUwCoordYX sOrigin = commGetOrigin();
	sOrigin.uwX += COMM_DISPLAY_X;
	sOrigin.uwY += COMM_DISPLAY_Y;
	return sOrigin;
}
