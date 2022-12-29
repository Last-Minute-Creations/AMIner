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

void warehouseAdvancePlan(void) {
	if(!warehouseGetCurrentPlan()->isFailed) {
		if(!s_sCurrentPlan.isPenaltyCountdownStarted) {
			gameAdvanceAccolade();
		}

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
		pageAccountingReduceChanceFail();
	}
	else {
		planReset(warehouseGetCurrentPlan());
	}
}

void warehouseRerollPlan(void) {
	planReset(warehouseGetCurrentPlan());
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

UWORD warehouseGetStock(UBYTE ubMineralType) {
	return s_pStock[ubMineralType];
}

void warehouseSetStock(UBYTE ubMineralType, UWORD uwCount) {
	s_pStock[ubMineralType] = uwCount;
}
