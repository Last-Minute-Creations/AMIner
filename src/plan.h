/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PLAN_H_
#define _PLAN_H_

#include "mineral.h"

typedef struct _tPlanMineral {
	UWORD uwTargetCount;
	UWORD uwCurrentCount;
} tPlanMineral;

typedef struct _tPlan {
	tPlanMineral pMinerals[MINERAL_TYPE_COUNT];
	ULONG ulMineralsUnlocked; ///< Acts as bitfield
	ULONG ulTargetSum;
	WORD wTimeMax;
	WORD wTimeRemaining;
	UWORD uwIndex;
	UBYTE isExtendedTimeByFavor;
	UBYTE isPenaltyCountdownStarted;
	UBYTE isActive;
} tPlan;

UBYTE planIsFulfilled(const tPlan *pPlan);

void planElapseTime(tPlan *pPlan, UWORD uwTime);

void planStart(tPlan *pPlan);

void planUnlockMineral(tPlan *pPlan, tMineralType eMineral);

WORD planGetRemainingDays(const tPlan *pPlan);

void planAddDays(tPlan *pPlan, UBYTE ubDays, UBYTE isFavor);

void planStartPenaltyCountdown(tPlan *pPlan);

void planFail(tPlan *pPlan);

UWORD planGetRemainingCost(const tPlan *pPlan);

void planReset(tPlan *pPlan, UBYTE isActive, UBYTE isNext);

#endif // _PLAN_H_
