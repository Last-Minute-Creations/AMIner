/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _WAREHOUSE_H_
#define _WAREHOUSE_H_

#include "plan.h"

void warehouseReserveMineralsForPlan(UBYTE ubMineralType, UBYTE ubCount);

UWORD warehouseGetStock(UBYTE ubMineralType);

void warehouseSetStock(UBYTE ubMineralType, UWORD uwCount);

void warehouseReset(void);

tPlan *warehouseGetCurrentPlan(void);

//------------------------------------------------------------------------- PLAN

void warehouseNextPlan(void);

void warehouseRerollPlan(void);

UWORD planGetRemainingCost(const tPlan *pPlan);

#endif // _WAREHOUSE_H_
