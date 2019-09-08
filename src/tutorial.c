/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "game.h"
#include <ace/managers/game.h>
#include "comm_shop.h"
#include "comm_msg.h"
#include "warehouse.h"
#include "vehicle.h"
#include "hud.h"

typedef enum _tTutorialState {
	TUTORIAL_SHOW_MESSAGE_INTRO = 0,
	TUTORIAL_SHOW_MESSAGE_TO_DIG,
	TUTORIAL_WAITING_FOR_DIG,
	TUTORIAL_WAITING_FOR_RESTOCK,
	TUTORIAL_SHOW_PLAN_FILL_MSG,
	TUTORIAL_WAITING_FOR_PLAN_DONE,
	TUTORIAL_DONE
} tTutorialState;

tTutorialState s_eTutorialState = TUTORIAL_SHOW_MESSAGE_INTRO;

void tutorialReset(void) {
	s_eTutorialState = TUTORIAL_SHOW_MESSAGE_INTRO;
}

UBYTE tutorialProcess(void) {
	UBYTE isEarlyReturn = 0;
	switch(s_eTutorialState) {
		case TUTORIAL_SHOW_MESSAGE_INTRO:
			gamePushState(commMsgGsCreate, commMsgGsLoop, commMsgGsDestroy);
			++s_eTutorialState;
			isEarlyReturn = 1;
			break;
		case TUTORIAL_SHOW_MESSAGE_TO_DIG:
			hudShowMessage(0, "Comrade, go dig 3 ores of silver!");
			++s_eTutorialState;
			break;
		case TUTORIAL_WAITING_FOR_DIG:
			if(g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] + g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] >= 3) {
				hudShowMessage(0,
					"Your drill gets damaged and cargo bay filled as you dig.\n"
					"Get to the warehouse to leave minerals and replace drill."
				);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_RESTOCK:
			if(g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] + g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] == 0) {
				hudShowMessage(0,
					"Comrade, press ENTER or FIRE or SPACE to enter shop.\n"
					"You can sell ore or use it for fulfilling plans."
				);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_SHOW_PLAN_FILL_MSG:
			if(commShopIsActive()) {
				hudShowMessage(0,
					"You need to spend 3 silver to fulfill the plan. Navigate to it\n"
					"and move minerals to plan column, then press Confirm."
				);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_PLAN_DONE:
			if(warehouseGetPlan()->pMinerals[MINERAL_TYPE_SILVER].uwTargetCount != 3) {
				hudShowMessage(0,
					"Congratulations comrade, you've fulfilled your first plan.\n"
					"Keep doing it and surely you will be promoted!"
				);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_DONE:
			break;
	}
	return isEarlyReturn;
}
