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
	if(g_pMainBuffer->pCamera->uPos.sUwCoord.uwY & (g_pMainBuffer->uwMarginedHeight-1)) {
		// Not positioned evenly
		return 0;
	}
	// Store content beneath window
	blitCopyAligned(
		g_pMainBuffer->pScroll->pBack,
		(g_pMainBuffer->sCommon.pVPort->uwWidth - WINDOW_WIDTH) / 2,
		(g_pMainBuffer->sCommon.pVPort->uwHeight - WINDOW_HEIGHT) / 2,
		s_pBgBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT
	);

	// Draw window
	blitRect(
		g_pMainBuffer->pScroll->pBack,
		(g_pMainBuffer->sCommon.pVPort->uwWidth - WINDOW_WIDTH) / 2,
		(g_pMainBuffer->sCommon.pVPort->uwHeight - WINDOW_HEIGHT) / 2,
		WINDOW_WIDTH, WINDOW_HEIGHT, 0
	);
	return 1;
}

void windowHide(void) {
	// Restore content beneath window
	blitCopyAligned(
		s_pBgBuffer, 0, 0,
		g_pMainBuffer->pScroll->pBack,
		(g_pMainBuffer->sCommon.pVPort->uwWidth - WINDOW_WIDTH) / 2,
		(g_pMainBuffer->sCommon.pVPort->uwHeight - WINDOW_HEIGHT) / 2,
		WINDOW_WIDTH, WINDOW_HEIGHT
	);
}
