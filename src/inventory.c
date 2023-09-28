/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inventory.h"
#include "save.h"

tInventory s_sInventory;

void inventoryInit(const UWORD *pPartsBase, const UWORD *pPartsAddPerLevel) {
	for(tPartName ePart = 0; ePart < INVENTORY_PART_COUNT; ++ePart) {
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
	saveWriteHeader(pFile, "IVTR");
	fileWrite(pFile, &s_sInventory, sizeof(s_sInventory));
}

UBYTE inventoryLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "IVTR")) {
		return 0;
	}

	fileRead(pFile, &s_sInventory, sizeof(s_sInventory));
	return 1;
}

const tPart *inventoryGetPartDef(tPartName eName) {
	return &s_sInventory.pParts[eName];
}

void inventorySetPartLevel(tPartName eName, UBYTE ubLevel) {
	tPart *pPart = &s_sInventory.pParts[eName];
	pPart->ubLevel = ubLevel;
	pPart->uwMax = pPart->uwMaxBase + ubLevel * pPart->uwMaxAddPerLevel;
}
