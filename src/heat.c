/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "heat.h"
#include "core.h"
#include "save.h"

#define HEAT_VALUE_MIN 0
#define HEAT_VALUE_MAX 90
#define HEAT_VALUE_START HEAT_VALUE_MIN

static UBYTE s_ubHeat;

UBYTE heatGetPercent(void) {
	return s_ubHeat;
}

void heatTryIncrease(UBYTE ubPercent) {
	s_ubHeat = MIN(s_ubHeat + ubPercent, HEAT_VALUE_MAX);
}

void heatTryReduce(UBYTE ubPercent) {
	BYTE bHeatNew = s_ubHeat - ubPercent;
	if(bHeatNew < HEAT_VALUE_MIN) {
		bHeatNew = HEAT_VALUE_MIN;
	}
	s_ubHeat = bHeatNew;
}

void heatReset(void) {
	s_ubHeat = HEAT_VALUE_START;
}

void heatSave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_HEAT);
	fileWriteBytes(pFile, &s_ubHeat, 1);
	saveWriteTag(pFile, SAVE_TAG_HEAT_END);
}

UBYTE heatLoad(tFile *pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_HEAT)) {
		return 0;
	}

	fileReadBytes(pFile, &s_ubHeat, 1);
	return saveReadTag(pFile, SAVE_TAG_HEAT_END);
}


UBYTE heatTryPassCheck()
{
	UBYTE isPassed = randUwMinMax(&g_sRand, 1, 100) > heatGetPercent();
	return isPassed;
}
