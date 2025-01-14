/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base_teleporter.h"
#include "game.h"
#include "base.h"

tBob s_sTeleporterBob;
tBitMap *s_pTeleporterFrames;
tBitMap *s_pTeleporterMasks;

void baseTeleporterCreate(void) {
	s_pTeleporterFrames = bitmapCreateFromFile("data/base_teleporter.bm", 0);
	s_pTeleporterMasks = bitmapCreateFromFile("data/base_teleporter_mask.bm", 0);

	bobInit(
		&s_sTeleporterBob, BASE_TELEPORTER_WIDTH, BASE_TELEPORTER_HEIGHT, 1,
		bobCalcFrameAddress(s_pTeleporterFrames, 0),
		bobCalcFrameAddress(s_pTeleporterMasks, 0),
		0, 0
	);
}

void baseTeleporterProcess(void) {
	const tBase *pBase = baseGetCurrent();
	s_sTeleporterBob.sPos.ulYX = pBase->sPosTeleport.ulYX;
	gameTryPushBob(&s_sTeleporterBob);
}

void baseTeleporterDestroy(void) {
	bitmapDestroy(s_pTeleporterFrames);
	bitmapDestroy(s_pTeleporterMasks);
}
