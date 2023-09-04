/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <comm/comm.h>
#include <comm/page_office.h>
#include <comm/button.h>
#include "../warehouse.h"
#include "../game.h"
#include "../core.h"
#include "../vehicle.h"
#include "../save.h"
#include "../heat.h"

static UWORD s_uwBribeCount;
static UWORD s_uwBribeCost;

static void pageBribeProcess(void) {
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
				g_pVehicles[0].lCash -= s_uwBribeCost;
				if(randUwMinMax(&g_sRand, 1, 100) > heatGetPercent()) {
					// Success
					pageOfficeTryUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_FAVOR);
					heatTryIncrease(5);
					++s_uwBribeCount;
					planAddDays(14, 1);
				}
				else {
					gameAddRebuke();
				}
			}
			commShopGoBack();
		}
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			commShopGoBack();
		}
	}
}

void pageBribeCreate(void) {
	commRegisterPage(pageBribeProcess, 0);
	const UBYTE ubLineHeight = commGetLineHeight();
	char szBfr[150];
	UWORD uwPosY = 0;

	s_uwBribeCost = 100;
	for(UBYTE i = s_uwBribeCount; i--;) {
		s_uwBribeCost = (s_uwBribeCost * 150 / 100);
	}

	if (!planManagerGet()->isPlanActive) {
		uwPosY += commDrawMultilineText(
			"You have no active plan! What do you want me to do?", 0, uwPosY
		) * ubLineHeight;
		buttonInitOk("Back");
	}
	else if(!planManagerGet()->isExtendedTimeByFavor) {
		sprintf(szBfr, "Bribe for extra %hhu days for plan.", 14);
		uwPosY += commDrawMultilineText(szBfr,0, uwPosY) * ubLineHeight;
		uwPosY += ubLineHeight / 2;
		sprintf(
			szBfr, "There is %hhu%% chance that we will get caught, which would result in instantly getting a rebuke.",
			heatGetPercent()
		);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;
		sprintf(szBfr, "It will cost you %hu\x1F.", s_uwBribeCost);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Accept", "Decline");
	}
	else {
		uwPosY += commDrawMultilineText("Comrade, not now... I've already helped you with current plan!", 0, uwPosY) * ubLineHeight;
		buttonInitOk("Back");
	}

	buttonDrawAll(commGetDisplayBuffer());
}

void pageBribeReset(void) {
	s_uwBribeCount = 0;
}

void pageBribeSave(tFile *pFile) {
	saveWriteHeader(pFile, "BRBE");
	fileWrite(pFile, &s_uwBribeCount, sizeof(s_uwBribeCount));
}

UBYTE pageBribeLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "BRBE")) {
		return 0;
	}

	fileRead(pFile, &s_uwBribeCount, sizeof(s_uwBribeCount));
	return 1;
}

