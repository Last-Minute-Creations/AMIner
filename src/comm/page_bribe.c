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

static UBYTE s_ubBribeAccoladeCount, s_ubBribeRebukeCount;
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
					tPlan *pPlan = warehouseGetCurrentPlan();
					if(!pPlan->isPenaltyCountdownStarted) {
						// accolade bribe
						++s_ubBribeAccoladeCount;
						heatTryIncrease(2);
					}
					else {
						// rebuke bribe
						++s_ubBribeRebukeCount;
					}
					planAddDays(pPlan, 14, 1);
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
	const tPlan *pPlan = warehouseGetCurrentPlan();
	char szBfr[150];
	UWORD uwPosY = 0;
	UWORD uwCost;

	if (!pPlan->isActive) {
		uwPosY += commDrawMultilineText(
			"You have no active plan! What do you want me to do?", 0, uwPosY
		) * ubLineHeight;
		buttonInitOk("Back");
	}
	else if(!pPlan->isExtendedTimeByFavor) {
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
			heatGetPercent()
		);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;
		s_uwBribeCost = uwCost;
		sprintf(szBfr, "It will cost you %hu\x1F.", uwCost);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Accept", "Decline");
	}
	else {
		uwPosY += commDrawMultilineText("Comrade, not now... there's too much heat!", 0, uwPosY) * ubLineHeight;
		buttonInitOk("Back");
	}

	buttonDrawAll(commGetDisplayBuffer());
}

void pageBribeReset(void) {
	s_ubBribeAccoladeCount = 0;
	s_ubBribeRebukeCount = 0;
}

void pageBribeSave(tFile *pFile) {
	saveWriteHeader(pFile, "BRBE");
	fileWrite(pFile, &s_ubBribeAccoladeCount, sizeof(s_ubBribeAccoladeCount));
	fileWrite(pFile, &s_ubBribeRebukeCount, sizeof(s_ubBribeRebukeCount));
}

UBYTE pageBribeLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "BRBE")) {
		return 0;
	}

	fileRead(pFile, &s_ubBribeAccoladeCount, sizeof(s_ubBribeAccoladeCount));
	fileRead(pFile, &s_ubBribeRebukeCount, sizeof(s_ubBribeRebukeCount));
	return 1;
}

