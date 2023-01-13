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

// Not spent on plan, not sold
static UWORD s_pStock[MINERAL_TYPE_COUNT] = {0};

static tPlan s_sCurrentPlan;
static const tPlan s_sFirstPlan = {
	.pMinerals = {{0}},
	.ulMineralsUnlocked = 1 << MINERAL_TYPE_SILVER,
	.ulTargetSum = 15,
};

void warehouseReset(void) {
	memset(s_pStock, 0, sizeof(s_pStock));
	s_sCurrentPlan = s_sFirstPlan;
	planReset(warehouseGetCurrentPlan());
}

tPlan *warehouseGetCurrentPlan(void) {
	return &s_sCurrentPlan;
}

void warehouseReserveMineralsForPlan(UBYTE ubMineralType, UBYTE ubCount) {
	s_sCurrentPlan.pMinerals[ubMineralType].uwCurrentCount = ubCount;
}

void warehouseNextPlan(UBYTE isFailed) {
	if(!isFailed) {
		if(!s_sCurrentPlan.isPenaltyCountdownStarted) {
			gameAdvanceAccolade();
		}
		pageAccountingReduceChanceFail();
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

	planReset(warehouseGetCurrentPlan());
}

void warehouseRerollPlan(void) {
	planReset(warehouseGetCurrentPlan());
}

UWORD warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UWORD uwCount) {
	s_pStock[ubMineralType] = uwCount;
}
