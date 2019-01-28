/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <mineral.h>

const tMineralDef g_pMinerals[MINERAL_TYPE_COUNT] = {
	[MINERAL_TYPE_SILVER] = {.szName = "Silver", .ubReward = 5, .ubTitleColor = 15},
	[MINERAL_TYPE_GOLD] = {.szName = "Gold", .ubReward = 10, .ubTitleColor = 14},
	[MINERAL_TYPE_EMERALD] = {.szName = "Emerald", .ubReward = 15, .ubTitleColor = 12},
	[MINERAL_TYPE_RUBY] = {.szName = "Ruby", .ubReward = 20, .ubTitleColor = 9},
	[MINERAL_TYPE_MOONSTONE] = {.szName = "Moonstone", .ubReward = 25, .ubTitleColor = 10},
	[MINERAL_TYPE_COAL] = {.szName = "Coal", .ubReward = 5, .ubTitleColor = 10},
};

