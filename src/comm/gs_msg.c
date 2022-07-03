/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gs_msg.h"
#include <ace/managers/system.h>
#include <ace/utils/font.h>
#include <comm/base.h>
#include <comm/page_msg.h>
#include "core.h"
#include "color.h"
#include "defs.h"

static UBYTE s_isShown;

static void cbOnClose(void) {
	commRegisterPage(0, 0);
}

static void commGsMsgCreate(void) {
	s_isShown = commTryShow();
	if(!s_isShown) {
		// Camera not placed properly
		statePop(g_pGameStateManager);
		return;
	}

	pageMsgCreate("intro", cbOnClose);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now on!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void commGsMsgLoop(void) {
	commProcess();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	if(!commProcessPage()) {
		blitWait();
		statePop(g_pGameStateManager);
		return;
	}
}

static void commGsMsgDestroy(void) {
	if(!s_isShown) {
		return;
	}

	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}

tState g_sStateMsg = {
	.cbCreate = commGsMsgCreate, .cbLoop = commGsMsgLoop,
	.cbDestroy = commGsMsgDestroy
};
