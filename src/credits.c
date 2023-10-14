/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "credits.h"
#include <comm/comm.h>
#include "aminer.h"
#include "core.h"

static void creditsGsCreate(void) {
	commEraseAll();
	commDrawText(0, 0, "dupa", FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
}

static void creditsGsLoop(void) {
	commProcess();

	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		statePop(g_pGameStateManager);
	}

	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void creditsGsDestroy(void) {

}

void creditsReset(UBYTE isOnGameEnd) {

}

tState g_sStateMenuCredits = {
	.cbCreate = creditsGsCreate, .cbLoop = creditsGsLoop,
	.cbDestroy = creditsGsDestroy
};
