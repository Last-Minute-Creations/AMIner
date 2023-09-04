/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "warehouse.h"
#include <ace/macros.h>
#include <ace/managers/log.h>
#include <fixmath/fix16.h>
#include "mineral.h"
#include "game.h"
#include "core.h"
#include "defs.h"
#include "comm/page_accounting.h"
#include "save.h"
#include "heat.h"

// Not spent on plan, not sold
static UWORD s_pStock[MINERAL_TYPE_COUNT] = {0};

void warehouseReset(void) {
	memset(s_pStock, 0, sizeof(s_pStock));
	planManagerInit();
}

void warehouseSave(tFile *pFile) {
	saveWriteHeader(pFile, "WHSE");
	fileWrite(pFile, s_pStock, sizeof(s_pStock[0]) * MINERAL_TYPE_COUNT);
	planManagerSave(pFile);
}

UBYTE warehouseLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "WHSE")) {
		return 0;
	}

	fileRead(pFile, s_pStock, sizeof(s_pStock[0]) * MINERAL_TYPE_COUNT);
	return planManagerLoad(pFile);
}

void warehouseNextPlan(tNextPlanReason eReason) {
	if(eReason != NEXT_PLAN_REASON_FAILED) {
		if(!planManagerGet()->isPenaltyCountdownStarted) {
			gameAdvanceAccolade();
		}

		if(eReason == NEXT_PLAN_REASON_FULFILLED_ACCOUNTING) {
			heatTryReduce(2);
		}
	}

	// Increase the plan target sum by relevant ratio
	planAdvance();
}

UWORD warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UWORD uwCount) {
	s_pStock[ubMineralType] = uwCount;
}
