/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "warehouse.h"
#include "mineral.h"
#include "game.h"
#include "core.h"
#include <ace/macros.h>
#include <ace/managers/log.h>

#define TIME_PER_DAY 140

// Not spent on plan, not sold
static UWORD s_pStock[MINERAL_TYPE_COUNT] = {0};

static tPlan s_sCurrentPlan;
static const tPlan s_sFirstPlan = {
	.pMinerals = {{0}},
	.ulMineralsUnlocked = 1 << MINERAL_TYPE_SILVER,
	.ulTargetSum = 15,
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
	s_sCurrentPlan.pMinerals[ubMineralType].uwCurrentCount = ubCount;
}

UBYTE warehouseIsPlanFulfilled(void) {
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		const tPlanMineral *pMineral = &s_sCurrentPlan.pMinerals[i];
		if(pMineral->uwCurrentCount < pMineral->uwTargetCount) {
			logWrite("Plan not fulfilled\n");
			return 0;
		}
	}
	logWrite("Plan fulfilled\n");
	return 1;
}

UBYTE warehouseTryFulfillPlan(void) {
	UWORD pNewStock[MINERAL_TYPE_COUNT];
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		const tPlanMineral *pMineral = &s_sCurrentPlan.pMinerals[i];
		UWORD uwTotal = pMineral->uwCurrentCount + s_pStock[i];
		if(uwTotal < pMineral->uwTargetCount) {
			return 0;
		}
		else {
			pNewStock[i] = uwTotal - pMineral->uwTargetCount;
		}
	}
	// We can fulfill plan from current stock, so replace it with reduced one
	memcpy(s_pStock, pNewStock, MINERAL_TYPE_COUNT * sizeof(pNewStock[0]));
	return 1;
}

void warehouseNewPlan(UBYTE isBigger, UBYTE is2pPlaying) {
	logBlockBegin(
		"warehouseNewPlan(isBigger: %hhu, is2pPlaying: %hhu)", isBigger, is2pPlaying
	);
	if(isBigger) {
		if(!s_sCurrentPlan.isPenaltyCountdownStarted) {
			gameAddAccolade();
		}
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
		UBYTE ubMineral = randUwMax(&g_sRand, MINERAL_TYPE_COUNT - 1);
		if(s_sCurrentPlan.ulMineralsUnlocked & (1 << ubMineral)) {
			UBYTE ubReward = g_pMinerals[ubMineral].ubReward;
			UWORD uwCount = randUwMax(&g_sRand, (lCostRemaining + ubReward - 1) / ubReward);
			s_sCurrentPlan.pMinerals[ubMineral].uwTargetCount += uwCount;
			lCostRemaining -= uwCount * ubReward;
			if(lCostRemaining <= 0) {
				isDone = 1;
			}
		}
	} while(!isDone);
	s_sCurrentPlan.wTimeMax = 4 * 1000; // Two times fuel capacity for 2p
	s_sCurrentPlan.wTimeMax += 200; // Add for nice division into 30 days
	s_sCurrentPlan.wTimeRemaining = s_sCurrentPlan.wTimeMax;
	s_sCurrentPlan.isExtendedTimeByFavor = 0;
	s_sCurrentPlan.isPenaltyCountdownStarted = 0;
	s_sCurrentPlan.isFailed = 0;
	logBlockEnd("warehouseNewPlan()");
}

void warehousePlanUnlockMineral(tMineralType eMineral) {
	s_sCurrentPlan.ulMineralsUnlocked |= 1 << eMineral;
}

UWORD warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UWORD uwCount) {
	s_pStock[ubMineralType] = uwCount;
}

void warehouseElapseDay(void) {
	warehouseElapseTime(TIME_PER_DAY);
}

void warehouseElapseTime(UWORD uwTime) {
	if(s_sCurrentPlan.isFailed) {
		s_sCurrentPlan.wTimeRemaining = 0;
	}
	else {
		s_sCurrentPlan.wTimeRemaining = MAX(0, s_sCurrentPlan.wTimeRemaining - uwTime);
	}
}

WORD warehouseGetRemainingDays(const tPlan *pPlan) {
	WORD wRemainingDays = (pPlan->wTimeRemaining + TIME_PER_DAY - 1) / TIME_PER_DAY;
	return wRemainingDays;
}

void warehouseAddDaysToPlan(UBYTE ubDays, UBYTE isBribe) {
	if(isBribe) {
		s_sCurrentPlan.isExtendedTimeByFavor = 1;
	}
	s_sCurrentPlan.wTimeRemaining += ubDays * TIME_PER_DAY;
}

UWORD warehouseGetPlanRemainingCost(const tPlan *pPlan) {
	UWORD uwRemainingCost = 0;
	for(UBYTE i = MINERAL_TYPE_COUNT; i--;) {
		const tPlanMineral *pMineral = &pPlan->pMinerals[i];
		UWORD uwCount = pMineral->uwTargetCount - pMineral->uwCurrentCount;
		UBYTE ubReward = g_pMinerals[i].ubReward;

		uwRemainingCost += uwCount * ubReward;
	}
	return uwRemainingCost;
}

void warehouseStartPenaltyCountdown() {
	warehouseAddDaysToPlan(14, 0);
	s_sCurrentPlan.isPenaltyCountdownStarted = 1;
}

void warehouseFailPlan() {
	s_sCurrentPlan.isFailed = 1;
	gameAddRebuke();
}
