/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "game.h"
#include <comm/gs_msg.h>
#include <comm/gs_shop.h>
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
	UBYTE isEarlyReturn = 0;
	switch(s_eTutorialState) {
		case TUTORIAL_SHOW_MESSAGE_INTRO:
			gsMsgInit("intro_campaign");
			statePush(g_pGameStateManager, &g_sStateMsg);
			++s_eTutorialState;
			isEarlyReturn = 1;
			break;
		case TUTORIAL_SHOW_MESSAGE_TO_DIG:
			hudShowMessage(0, g_pMsgs[MSG_TUTORIAL_GO_DIG]);
			++s_eTutorialState;
			break;
		case TUTORIAL_WAITING_FOR_DIG:
			if(g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] + g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] >= 3) {
				hudShowMessage(0, g_pMsgs[MSG_TUTORIAL_ON_DUG]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_RESTOCK:
			if(g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] + g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] == 0) {
				hudShowMessage(0, g_pMsgs[MSG_TUTORIAL_NEAR_SHOP]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_SHOW_PLAN_FILL_MSG:
			if(commShopIsActive()) {
				hudShowMessage(0, g_pMsgs[MSG_TUTORIAL_IN_SHOP]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_PLAN_DONE:
			// Assume the plan is fulfilled when the target count of silver has changed
			if(warehouseGetPlan()->pMinerals[MINERAL_TYPE_SILVER].uwTargetCount != 3) {
				hudShowMessage(0, g_pMsgs[MSG_TUTORIAL_ON_MOVE_TO_PLAN]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_DONE:
			break;
	}
	return isEarlyReturn;
}

void tutorialReset(void) {
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
