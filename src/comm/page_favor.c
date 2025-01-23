/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_favor.h"
#include <comm/comm.h>
#include <comm/button.h>
#include <comm/page_office.h>
#include <comm/gs_shop.h>
#include "../game.h"
#include "../plan.h"
#include "../save.h"

static UBYTE s_ubFavorsLeft;
static UBYTE s_ubFavorsUses;

static void pageFavorProcess(void) {
	if(buttonGetPreset() == BUTTON_PRESET_ACCEPT_DECLINE) {
		BYTE bButtonPrev = buttonGetSelected();
		BYTE bButtonCurr = bButtonPrev;
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
				--s_ubFavorsLeft;
				++s_ubFavorsUses;
				planReroll();
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

void pageFavorCreate(void) {
	commRegisterPage(pageFavorProcess, 0);
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = commGetLineHeight();
	WORD wDays = planGetRemainingDays();
	if (!planManagerGet()->isPlanActive) {
		uwPosY += commDrawMultilineText(
			"You have no active plan! What do you want me to do?", 0, uwPosY
		) * ubLineHeight;
		buttonInitOk(g_pMsgs[MSG_PAGE_BACK]);
	}
	else if(s_ubFavorsLeft > 0 && wDays >= 15) {
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
	}
	else {
		uwPosY += commDrawMultilineText("You ask me for too much, Comrade. Do some real work, will you?", 0, uwPosY) * ubLineHeight;
		buttonInitOk(g_pMsgs[MSG_PAGE_BACK]);
	}

	buttonDrawAll(commGetDisplayBuffer());
}

void pageFavorReset(void) {
	s_ubFavorsLeft = 10;
	s_ubFavorsUses = 0;
}

void pageFavorSave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_FAVOR);
	fileWrite(pFile, &s_ubFavorsLeft, sizeof(s_ubFavorsLeft));
	fileWrite(pFile, &s_ubFavorsUses, sizeof(s_ubFavorsUses));
	saveWriteTag(pFile, SAVE_TAG_FAVOR_END);
}

UBYTE pageFavorLoad(tFile *pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_FAVOR)) {
		return 0;
	}
	fileRead(pFile, &s_ubFavorsLeft, sizeof(s_ubFavorsLeft));
	fileRead(pFile, &s_ubFavorsUses, sizeof(s_ubFavorsUses));
	return saveReadTag(pFile, SAVE_TAG_FAVOR_END);
}

UBYTE pageFavorGetUses(void) {
	return s_ubFavorsUses;
}
