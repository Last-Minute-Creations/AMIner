#include "vendor.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include "window.h"
#include "game.h"

static UBYTE s_isShown;
tBitMap *s_pBuffer;

void vendorGsCreate(void) {
	s_isShown = windowShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_pBuffer = g_pMainBuffer->pScroll->pBack;

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void vendorGsLoop(void) {
	if(keyUse(KEY_L)) {
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
