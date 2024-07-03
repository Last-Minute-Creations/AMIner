/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <comm/page_office.h>
#include <comm/button.h>
#include "../vehicle.h"
#include "../quest_crate.h"
#include "../hud.h"

static void pageSellCratesDrawAll(void) {
	const UBYTE ubLineHeight = commGetLineHeight();
	char szBfr[150];
	UWORD uwPosY = 0;

	UBYTE ubCrateCount = questCrateGetCount();
	if (ubCrateCount == 0) {
		uwPosY += commDrawMultilineText(
			"Wroc jak bedziesz mial cos na sprzedaz!", 0, uwPosY
		) * ubLineHeight;
		buttonInitOk("Back");
	}
	else {
		sprintf(szBfr, "Kupie od Ciebie kazda skrzynie za %hu\x1F.", 1000);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;
		uwPosY += ubLineHeight;
		sprintf(szBfr, "Pozostale skrzynie: %hu", ubCrateCount);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Sell", "Back");
	}

	buttonDrawAll(commGetDisplayBuffer());
}

static void pageSellCratesProcess(void) {
	if(buttonGetPreset() == BUTTON_PRESET_ACCEPT_DECLINE) {
		BYTE bButtonPrev = buttonGetSelected(), bButtonCurr = bButtonPrev;
		BYTE bButtonCount = buttonGetCount();
		if(commNavUse(DIRECTION_RIGHT)) {
			bButtonCurr = MIN(bButtonCurr + 1, bButtonCount - 1);
		}
		else if(commNavUse(DIRECTION_LEFT)) {
			bButtonCurr = MAX(bButtonCurr - 1, 0);
		}
		if(bButtonPrev != bButtonCurr) {
			buttonSelect(bButtonCurr);
			buttonDrawAll(commGetDisplayBuffer());
		}

		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			if(bButtonCurr == 0) {
				if(questCrateTrySell()) {
					g_pVehicles[0].lCash += 1000;
					hudSetCash(0, g_pVehicles[0].lCash);
				}
				commEraseAll();
				pageSellCratesDrawAll();
			}
			else {
				commShopGoBack();
			}
		}
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			commShopGoBack();
		}
	}
}

void pageSellCratesCreate(void) {
	commRegisterPage(pageSellCratesProcess, 0);
	pageSellCratesDrawAll();
}
