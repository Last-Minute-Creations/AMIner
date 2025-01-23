/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_questioning.h"
#include <comm/comm.h>
#include <comm/page_office.h>
#include <comm/button.h>
#include <comm/inbox.h>
#include "../save.h"
#include "../heat.h"
#include "../game.h"
#include "../hud.h"

#define QUESTIONING_HEAT_INCREASE 5
#define QUESTIONING_HEAT_DECREASE_TRUTH 5
#define QUESTIONING_HEAT_DECREASE_REPORT 100

static tQuestioningFlag s_eQuestioningsPending;
static tQuestioningFlag s_eQuestioningsReported;
static tQuestioningFlag s_eQuestioningsNotReported; // for future reporting at will
static tQuestioningBit s_eQuestioningBitCurrent;
static tQuestioningHandler s_pQuestioningHandlers[QUESTIONING_BIT_COUNT] = {0};
static tCommShopPage s_pOfficeQuestioningPages[PAGE_OFFICE_SUBPAGES_PER_PERSON];

static tCommShopPage pageQuestioningBitToShopPage(tQuestioningBit eBit) {
	static const tCommShopPage pQuestioningPages[QUESTIONING_BIT_COUNT] = {
		COMM_SHOP_PAGE_OFFICE_KOMISARZ_REPORTING_GATE,
		COMM_SHOP_PAGE_OFFICE_KOMISARZ_REPORTING_TELEPORT_PARTS,
		COMM_SHOP_PAGE_OFFICE_KOMISARZ_REPORTING_AGENT,
	};
	return pQuestioningPages[eBit];
}

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
		UBYTE isShowReportMessage = 0;
		if(bButtonCurr == 0) {
			// Told the truth
			heatTryReduce(QUESTIONING_HEAT_DECREASE_TRUTH);
			pageQuestioningReport(s_eQuestioningBitCurrent, 0);
			isShowReportMessage = 1;
		}
		else {
			if(heatTryPassCheck()) {
				s_eQuestioningsNotReported |= BV(s_eQuestioningBitCurrent);
			}
			else {
				// Got caught
				gameAddRebuke();
				pageQuestioningReport(s_eQuestioningBitCurrent, 0);
				isShowReportMessage = 1;
			}
		}

		// Questioning has ended - clear pending and launch callback
		s_eQuestioningsPending &= ~BV(s_eQuestioningBitCurrent);
		if(s_pQuestioningHandlers[s_eQuestioningBitCurrent]) {
			s_pQuestioningHandlers[s_eQuestioningBitCurrent](
				s_eQuestioningBitCurrent, pageQuestioningIsReported(s_eQuestioningBitCurrent)
			);
		}

		if(isShowReportMessage) {
			tCommShopPage eNextPage = pageQuestioningBitToShopPage(s_eQuestioningBitCurrent);
			commShopChangePage(COMM_SHOP_PAGE_OFFICE_MAIN, eNextPage);
		}
		else {
			commShopGoBack();
		}
	}
}

void pageQuestioningCreate(void) {
	commRegisterPage(pageQuestioningProcess, 0);
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = commGetLineHeight();

	s_eQuestioningBitCurrent = QUESTIONING_BIT_GATE;
	while(s_eQuestioningBitCurrent < QUESTIONING_BIT_COUNT) {
		if(s_eQuestioningsPending & BV(s_eQuestioningBitCurrent)) {
			break;
		}
		++s_eQuestioningBitCurrent;
	}

	if(s_eQuestioningBitCurrent >= QUESTIONING_BIT_COUNT) {
		commShopGoBack();
		return;
	}

	const char *szMsg = "???";
	switch (s_eQuestioningBitCurrent)
	{
		case QUESTIONING_BIT_GATE:
			szMsg = "Have you found some gate parts?";
			break;
		case QUESTIONING_BIT_TELEPORT_PARTS:
			szMsg = "Have you found some teleport parts?";
			break;
		default:
			logWrite("ERR: Unhandled questioning value: %d\n", s_eQuestioningBitCurrent);
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
	s_eQuestioningsNotReported = 0;
}

void pageQuestioningSave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_QUESTIONING);
	fileWrite(pFile, &s_eQuestioningsPending, sizeof(s_eQuestioningsPending));
	fileWrite(pFile, &s_eQuestioningsReported, sizeof(s_eQuestioningsReported));
	fileWrite(pFile, &s_eQuestioningsNotReported, sizeof(s_eQuestioningsNotReported));
	saveWriteTag(pFile, SAVE_TAG_QUESTIONING_END);
}

