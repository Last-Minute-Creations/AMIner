/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "credits.h"
#include <comm/comm.h>
#include "aminer.h"
#include "core.h"

static const char *s_pCreditsLines[] = {
	"Aminer by Last Minute Creations",
	"lastminutecreations.itch.io/aminer",
	"",
	"Softiron: graphics, game design",
	"Luc3k: sounds, music",
	"KaiN: code",
	"",
	"Special thanks to: Rav.En, JakubH",
	"and two little RKLE18 testers!"
};
#define CREDITS_LINE_COUNT (sizeof(s_pCreditsLines) / sizeof(s_pCreditsLines[0]))


static void creditsGsCreate(void) {
	commEraseAll();
	UWORD uwOffsY = 0;
	UBYTE ubLineHeight = commGetLineHeight() - 2;
	for(UBYTE i = 0; i < CREDITS_LINE_COUNT; ++i) {
		commDrawText(0, uwOffsY, s_pCreditsLines[i], FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
		uwOffsY += ubLineHeight;
	}
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
