/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _WAREHOUSE_H_
#define _WAREHOUSE_H_

#include <ace/types.h>
#include "mineral.h"

typedef struct _tPlanMineral {
	UBYTE ubTargetCount;
	UBYTE ubCurrentCount;
} tPlanMineral;

typedef struct _tPlan {
	tPlanMineral pMinerals[MINERAL_TYPE_COUNT];
	ULONG ulMineralsUnlocked; ///< Acts as bitfield
	ULONG ulTargetSum;
	WORD wTimeMax;
	WORD wTimeRemaining;
} tPlan;

const tPlan *warehouseGetPlan(void);

void warehouseReserveMineralsForPlan(UBYTE ubMineralType, UBYTE ubCount);

UBYTE warehouseIsPlanFulfilled(void);

void warehouseNewPlan(UBYTE isBigger, UBYTE is2pPlaying);

UBYTE warehouseGetStock(UBYTE ubMineralType);

void warehouseSetStock(UBYTE ubMineralType, UBYTE ubCount);

void warehouseReset(UBYTE is2pPlaying);

void warehouseElapseTime(UBYTE ubTime);

void warehousePlanUnlockMineral(tMineralType eMineral);

#endif // _WAREHOUSE_H_
