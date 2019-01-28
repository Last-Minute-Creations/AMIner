/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PLAN_H_
#define _PLAN_H_

#include <ace/types.h>
#include "mineral.h"

typedef struct _tPlanMineral {
	UBYTE ubTargetCount;
	UBYTE ubCurrentCount;
} tPlanMineral;

typedef struct _tPlan {
	tPlanMineral pMinerals[MINERAL_TYPE_COUNT];
	ULONG ulTargetSum;
	ULONG ulCurrentSum;
} tPlan;

const tPlan *planGetCurrent(void);

void planAddMinerals(UBYTE ubMineralType, UBYTE ubCount);

UBYTE planIsFulfilled(void);

void planGenerateNew(void);

#endif // _PLAN_H_
