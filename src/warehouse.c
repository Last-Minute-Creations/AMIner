/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "warehouse.h"
#include "mineral.h"
#include <ace/macros.h>
#include <ace/managers/log.h>
#include <ace/managers/rand.h>

// Not spent on plan, not sold
static UBYTE s_pStock[MINERAL_TYPE_COUNT] = {0};

static tPlan s_sCurrentPlan;
static const tPlan s_sFirstPlan = {
	.pMinerals = {{0}},
	.ulMineralsUnlocked = 1 << MINERAL_TYPE_SILVER,
	.ulTargetSum = 100
};

void warehouseReset(UBYTE is2pPlaying) {
	memset(s_pStock, 0, sizeof(s_pStock));
	s_sCurrentPlan = s_sFirstPlan;
	warehouseNewPlan(0, is2pPlaying);
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

void warehouseNewPlan(UBYTE isBigger, UBYTE is2pPlaying) {
	if(isBigger) {
		if(is2pPlaying) {
			s_sCurrentPlan.ulTargetSum += s_sCurrentPlan.ulTargetSum * 3 / 4;
		}
		else {
			s_sCurrentPlan.ulTargetSum += s_sCurrentPlan.ulTargetSum * 1 / 2;
		}
	}
	memset(
		s_sCurrentPlan.pMinerals, 0, sizeof(s_sCurrentPlan.pMinerals)
	);
	UBYTE isDone = 0;
	LONG lCostRemaining = s_sCurrentPlan.ulTargetSum;
	do {
		UBYTE ubMineral = ubRandMax(MINERAL_TYPE_COUNT - 1);
		if(s_sCurrentPlan.ulMineralsUnlocked & (1 << ubMineral)) {
			UWORD uwCount = uwRandMax(lCostRemaining / g_pMinerals[ubMineral].ubReward);
			s_sCurrentPlan.pMinerals[ubMineral].ubTargetCount += uwCount;
			lCostRemaining -= uwCount * g_pMinerals[ubMineral].ubReward;
			if(lCostRemaining <= 0) {
				isDone = 1;
			}
		}
	} while(!isDone);
	s_sCurrentPlan.wTimeMax = 4 * 1000; // Two times fuel capacity for 2p
	s_sCurrentPlan.wTimeRemaining = s_sCurrentPlan.wTimeMax;
}

void warehousePlanUnlockMineral(tMineralType eMineral) {
	s_sCurrentPlan.ulMineralsUnlocked |= 1 << eMineral;
}

UBYTE warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UBYTE ubCount) {
	s_pStock[ubMineralType] = ubCount;
}

void warehouseElapseTime(UBYTE ubTime) {
	s_sCurrentPlan.wTimeRemaining = MAX(0, s_sCurrentPlan.wTimeRemaining - ubTime);
}
