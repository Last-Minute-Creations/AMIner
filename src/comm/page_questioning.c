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

static tQuestioningBit s_ePendingQuestioning;
static tQuestioningBit s_eQuestioningReports;
static tQuestioningBit s_eCurrentQuestioningBit;

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
			s_eQuestioningReports |= s_eCurrentQuestioningBit;
		}
		else {
			if(heatTryPassCheck()) {
				s_ePendingQuestioning &= ~(s_eCurrentQuestioningBit);
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

	s_eCurrentQuestioningBit = QUESTIONING_BIT_GATE;
	while(s_eCurrentQuestioningBit < QUESTIONING_BIT_END) {
		if(s_eCurrentQuestioningBit & s_ePendingQuestioning) {
			break;
		}
		s_eCurrentQuestioningBit <<= 1;
	}

	const char *szMsg = "???";
	switch (s_eCurrentQuestioningBit)
	{
		case QUESTIONING_BIT_GATE:
			szMsg = "Have you found some gate parts?";
			break;
		case QUESTIONING_BIT_TELEPORT_PARTS:
			szMsg = "Have you found some teleport parts?";
			break;
		default:
			logWrite("ERR: Unhandled questioning value: %d\n", s_eCurrentQuestioningBit);
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
	s_ePendingQuestioning = 0;
	s_eQuestioningReports = 0;
}

void pageQuestioningSave(tFile *pFile) {
	saveWriteHeader(pFile, "QTNG");
	fileWrite(pFile, &s_ePendingQuestioning, sizeof(s_ePendingQuestioning));
	fileWrite(pFile, &s_eQuestioningReports, sizeof(s_eQuestioningReports));
}

UBYTE pageQuestioningLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "QTNG")) {
		return 0;
	}

	fileRead(pFile, &s_ePendingQuestioning, sizeof(s_ePendingQuestioning));
	fileRead(pFile, &s_eQuestioningReports, sizeof(s_eQuestioningReports));
	return 1;
}

void pageQuestioningTrySetPendingQuestioning(tQuestioningBit eQuestioningBit) {
	tQuestioningBit ePendingQuestioningPrev = s_ePendingQuestioning;

	if(!(s_eQuestioningReports & eQuestioningBit)) {
		// Not reported yet - increase heat and add as pending questioning.
		// Increases heat each time it's triggered, even when questioning is already pending.
		s_ePendingQuestioning |= eQuestioningBit;
		heatTryIncrease(QUESTIONING_HEAT_INCREASE);
	}

	if(ePendingQuestioningPrev != s_ePendingQuestioning) {
		inboxPushBack(COMM_SHOP_PAGE_OFFICE_KOMISARZ_QUESTIONING, 1);
		hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_WAITING_URZEDAS]);
	}
}

tQuestioningBit pageQuestioningGetQuestioningReports(void) {
	return s_eQuestioningReports;
}
