/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base_unlocks.h"
#include "game.h"
#include "base.h"
#include "inventory.h"

#define BASE_ANTENNA_WIDTH 32
#define BASE_ANTENNA_HEIGHT 27
#define BASE_WORKSHOP_WIDTH 32
#define BASE_WORKSHOP_HEIGHT 25
#define BASE_WORKSHOP_OFFSET BASE_ANTENNA_HEIGHT
#define BASE_TELEPORTER_IDLE_FRAME_HEIGHT 12
#define BASE_TELEPORTER_IDLE_DELTA_Y (BASE_TELEPORTER_HEIGHT - BASE_TELEPORTER_IDLE_FRAME_HEIGHT)

static tBob s_sTeleporterIdleBob, s_sAntennaBob, s_sWarehouseBob;
static tBitMap *s_pTeleporterIdleFrame;
static tBitMap *s_pTeleporterIdleMask;
static tBitMap *s_pBaseUnlocksFrames;
static tBitMap *s_pBaseUnlocksMasks;

void baseUnlocksCreate(void) {
	s_pTeleporterIdleFrame = bitmapCreateFromPath("data/base_teleporter_idle.bm", 0);
	s_pTeleporterIdleMask = bitmapCreateFromPath("data/base_teleporter_idle_mask.bm", 0);
	s_pBaseUnlocksFrames = bitmapCreateFromPath("data/base_unlocks.bm", 0);
	s_pBaseUnlocksMasks = bitmapCreateFromPath("data/base_unlocks_mask.bm", 0);

	bobInit(
		&s_sTeleporterIdleBob, BASE_TELEPORTER_WIDTH, BASE_TELEPORTER_IDLE_FRAME_HEIGHT, 0,
		s_pTeleporterIdleFrame->Planes[0], s_pTeleporterIdleMask->Planes[0], 0, 0
	);
	bobInit(
		&s_sAntennaBob, BASE_ANTENNA_WIDTH, BASE_ANTENNA_HEIGHT, 0,
		s_pBaseUnlocksFrames->Planes[0], s_pBaseUnlocksMasks->Planes[0], 0, 0
	);
	bobInit(
		&s_sWarehouseBob, BASE_WORKSHOP_WIDTH, BASE_WORKSHOP_HEIGHT, 0,
		bobCalcFrameAddress(s_pBaseUnlocksFrames, BASE_WORKSHOP_OFFSET),
		bobCalcFrameAddress(s_pBaseUnlocksMasks, BASE_WORKSHOP_OFFSET), 0, 0
	);
}

void baseUnlocksDrawBack(void) {
	tBaseId eBaseId = baseGetCurrentId();
	const tBase *pBase = baseGetCurrent();
	if(inventoryGetBasePartLevel(INVENTORY_PART_BASE_PLATFORM, eBaseId)) {
		s_sTeleporterIdleBob.sPos.ulYX = pBase->sPosTeleport.ulYX;
		s_sTeleporterIdleBob.sPos.uwY += BASE_TELEPORTER_IDLE_DELTA_Y;
		gameTryPushBob(&s_sTeleporterIdleBob);
	}
	if(eBaseId != BASE_ID_GROUND) {
		tCommUnlockState eCommUnlockState = inventoryGetCommUnlockState(eBaseId);
		if(eCommUnlockState >= COMM_UNLOCK_STATE_OFFICE_WORKSHOP) {
			s_sAntennaBob.sPos.ulYX = pBase->sPosAntenna.ulYX;
			gameTryPushBob(&s_sAntennaBob);
		}
		if(eCommUnlockState >= COMM_UNLOCK_STATE_WAREHOUSE) {
			s_sWarehouseBob.sPos.ulYX = pBase->sPosWorkshop.ulYX;
			gameTryPushBob(&s_sWarehouseBob);
		}
	}
}

void baseUnlocksDestroy(void) {
	bitmapDestroy(s_pTeleporterIdleFrame);
	bitmapDestroy(s_pTeleporterIdleMask);
	bitmapDestroy(s_pBaseUnlocksFrames);
	bitmapDestroy(s_pBaseUnlocksMasks);
}
