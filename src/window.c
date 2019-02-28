/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "window.h"
#include "game.h"
#include <ace/utils/bmframe.h>

tBitMap *s_pBgBuffer;
tBitMap *s_pFrame;

void windowInit(void) {
	s_pBgBuffer = bitmapCreate(
		WINDOW_WIDTH, WINDOW_HEIGHT,
		g_pMainBuffer->sCommon.pVPort->ubBPP, BMF_INTERLEAVED
	);
	s_pFrame = bitmapCreateFromFile("data/shop_border.bm", 0);
}

void windowDeinit(void) {
	bitmapDestroy(s_pFrame);
	bitmapDestroy(s_pBgBuffer);
}

UBYTE windowShow(void) {
	tUwCoordYX sOrigin = windowGetOrigin();
	if(g_pMainBuffer->uwMarginedHeight - sOrigin.sUwCoord.uwX < WINDOW_HEIGHT) {
		// Not positioned evenly
		return 0;
	}
	// Store content beneath window
	blitCopyAligned(
		g_pMainBuffer->pScroll->pBack,
		sOrigin.sUwCoord.uwX, sOrigin.sUwCoord.uwY,
		s_pBgBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT
	);

	// Draw window background
	bmFrameDraw(
		s_pFrame, g_pMainBuffer->pScroll->pBack,
		sOrigin.sUwCoord.uwX, sOrigin.sUwCoord.uwY,
		WINDOW_WIDTH / 16, WINDOW_HEIGHT / 16, 16
	);
	return 1;
}

void windowHide(void) {
	tUwCoordYX sOrigin = windowGetOrigin();
	// Restore content beneath window
	blitCopyAligned(
		s_pBgBuffer, 0, 0, g_pMainBuffer->pScroll->pBack,
		sOrigin.sUwCoord.uwX, sOrigin.sUwCoord.uwY,
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
