/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <comm/page_office.h>
#include <comm/button.h>
#include "../vehicle.h"
#include "../quest_crate.h"
#include "../hud.h"

static void pageEscapeProcess(void) {
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
			commShopChangePage(COMM_SHOP_PAGE_OFFICE_MAIN, COMM_SHOP_PAGE_NEWS_ESCAPE_AGENT);
		}
		else {
			commShopGoBack();
		}
	}
}

void pageEscapeCreate(void) {
	commRegisterPage(pageEscapeProcess, 0);
	const UBYTE ubLineHeight = commGetLineHeight();
	UWORD uwPosY = 0;

	uwPosY += commDrawMultilineText(
		"Za Twoje zaslugi mozemy Cie przetransportowac na zachod. Daj znac jak bedziesz gotowy.", 0, uwPosY
	) * ubLineHeight;

	buttonInitAcceptDecline("Uciekaj", "Powrot");
	buttonDrawAll(commGetDisplayBuffer());
}
