/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <comm/base.h>
#include <comm/page_office.h>
#include <comm/button.h>
#include "../warehouse.h"
#include "../game.h"
#include "../core.h"

static UBYTE s_ubBribeAccoladeCount, s_ubBribeRebukeCount;
static BYTE s_bBribeChanceFail;

static void pageBribeProcess(void) {
// #error take cash for a bribe!

	if(buttonGetPreset() == BUTTON_PRESET_ACCEPT_DECLINE) {
		BYTE bButtonPrev = buttonGetSelected(), bButtonCurr = bButtonPrev;
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
				if(randUwMinMax(&g_sRand, 1, 100) > s_bBribeChanceFail) {
					// Success
					const tPlan *pPlan = warehouseGetPlan();
					if(!pPlan->isPenaltyCountdownStarted) {
						// accolade bribe
						++s_ubBribeAccoladeCount;
						s_bBribeChanceFail = MIN(s_bBribeChanceFail + 2, 100);
					}
					else {
						// rebuke bribe
						++s_ubBribeRebukeCount;
						s_bBribeChanceFail = MIN(s_bBribeChanceFail + 5, 100);
					}
					warehouseAddDaysToPlan(14, 1);
				}
				else {
					gameAddRebuke();
				}
			}
			pageOfficeCreate();
		}
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			pageOfficeCreate();
		}
	}
}

void pageBribeCreate(void) {
	commRegisterPage(pageBribeProcess, 0);
	const UBYTE ubLineHeight = commGetLineHeight();
	const tPlan *pPlan = warehouseGetPlan();
	char szBfr[150];
	UWORD uwPosY = 0;
	UWORD uwCost;

	if(!pPlan->isExtendedTime) {
		if(!pPlan->isPenaltyCountdownStarted) {
			sprintf(szBfr, "Bribe for extra %hhu days for finishing plan in time.", 14);
			uwCost = 100;
			for(UBYTE i = s_ubBribeAccoladeCount; i--;) {
				uwCost = (uwCost * 120 / 100);
			}
		}
		else {
			sprintf(szBfr, "Bribe for extra %hhu days before getting a penalty.", 14);
			uwCost = 200;
			for(UBYTE i = s_ubBribeRebukeCount; i--;) {
				uwCost = (uwCost * 120 / 100);
			}
		}
		uwPosY += commDrawMultilineText(szBfr,0, uwPosY) * ubLineHeight;

		uwPosY += ubLineHeight / 2;
		sprintf(
			szBfr, "There is %hhu%% chance that we will get caught, which would result in instantly getting a rebuke.",
			s_bBribeChanceFail
		);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;
		sprintf(szBfr, "It will cost you %hu\x1F.", uwCost);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Accept", "Decline");
		buttonDrawAll(commGetDisplayBuffer());
	}
	else {
		uwPosY += commDrawMultilineText("Comrade, not now... there's too much heat!", 0, uwPosY) * ubLineHeight;
		buttonInitOk("Back");
	}
}

void pageBribeReset(void) {
	s_bBribeChanceFail = 0;
	s_ubBribeAccoladeCount = 0;
	s_ubBribeRebukeCount = 0;
}
