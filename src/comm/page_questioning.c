/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_questioning.h"
#include <comm/comm.h>
#include <comm/page_office.h>
#include <comm/button.h>
#include <comm/gs_shop.h>
#include <comm/inbox.h>
#include "../save.h"
#include "../heat.h"
#include "../game.h"
#include "../hud.h"

#define QUESTIONING_HEAT_INCREASE 5
#define QUESTIONING_HEAT_DECREASE_TRUTH 5

static tQuestioningBit s_eQuestioningsPending;
static tQuestioningBit s_eQuestioningsReported;
static tQuestioningBit s_eQuestioningCurrent;

static void pageQuestioningProcess(void) {
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
			s_eQuestioningsReported |= s_eQuestioningCurrent;
		}
		else {
			if(heatTryPassCheck()) {
				s_eQuestioningsPending &= ~(s_eQuestioningCurrent);
			}
			else {
				gameAddRebuke();
			}
		}
		commShopGoBack();
	}
}

void pageQuestioningCreate(void) {
	commRegisterPage(pageQuestioningProcess, 0);
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = commGetLineHeight();

	s_eQuestioningCurrent = QUESTIONING_BIT_GATE;
	while(s_eQuestioningCurrent < QUESTIONING_BIT_END) {
		if(s_eQuestioningCurrent & s_eQuestioningsPending) {
			break;
		}
		s_eQuestioningCurrent <<= 1;
	}

	const char *szMsg = "???";
	switch (s_eQuestioningCurrent)
	{
		case QUESTIONING_BIT_GATE:
			szMsg = "Have you found some gate parts?";
			break;
		case QUESTIONING_BIT_TELEPORT_PARTS:
			szMsg = "Have you found some teleport parts?";
			break;
		default:
			logWrite("ERR: Unhandled questioning value: %d\n", s_eQuestioningCurrent);
			// TODO: exit earlier?
	}

	uwPosY += commDrawMultilineText(szMsg, 0, uwPosY) * ubLineHeight;
	char szBfr[250];
	uwPosY += ubLineHeight / 2;
	sprintf(
		szBfr,
		"If you tell the truth, heat will decrease by %d and you will not get a rebuke.\n\n"
		"If you risk lying to commissar, there is %hhu%% chance that you will get caught, "
		"which will result in instantly getting a rebuke and him discovering the truth.",
		QUESTIONING_HEAT_DECREASE_TRUTH,
		heatGetPercent()
	);
	uwPosY += commDrawMultilineText(szBfr,  0, uwPosY) * ubLineHeight;

	buttonInitAcceptDecline("Tell the truth", "Lie");
	buttonDrawAll(commGetDisplayBuffer());
}

void pageQuestioningReset(void) {
	s_eQuestioningsPending = 0;
	s_eQuestioningsReported = 0;
}

void pageQuestioningSave(tFile *pFile) {
	saveWriteHeader(pFile, "QTNG");
	fileWrite(pFile, &s_eQuestioningsPending, sizeof(s_eQuestioningsPending));
	fileWrite(pFile, &s_eQuestioningsReported, sizeof(s_eQuestioningsReported));
}

UBYTE pageQuestioningLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "QTNG")) {
		return 0;
	}

	fileRead(pFile, &s_eQuestioningsPending, sizeof(s_eQuestioningsPending));
	fileRead(pFile, &s_eQuestioningsReported, sizeof(s_eQuestioningsReported));
	return 1;
}

void pageQuestioningTrySetPendingQuestioning(tQuestioningBit eQuestioningBit) {
	tQuestioningBit ePendingQuestioningPrev = s_eQuestioningsPending;

	if(!(s_eQuestioningsReported & eQuestioningBit)) {
		// Not reported yet - increase heat and add as pending questioning.
		// Increases heat each time it's triggered, even when questioning is already pending.
		s_eQuestioningsPending |= eQuestioningBit;
		heatTryIncrease(QUESTIONING_HEAT_INCREASE);
	}

	if(ePendingQuestioningPrev != s_eQuestioningsPending) {
		inboxPushBack(COMM_SHOP_PAGE_OFFICE_KOMISARZ_QUESTIONING, 1);
		hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_WAITING_URZEDAS]);
	}
}

UBYTE pageQuestioningIsReported(tQuestioningBit eQuestioning) {
	return (s_eQuestioningsReported & eQuestioning) != 0;
}
