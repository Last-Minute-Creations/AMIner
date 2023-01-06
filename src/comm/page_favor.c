/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_favor.h"
#include <comm/base.h>
#include <comm/button.h>
#include <comm/page_office.h>
#include "../game.h"
#include "../warehouse.h"

static UBYTE s_ubFavorsLeft;

static void pageFavorProcess(void) {
	if(buttonGetPreset() == BUTTON_PRESET_ACCEPT_DECLINE) {
		BYTE bButtonPrev = buttonGetSelected();
		BYTE bButtonCurr = bButtonPrev;
		BYTE bButtonCount = buttonGetCount();
		if(commNavUse(COMM_NAV_RIGHT)) {
			bButtonCurr = MIN(bButtonCurr + 1, bButtonCount - 1);
		}
		else if(commNavUse(COMM_NAV_LEFT)) {
			bButtonCurr = MAX(bButtonCurr - 1, 0);
		}
		if(bButtonPrev != bButtonCurr) {
			buttonSelect(bButtonCurr);
			buttonDrawAll(commGetDisplayBuffer());
		}

		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			if(bButtonCurr == 0) {
				--s_ubFavorsLeft;
				warehouseRerollPlan();
			}
			pageOfficeGoBack();
		}
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			pageOfficeGoBack();
		}
	}
}

void pageFavorCreate(void) {
	commRegisterPage(pageFavorProcess, 0);
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = commGetLineHeight();
	WORD wDays = planGetRemainingDays(warehouseGetCurrentPlan());
	if(s_ubFavorsLeft > 0 && wDays >= 25) {

		uwPosY += commDrawMultilineText(
			"I like working with you Comrade, I really do."
			" I heard that current plan is tough for you. If you want,"
			" I can make some calls and try to do something about it.",
			0, uwPosY
		) * ubLineHeight;

		uwPosY += ubLineHeight / 2;
		uwPosY += commDrawMultilineText("Urz\x84""das can replace current plan with another one.", 0, uwPosY) * ubLineHeight;
		char szBfr[100];
		sprintf(szBfr, "You have %hhu favors left.", s_ubFavorsLeft);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Accept", "Decline");
		buttonDrawAll(commGetDisplayBuffer());
	}
	else {
		uwPosY += commDrawMultilineText("You ask me for too much, Comrade. Do some real work, will you?", 0, uwPosY) * ubLineHeight;
		buttonInitOk("Back");
	}
}

void pageFavorReset(void) {
	s_ubFavorsLeft = 10;
}
