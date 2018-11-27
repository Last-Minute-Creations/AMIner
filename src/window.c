/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "window.h"
#include "game.h"

#define WINDOW_WIDTH (320-64)
#define WINDOW_HEIGHT (200)

tBitMap *s_pBgBuffer;

void windowInit(void) {
	s_pBgBuffer = bitmapCreate(
		WINDOW_WIDTH, WINDOW_HEIGHT,
		g_pMainBuffer->sCommon.pVPort->ubBPP, BMF_INTERLEAVED
	);
}

void windowDeinit(void) {
	bitmapDestroy(s_pBgBuffer);
}

UBYTE windowShow(void) {
	UWORD uwScrollX = g_pMainBuffer->pCamera->uPos.sUwCoord.uwX;
	UWORD uwScrollY = g_pMainBuffer->pCamera->uPos.sUwCoord.uwY;
	UWORD uwBufferHeight = g_pMainBuffer->uwMarginedHeight;
	UWORD uwFoldScrollY = uwScrollY & (uwBufferHeight-1);
	UWORD uwWindowOffsY = (g_pMainBuffer->sCommon.pVPort->uwHeight - WINDOW_HEIGHT) / 2;
	UWORD uwWindowOffsX = (g_pMainBuffer->sCommon.pVPort->uwWidth - WINDOW_WIDTH) / 2;
	if(uwBufferHeight - uwFoldScrollY < uwWindowOffsY + WINDOW_HEIGHT) {
		// Not positioned evenly
		return 0;
	}
	// Store content beneath window
	blitCopyAligned(
		g_pMainBuffer->pScroll->pBack,
		uwScrollX + uwWindowOffsX, uwFoldScrollY + uwWindowOffsY,
		s_pBgBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT
	);

	// Draw window
	blitRect(
		g_pMainBuffer->pScroll->pBack,
		uwScrollX + uwWindowOffsX, uwFoldScrollY + uwWindowOffsY,
		WINDOW_WIDTH, WINDOW_HEIGHT, 0
	);
	return 1;
}

void windowHide(void) {
	UWORD uwScrollX = g_pMainBuffer->pCamera->uPos.sUwCoord.uwX;
	UWORD uwScrollY = g_pMainBuffer->pCamera->uPos.sUwCoord.uwY;
	UWORD uwBufferHeight = g_pMainBuffer->uwMarginedHeight;
	UWORD uwFoldScrollY = uwScrollY & (uwBufferHeight-1);
	UWORD uwWindowOffsY = (g_pMainBuffer->sCommon.pVPort->uwHeight - WINDOW_HEIGHT) / 2;
	UWORD uwWindowOffsX = (g_pMainBuffer->sCommon.pVPort->uwWidth - WINDOW_WIDTH) / 2;
	// Restore content beneath window
	blitCopyAligned(
		s_pBgBuffer, 0, 0, g_pMainBuffer->pScroll->pBack,
		uwScrollX + uwWindowOffsX, uwFoldScrollY + uwWindowOffsY,
		WINDOW_WIDTH, WINDOW_HEIGHT
	);
}
