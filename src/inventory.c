/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inventory.h"
#include "save.h"
#include "game.h"

tInventory s_sInventory;

void inventoryInit(const UWORD *pPartsBase, const UWORD *pPartsAddPerLevel) {
	for(tPartKind ePart = 0; ePart < INVENTORY_PART_COUNT; ++ePart) {
		s_sInventory.pParts[ePart].uwMaxBase = pPartsBase[ePart];
		s_sInventory.pParts[ePart].uwMaxAddPerLevel = pPartsAddPerLevel[ePart];
	}
}

void inventoryReset(void) {
	for(UBYTE i = 0; i < INVENTORY_PART_COUNT; ++i) {
		inventorySetPartLevel(i, 0);
	}
	for(tBaseId eBase = 0; eBase < BASE_ID_COUNT_UNIQUE; ++eBase) {
		inventorySetCommUnlock(eBase, COMM_UNLOCK_STATE_NONE);
	}

	// default base unlocks
	inventorySetBasePartLevel(INVENTORY_PART_BASE_WORKSHOP, BASE_ID_GROUND, 2);
	inventorySetCommUnlock(BASE_ID_GROUND, COMM_UNLOCK_STATE_WAREHOUSE);
	if(g_eGameMode == GAME_MODE_DEADLINE) {
		inventorySetBasePartLevel(INVENTORY_PART_BASE_PLATFORM, BASE_ID_GROUND, INVENTORY_LEVEL_PLATFORM_ALL);
		inventorySetPartLevel(INVENTORY_PART_TELEPORT, INVENTORY_LEVEL_TELEPORTER_RETURN);
		inventorySetPartLevel(INVENTORY_PART_TNT, g_ubUpgradeLevels);
		inventorySetPartLevel(INVENTORY_PART_DRILL, 2);
	}
}

void inventorySave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_INVENTORY);
	fileWrite(pFile, &s_sInventory, sizeof(s_sInventory));
	saveWriteTag(pFile, SAVE_TAG_INVENTORY_END);
}

UBYTE inventoryLoad(tFile *pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_INVENTORY)) {
		return 0;
	}

	fileRead(pFile, &s_sInventory, sizeof(s_sInventory));
	return saveReadTag(pFile, SAVE_TAG_INVENTORY_END);
}

const tPartDef *inventoryGetPartDef(tPartKind ePart) {
	return &s_sInventory.pParts[ePart];
}

void inventorySetPartLevel(tPartKind ePart, UBYTE ubLevel) {
	tPartDef *pPart = &s_sInventory.pParts[ePart];
	pPart->ubLevel = ubLevel;
	pPart->uwMax = pPart->uwMaxBase + ubLevel * pPart->uwMaxAddPerLevel;
}

UBYTE inventoryGetBasePartLevel(tPartKind ePart, tBaseId eBaseId) {
	const tPartDef *pPart = &s_sInventory.pParts[ePart];
	UBYTE ubLevel = (pPart->ubLevel >> ((UBYTE)eBaseId * 2)) & 0b11;
	return ubLevel;
}

void inventorySetBasePartLevel(tPartKind ePart, tBaseId eBaseId, UBYTE ubLevel) {
	tPartDef *pPart = &s_sInventory.pParts[ePart];
	UBYTE ubLevelMask = ubLevel << (UBYTE)(eBaseId * 2);
	UBYTE ubLevelClearMask = 0b11 << (UBYTE)(eBaseId * 2);
	pPart->ubLevel = (pPart->ubLevel & ~ubLevelClearMask) | ubLevelMask;
}

UBYTE inventoryIsBasePart(tPartKind ePart) {
	return ePart == INVENTORY_PART_BASE_PLATFORM || ePart == INVENTORY_PART_BASE_WORKSHOP;
}

void inventorySetCommUnlock(tBaseId eBase, tCommUnlockState eState) {
	s_sInventory.pCommUnlock[eBase] = eState;
}

tCommUnlockState inventoryGetCommUnlockState(tBaseId eBase) {
	return s_sInventory.pCommUnlock[eBase];
}
