/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "warehouse.h"
#include "mineral.h"
#include <ace/macros.h>
#include <ace/managers/log.h>

// Not spent on plan, not sold
static UBYTE s_pStock[MINERAL_TYPE_COUNT] = {0};

static tPlan s_sCurrentPlan;
static const tPlan s_sFirstPlan = {
	.pMinerals = { {0},
		[MINERAL_TYPE_SILVER] = {.ubTargetCount = 3, .ubCurrentCount = 0},
		[MINERAL_TYPE_GOLD] = {.ubTargetCount = 1, .ubCurrentCount = 0}
	},
	.ulTargetSum = 100000
};

void warehouseReset(void) {
	memset(s_pStock, 0, sizeof(s_pStock));
	s_sCurrentPlan = s_sFirstPlan;
}

const tPlan *warehouseGetPlan(void) {
	return &s_sCurrentPlan;
}

void warehouseReserveMineralsForPlan(UBYTE ubMineralType, UBYTE ubCount) {
	s_sCurrentPlan.pMinerals[ubMineralType].ubCurrentCount = ubCount;
}

UBYTE warehouseIsPlanFulfilled(void) {
	UBYTE hasAllMinerals = 1;
	ULONG ulSum = 0;
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		ulSum += g_pMinerals[i].ubReward * s_sCurrentPlan.pMinerals[i].ubCurrentCount;
		if(s_sCurrentPlan.pMinerals[i].ubCurrentCount < s_sCurrentPlan.pMinerals[i].ubTargetCount) {
			hasAllMinerals = 0;
		}
	}

	UBYTE hasSum = 0; //(ulSum == s_sCurrentPlan.ulTargetSum);
	return hasAllMinerals || hasSum;
}

void warehouseNewPlan(void) {
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		tPlanMineral *pMineral = &s_sCurrentPlan.pMinerals[i];
		if(pMineral->ubCurrentCount) {
			pMineral->ubTargetCount = pMineral->ubCurrentCount + pMineral->ubCurrentCount / 2;
			pMineral->ubCurrentCount = 0;
		}
	}
	s_sCurrentPlan.ulTargetSum += s_sCurrentPlan.ulTargetSum / 2;
}

UBYTE warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UBYTE ubCount) {
	s_pStock[ubMineralType] = ubCount;
}
