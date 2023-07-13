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

static tPlan s_sCurrentPlan;
static const tPlan s_sFirstPlan = {
	.pMinerals = {{0}},
	.ulMineralsUnlocked = 1 << MINERAL_TYPE_SILVER,
	.ulTargetSum = 15,
	.uwIndex = 0,
	.isActive = 0,
};

void warehouseReset(void) {
	memset(s_pStock, 0, sizeof(s_pStock));
	s_sCurrentPlan = s_sFirstPlan;
	planReset(&s_sCurrentPlan, 0, 0);
}

void warehouseSave(tFile *pFile) {
	saveWriteHeader(pFile, "WHSE");
	fileWrite(pFile, s_pStock, sizeof(s_pStock[0]) * MINERAL_TYPE_COUNT);
	fileWrite(pFile, &s_sCurrentPlan, sizeof(s_sCurrentPlan));
}

UBYTE warehouseLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "WHSE")) {
		return 0;
	}

	fileRead(pFile, s_pStock, sizeof(s_pStock[0]) * MINERAL_TYPE_COUNT);
	fileRead(pFile, &s_sCurrentPlan, sizeof(s_sCurrentPlan));
	return 1;
}

tPlan *warehouseGetCurrentPlan(void) {
	return &s_sCurrentPlan;
}

void warehouseReserveMineralsForPlan(UBYTE ubMineralType, UBYTE ubCount) {
	s_sCurrentPlan.pMinerals[ubMineralType].uwCurrentCount = ubCount;
}

void warehouseNextPlan(tNextPlanReason eReason) {
	if(eReason != NEXT_PLAN_REASON_FAILED) {
		if(!s_sCurrentPlan.isPenaltyCountdownStarted) {
			gameAdvanceAccolade();
		}

		if(eReason == NEXT_PLAN_REASON_FULFILLED_ACCOUNTING) {
			heatTryReduce(2);
		}
	}

	// Increase the plan target sum by relevant ratio
	fix16_t fRatio = (
		g_is2pPlaying ?
		g_fPlanIncreaseRatioMultiplayer :
		g_fPlanIncreaseRatioSingleplayer
	);
	s_sCurrentPlan.ulTargetSum += fix16_to_int(fix16_mul(
		fix16_from_int(s_sCurrentPlan.ulTargetSum),
		fRatio
	));

	planReset(warehouseGetCurrentPlan(), 0, 1);
}

void warehouseRerollPlan(void) {
	planReset(warehouseGetCurrentPlan(), 1, 0);
}

UWORD warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UWORD uwCount) {
	s_pStock[ubMineralType] = uwCount;
}
