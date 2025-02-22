/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "achievement.h"
#include <ace/utils/string.h>
#include "settings.h"
#include "hud.h"
#include <comm/page_accounting.h>
#include <comm/page_bribe.h>
#include <comm/page_favor.h>
#include <comm/page_questioning.h>
#include <comm/page_market.h>

static char s_szAchievementMsgBuffer[50];

void achievementUnlock(tAchievement eAchievement) {
	if(settingsTryUnlockAchievement(eAchievement)) {
		logWrite("Unlocking achievement %hu\n", eAchievement);
		char *pEnd = stringCopy(g_pMsgs[MSG_HUD_ACHIEVEMENT_UNLOCKED], s_szAchievementMsgBuffer);
		*(pEnd++) = ':';
		*(pEnd++) = '\n';
		stringCopy(g_pMsgs[MSG_ACHIEVEMENT_TITLE_LAST_RIGHTEOUS + eAchievement], pEnd);
		hudShowMessage(FACE_ID_MIETEK, s_szAchievementMsgBuffer);
	}
}

UBYTE achievementTryUnlockRighteous(void) {
	if(
		!pageQuestioningIsAnyReported() && pageQuestioningGetLiesCount() == 0 &&
		pageAccountingGetUses() == 0 && pageBribeGetCount() == 0 &&
		pageFavorGetUses() == 0 && pageMarketGetResourcesTraded() == 0
	) {
		return 1;
	}
	return 0;
}

UBYTE achievementIsUnlocked(tAchievement eAchievement) {
	return (g_sSettings.ulAchievementsUnlocked & BV(eAchievement)) != 0;
}
