/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "plan.h"
#include <ace/macros.h>
#include <ace/managers/log.h>
#include "core.h"

#define PLAN_TIME_PER_DAY 140

UBYTE planIsFulfilled(const tPlan *pPlan) {
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		const tPlanMineral *pMineral = &pPlan->pMinerals[i];
		if(pMineral->uwCurrentCount < pMineral->uwTargetCount) {
			return 0;
		}
	}
	return 1;
}

void planElapseDay(tPlan *pPlan) {
	planElapseTime(pPlan, PLAN_TIME_PER_DAY);
}

void planElapseTime(tPlan *pPlan, UWORD uwTime) {
	if(pPlan->isFailed) {
		pPlan->wTimeRemaining = 0;
	}
	else {
		pPlan->wTimeRemaining = MAX(0, pPlan->wTimeRemaining - uwTime);
	}
}

void planUnlockMineral(tPlan *pPlan, tMineralType eMineral) {
	pPlan->ulMineralsUnlocked |= 1 << eMineral;
}

WORD planGetRemainingDays(const tPlan *pPlan) {
	WORD wRemainingDays = (pPlan->wTimeRemaining + PLAN_TIME_PER_DAY - 1) / PLAN_TIME_PER_DAY;
	return wRemainingDays;
}

void planAddDays(tPlan *pPlan, UBYTE ubDays, UBYTE isFavor) {
	if(isFavor) {
		pPlan->isExtendedTimeByFavor = 1;
	}
	pPlan->wTimeRemaining += ubDays * PLAN_TIME_PER_DAY;
}

void planStartPenaltyCountdown(tPlan *pPlan) {
	planAddDays(pPlan, 14, 0);
	pPlan->isPenaltyCountdownStarted = 1;
}

void planFail(tPlan *pPlan) {
	pPlan->isFailed = 1;
}

UWORD planGetRemainingCost(const tPlan *pPlan) {
	UWORD uwRemainingCost = 0;
	for(UBYTE i = MINERAL_TYPE_COUNT; i--;) {
		const tPlanMineral *pMineral = &pPlan->pMinerals[i];
		UWORD uwCount = pMineral->uwTargetCount - pMineral->uwCurrentCount;
		UBYTE ubReward = g_pMinerals[i].ubReward;

		uwRemainingCost += uwCount * ubReward;
	}
	return uwRemainingCost;
}

void planReset(tPlan *pPlan) {
	logBlockBegin("planReset()");
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		pPlan->pMinerals[i] = (tPlanMineral){0};
	};
	UBYTE isDone = 0;
	LONG lCostRemaining = pPlan->ulTargetSum;
	do {
		UBYTE ubMineral = randUwMax(&g_sRand, MINERAL_TYPE_COUNT - 1);
		if(pPlan->ulMineralsUnlocked & (1 << ubMineral)) {
			UBYTE ubReward = g_pMinerals[ubMineral].ubReward;
			UWORD uwCount = randUwMax(&g_sRand, (lCostRemaining + ubReward - 1) / ubReward);
			pPlan->pMinerals[ubMineral].uwTargetCount += uwCount;
			lCostRemaining -= uwCount * ubReward;
			if(lCostRemaining <= 0) {
				isDone = 1;
			}
		}
	} while(!isDone);
	pPlan->wTimeMax = 4 * 1000; // Two times fuel capacity for 2p
	pPlan->wTimeMax += 200; // Add for nice division into 30 days
	pPlan->wTimeRemaining = pPlan->wTimeMax;
	pPlan->isExtendedTimeByFavor = 0;
	pPlan->isPenaltyCountdownStarted = 0;
	pPlan->isFailed = 0;
	logBlockEnd("planReset()");
}