UBYTE pageQuestioningLoad(tFile *pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_QUESTIONING)) {
		return 0;
	}

	fileRead(pFile, &s_eQuestioningsPending, sizeof(s_eQuestioningsPending));
	fileRead(pFile, &s_eQuestioningsReported, sizeof(s_eQuestioningsReported));
	fileRead(pFile, &s_eQuestioningsNotReported, sizeof(s_eQuestioningsNotReported));
	return saveReadTag(pFile, SAVE_TAG_QUESTIONING_END);
}

void pageQuestioningTrySetPendingQuestioning(tQuestioningBit eQuestioningBit, UBYTE isForce) {
	tQuestioningFlag ePendingQuestioningPrev = s_eQuestioningsPending;

	if(isForce || !pageQuestioningIsReported(eQuestioningBit)) {
		// Not reported yet - increase heat and add as pending questioning.
		// Increases heat each time it's triggered, even when questioning is already pending.
		s_eQuestioningsPending |= BV(eQuestioningBit);
		heatTryIncrease(QUESTIONING_HEAT_INCREASE);
		pageOfficeTryUnlockPersonSubpage(FACE_ID_KOMISARZ, COMM_SHOP_PAGE_OFFICE_KOMISARZ_REPORTING_LIST);
	}

	if(ePendingQuestioningPrev != s_eQuestioningsPending) {
		inboxPushBack(COMM_SHOP_PAGE_OFFICE_KOMISARZ_QUESTIONING, 1);
		hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_WAITING_KOMISARZ]);
	}
}

void pageQuestioningTryCancelPendingQuestioning(tQuestioningBit eQuestioningBit) {
	s_eQuestioningsPending &= ~BV(eQuestioningBit);
}

UBYTE pageQuestioningIsReported(tQuestioningBit eQuestioningBit) {
	return (s_eQuestioningsReported & BV(eQuestioningBit)) != 0;
}

void pageQuestioningSetHandler(
	tQuestioningBit eQuestioningBit, tQuestioningHandler cbOnQuestioningEnded
) {
	s_pQuestioningHandlers[eQuestioningBit] = cbOnQuestioningEnded;
}

const tCommShopPage *pageQuestioningGetNotReportedPages(void) {
	UBYTE ubPos = 0;
	for(tQuestioningBit eBit = 0; eBit < QUESTIONING_BIT_COUNT; ++eBit) {
		if(s_eQuestioningsNotReported & BV(eBit)) {
			s_pOfficeQuestioningPages[ubPos] = pageQuestioningBitToShopPage(eBit);
			++ubPos;
		}
	}

	s_pOfficeQuestioningPages[ubPos] = COMM_SHOP_PAGE_OFFICE_MAIN; // Serves as list terminator
	return s_pOfficeQuestioningPages;
}

void pageQuestioningReport(tQuestioningBit eQuestioningBit, UBYTE isVoluntarily) {
	s_eQuestioningsReported |= BV(eQuestioningBit);
	s_eQuestioningsNotReported &= ~BV(eQuestioningBit);
	s_eQuestioningsPending &= ~BV(eQuestioningBit);

	if(isVoluntarily) {
		heatTryReduce(QUESTIONING_HEAT_DECREASE_REPORT);
		gameAddAccolade();
	}
}

void pageQuestioningAddReporting(tQuestioningBit eQuestioningBit) {
	s_eQuestioningsNotReported |= BV(eQuestioningBit);
}
