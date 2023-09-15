/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_market.h"
#include <comm/comm.h>
#include <comm/button.h>
#include "defs.h"

//----------------------------------------------------------------- PRIVATE VARS

//------------------------------------------------------------ PRIVATE FUNCTIONS

static void pageMarketProcess(void) {

}

static void pageMarketDestroy(void) {

}

//------------------------------------------------------------- PUBLIC FUNCTIONS

void pageMarketCreate(void) {
	commRegisterPage(pageMarketProcess, pageMarketDestroy);

	UWORD uwOffsY = 0;
	UBYTE ubLineHeight = 8;

	// Create buttons
	buttonInitOk(g_pMsgs[MSG_COMM_EXIT]);
	buttonDrawAll(commGetDisplayBuffer());

	// Draw all
	commDrawText(0, uwOffsY, "Mineral: < Silver >", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT);
	uwOffsY += ubLineHeight;

	commDrawText(0, uwOffsY, "Stock: 35", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	uwOffsY +=  (3 * ubLineHeight) / 2;

	commDrawText(0, uwOffsY, "Trade:", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(60, uwOffsY, "For:", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(COMM_DISPLAY_WIDTH, uwOffsY, "Stock:", FONT_COOKIE | FONT_LAZY | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT_DARK);
	uwOffsY +=  (3 * ubLineHeight) / 2;

	commDrawText(5, uwOffsY, "4 Silver", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(60, uwOffsY, "1 Gold", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(COMM_DISPLAY_WIDTH, uwOffsY, "13", FONT_COOKIE | FONT_LAZY | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT_DARK);
	uwOffsY += ubLineHeight;
	commDrawText(5, uwOffsY, "8 Silver", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(60, uwOffsY, "1 Emerald", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(COMM_DISPLAY_WIDTH, uwOffsY, "7", FONT_COOKIE | FONT_LAZY | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT_DARK);
	uwOffsY += ubLineHeight;
	commDrawText(5, uwOffsY, "12 Silver", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(60, uwOffsY, "1 Ruby", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(COMM_DISPLAY_WIDTH, uwOffsY, "3", FONT_COOKIE | FONT_LAZY | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT_DARK);
	uwOffsY += ubLineHeight;
	commDrawText(5, uwOffsY, "16 Silver", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(60, uwOffsY, "1 Moonstone", FONT_COOKIE | FONT_LAZY, COMM_DISPLAY_COLOR_TEXT_DARK);
	commDrawText(COMM_DISPLAY_WIDTH, uwOffsY, "0", FONT_COOKIE | FONT_LAZY | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT_DARK);
	uwOffsY += ubLineHeight;
}
