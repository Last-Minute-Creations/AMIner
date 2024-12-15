/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <comm/page_use_crates.h>
#include <comm/page_office.h>
#include <comm/button.h>
#include "../vehicle.h"
#include "../quest_crate.h"
#include "../hud.h"

static tPageUseCratesScenario s_eScenario;

static UBYTE pageUseCratesGetMinAmount(void) {
	switch(s_eScenario) {
		case PAGE_USE_CRATES_SCENARIO_TELEPORTER:
			return 5;
		case PAGE_USE_CRATES_SCENARIO_CAPSULE:
			return 5;
		case PAGE_USE_CRATES_SCENARIO_SELL:
			return 1;
	}
	return 0;
}

static void pageUseCratesDrawAll(void) {
	const UBYTE ubLineHeight = commGetLineHeight();
	UWORD uwPosY = 0;
	UBYTE ubCrateCount = questCrateGetCount();
	char szBfr[150];

	const char *szMsgPremise = 0;
	const char *szMsgNotYet = 0;
	const char *szMsgUse = 0;
	UBYTE ubMinAmount = pageUseCratesGetMinAmount();
	switch(s_eScenario) {
		case PAGE_USE_CRATES_SCENARIO_TELEPORTER:
			szMsgPremise = "Aby naprawic teleporter potrzebujemy czesci z 5 skrzyn.";
			szMsgNotYet = "Daj znac jak je zbierzesz.";
			szMsgUse = "Przekaz skrzynie";
			break;
		case PAGE_USE_CRATES_SCENARIO_CAPSULE:
			szMsgPremise = "Aby ustabilizowac i otworzyc kapsule potrzebujemy czesci z 5 skrzyn.";
			szMsgNotYet = "Daj znac jak je zbierzesz.";
			szMsgUse = "Przekaz skrzynie";
			break;
		case PAGE_USE_CRATES_SCENARIO_SELL:
			sprintf(szBfr, "Kupie od Ciebie kazda skrzynie za %hu\x1F.", 1000);
			szMsgPremise = szBfr;
			szMsgNotYet = "Wroc jak bedziesz mial cos na sprzedaz.";
			szMsgUse = "Sprzedaj skrzynie";
			break;
	}

	uwPosY += commDrawMultilineText(szMsgPremise, 0, uwPosY) * ubLineHeight;
	if(ubCrateCount < ubMinAmount) {
		uwPosY += commDrawMultilineText(szMsgNotYet, 0, uwPosY) * ubLineHeight;
		buttonInitOk(g_pMsgs[MSG_PAGE_BACK]);
	}
	else {
		buttonInitAcceptDecline(szMsgUse, g_pMsgs[MSG_PAGE_BACK]);
	}

	sprintf(szBfr, "Pozostale skrzynie: %hhu", ubCrateCount);
	uwPosY += ubLineHeight;
	uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

	buttonDrawAll(commGetDisplayBuffer());
}

static void pageUseCratesProcess(void) {
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

	tButtonPreset ePreset = buttonGetPreset();
	if(ePreset == BUTTON_PRESET_ACCEPT_DECLINE) {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			if(bButtonCurr == 0) {
				switch(s_eScenario) {
					case PAGE_USE_CRATES_SCENARIO_TELEPORTER:
						if(questCrateTryConsume(pageUseCratesGetMinAmount())) {
							pageOfficeLockPersonSubpage(FACE_ID_SCIENTIST, COMM_SHOP_PAGE_OFFICE_SCIENTIST_CRATE_TELEPORTER);
							pageOfficeTryUnlockPersonSubpage(FACE_ID_SCIENTIST, COMM_SHOP_PAGE_OFFICE_SCIENTIST_ESCAPE);
							commShopChangePage(COMM_SHOP_PAGE_OFFICE_LIST_SCI, COMM_SHOP_PAGE_OFFICE_SCIENTIST_ESCAPE);
						}
						else {
							commShopGoBack();
						}
						break;
					case PAGE_USE_CRATES_SCENARIO_CAPSULE:
						if(questCrateTryConsume(pageUseCratesGetMinAmount())) {
							questCrateSetCapsuleState(CAPSULE_STATE_OPENED);
						}
						else {
							commShopGoBack();
						}
						break;
					case PAGE_USE_CRATES_SCENARIO_SELL:
						if(questCrateTrySell()) {
							g_pVehicles[0].lCash += 1000;
							hudSetCash(0, g_pVehicles[0].lCash);
						}
						commEraseAll();
						pageUseCratesDrawAll();
						break;
				}
			}
			else {
				commShopGoBack();
			}
		}
	}
	else if(ePreset == BUTTON_PRESET_OK) {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			commShopGoBack();
		}
	}
}

void pageUseCratesCreate(tPageUseCratesScenario eScenario) {
	commRegisterPage(pageUseCratesProcess, 0);
	s_eScenario = eScenario;
	pageUseCratesDrawAll();
}
