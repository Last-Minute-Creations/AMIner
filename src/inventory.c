/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inventory.h"
#include "save.h"

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

const tPartDef *inventoryGetPartDef(tPartKind eName) {
	return &s_sInventory.pParts[eName];
}

void inventorySetPartLevel(tPartKind eName, UBYTE ubLevel) {
	tPartDef *pPart = &s_sInventory.pParts[eName];
	pPart->ubLevel = ubLevel;
	pPart->uwMax = pPart->uwMaxBase + ubLevel * pPart->uwMaxAddPerLevel;
}
