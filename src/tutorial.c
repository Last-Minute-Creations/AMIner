/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "game.h"
#include <comm/gs_msg.h>
#include <comm/gs_shop.h>
#include <comm/inbox.h>
#include "warehouse.h"
#include "vehicle.h"
#include "hud.h"
#include "defs.h"

typedef enum _tTutorialState {
	TUTORIAL_SHOW_MESSAGE_INTRO = 0,
	TUTORIAL_SHOW_MESSAGE_TO_DIG,
	TUTORIAL_WAITING_FOR_DIG,
	TUTORIAL_WAITING_FOR_RESTOCK,
	TUTORIAL_SHOW_PLAN_FILL_MSG,
	TUTORIAL_WAITING_FOR_PLAN_DONE,
	TUTORIAL_DONE
} tTutorialState;

static tTutorialState s_eTutorialState = TUTORIAL_SHOW_MESSAGE_INTRO;
static UBYTE s_pDescriptionShownForTabs[COMM_TAB_COUNT];

static UBYTE tutorialProcessChallenge(void) {
	UBYTE isEarlyReturn = 0;
	switch(s_eTutorialState) {
		case TUTORIAL_SHOW_MESSAGE_INTRO:
			gsMsgInit("intro_challenge");
			statePush(g_pGameStateManager, &g_sStateMsg);
			s_eTutorialState = TUTORIAL_DONE;
			isEarlyReturn = 1;
			break;
		case TUTORIAL_DONE:
			break;
		default:
			break;
	}

	return isEarlyReturn;
}

static UBYTE tutorialProcessStory(void) {
	if(commShopIsActive()) {
		tCommShopPage ePage = commShopGetCurrentPage();
		tCommTab eTab = commShopPageToTab(ePage);
		if(eTab != COMM_TAB_COUNT) {
			if(!s_pDescriptionShownForTabs[eTab]) {
				if(!hudIsShowingMessage()) {
					s_pDescriptionShownForTabs[eTab] = 1;
					hudShowMessage(
						FACE_ID_MIETEK,
						g_pMsgs[MSG_TUTORIAL_DESCRIPTION_TAB_OFFICE + eTab - COMM_TAB_OFFICE]
					);
				}
			}
		}
	}

	UBYTE isEarlyReturn = 0;
	switch(s_eTutorialState) {
		case TUTORIAL_SHOW_MESSAGE_INTRO:
			gsMsgInit("intro_campaign");
			statePush(g_pGameStateManager, &g_sStateMsg);
			++s_eTutorialState;
			isEarlyReturn = 1;
			break;
		case TUTORIAL_SHOW_MESSAGE_TO_DIG:
			hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_GO_MEET_MIETEK]);
			inboxPushBack(COMM_SHOP_PAGE_OFFICE_MIETEK_WELCOME, 0);
			++s_eTutorialState;
			break;
		case TUTORIAL_WAITING_FOR_DIG:
			if(g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] + g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] >= 3) {
				hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_ON_DUG]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_RESTOCK:
			if(g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] + g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] == 0) {
				hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_NEAR_SHOP]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_SHOW_PLAN_FILL_MSG:
			if(commShopIsActive()) {
				hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_IN_SHOP]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_PLAN_DONE:
			// Assume the plan is fulfilled when the target count of silver has changed
			if(warehouseGetCurrentPlan()->pMinerals[MINERAL_TYPE_SILVER].uwTargetCount != 3) {
				hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_ON_MOVE_TO_PLAN]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_DONE:
			break;
	}
	return isEarlyReturn;
}

void tutorialReset(void) {
	for(UBYTE i = 0; i < COMM_TAB_COUNT; ++i) {
		s_pDescriptionShownForTabs[i] = 0;
	}

	s_eTutorialState = TUTORIAL_SHOW_MESSAGE_INTRO;
}

UBYTE tutorialProcess(void) {
	if(g_isChallenge) {
		return tutorialProcessChallenge();
	}
	else {
		return tutorialProcessStory();
	}
}
