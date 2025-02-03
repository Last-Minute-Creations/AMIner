/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base_teleporter.h"
#include "game.h"
#include "base.h"
#include "inventory.h"

#define BASE_TELEPORTER_IDLE_FRAME_HEIGHT 12
#define BASE_TELEPORTER_IDLE_DELTA_Y (BASE_TELEPORTER_HEIGHT - BASE_TELEPORTER_IDLE_FRAME_HEIGHT)

static tBob s_sTeleporterIdleBob;
static tBitMap *s_pTeleporterIdleFrame;
static tBitMap *s_pTeleporterIdleMask;

void baseTeleporterCreate(void) {
	s_pTeleporterIdleFrame = bitmapCreateFromPath("data/base_teleporter_idle.bm", 0);
	s_pTeleporterIdleMask = bitmapCreateFromPath("data/base_teleporter_idle_mask.bm", 0);

	bobInit(
		&s_sTeleporterIdleBob, BASE_TELEPORTER_WIDTH, BASE_TELEPORTER_IDLE_FRAME_HEIGHT, 1,
		s_pTeleporterIdleFrame->Planes[0], s_pTeleporterIdleMask->Planes[0], 0, 0
	);
}

void baseTeleporterProcess(void) {
	if(inventoryGetBasePartLevel(INVENTORY_PART_BASE_PLATFORM, baseGetCurrentId())) {
		const tBase *pBase = baseGetCurrent();
		s_sTeleporterIdleBob.sPos.ulYX = pBase->sPosTeleport.ulYX;
		s_sTeleporterIdleBob.sPos.uwY += BASE_TELEPORTER_IDLE_DELTA_Y;
		gameTryPushBob(&s_sTeleporterIdleBob);
	}
}

void baseTeleporterDestroy(void) {
	bitmapDestroy(s_pTeleporterIdleFrame);
	bitmapDestroy(s_pTeleporterIdleMask);
}
