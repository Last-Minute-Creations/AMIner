/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "achievement.h"
#include "settings.h"
#include "hud.h"
#include <comm/page_accounting.h>
#include <comm/page_bribe.h>
#include <comm/page_favor.h>
#include <comm/page_questioning.h>
#include <comm/page_market.h>

void achievementUnlock(tAchievement eAchievement) {
	if(settingsTryUnlockAchievement(eAchievement)) {
		logWrite("Unlocking achievement %hu\n", eAchievement);
		hudShowMessage(FACE_ID_MIETEK, "ACHIEVEMENT UNLOCKED!");
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
