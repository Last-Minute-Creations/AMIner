/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "plan.h"
#include "mineral.h"
#include <ace/macros.h>
#include <ace/managers/log.h>

tPlan s_sCurrentPlan = {
	.pMinerals = { {0},
		[MINERAL_TYPE_SILVER] = {.ubTargetCount = 3, .ubCurrentCount = 0},
		[MINERAL_TYPE_GOLD] = {.ubTargetCount = 1, .ubCurrentCount = 0}
	},
	.ulTargetSum = 100000
};

const tPlan *planGetCurrent(void) {
	return &s_sCurrentPlan;
}

void planAddMinerals(UBYTE ubMineralType, UBYTE ubCount) {
	s_sCurrentPlan.pMinerals[ubMineralType].ubCurrentCount += ubCount;
	s_sCurrentPlan.ulCurrentSum += g_pMinerals[ubMineralType].ubReward * ubCount;
}

UBYTE planIsFulfilled(void) {
	UBYTE hasAllMinerals = 1;
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		if(s_sCurrentPlan.pMinerals[i].ubCurrentCount < s_sCurrentPlan.pMinerals[i].ubTargetCount) {
			hasAllMinerals = 0;
			break;
		}
	}

	UBYTE hasSum = (
		s_sCurrentPlan.ulCurrentSum == s_sCurrentPlan.ulTargetSum
	);

	return hasAllMinerals || hasSum;
}

void planGenerateNew(void) {
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		tPlanMineral *pMineral = &s_sCurrentPlan.pMinerals[i];
		if(pMineral->ubCurrentCount) {
			pMineral->ubTargetCount = pMineral->ubCurrentCount + pMineral->ubCurrentCount / 2;
			pMineral->ubCurrentCount = 0;
		}
	}
	s_sCurrentPlan.ulTargetSum += s_sCurrentPlan.ulTargetSum / 2;
	s_sCurrentPlan.ulCurrentSum = 0;
}
