/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inventory.h"

tInventory s_sInventory;

void inventoryInit(
	const UWORD *pPartsBase, const UWORD *pPartsAddPerLevel,
	const UWORD *pItemsPrice, const UBYTE *pItemsMax
) {
	for(tPartName ePart = 0; ePart < INVENTORY_PART_COUNT; ++ePart) {
		s_sInventory.pParts[ePart].uwMaxBase = pPartsBase[ePart];
		s_sInventory.pParts[ePart].uwMaxAddPerLevel = pPartsAddPerLevel[ePart];
	}
	for(tItemName eItem = 0; eItem < INVENTORY_ITEM_COUNT; ++eItem) {
		s_sInventory.pItems[eItem].ubMax = pItemsMax[eItem];
		s_sInventory.pItems[eItem].uwPrice = pItemsPrice[eItem];
	}
}

void inventoryReset(void) {
	for(UBYTE i = 0; i < INVENTORY_ITEM_COUNT; ++i) {
		s_sInventory.pItems[i].ubCount = 0;
	}
	for(UBYTE i = 0; i < INVENTORY_PART_COUNT; ++i) {
		inventorySetPartLevel(i, 0);
	}
}

const tItem *inventoryGetItemDef(tItemName eName) {
	return &s_sInventory.pItems[eName];
}

const tPart *inventoryGetPartDef(tPartName eName) {
	return &s_sInventory.pParts[eName];
}

void inventorySetPartLevel(tPartName eName, UBYTE ubLevel) {
	tPart *pPart = &s_sInventory.pParts[eName];
	pPart->ubLevel = ubLevel;
	pPart->uwMax = pPart->uwMaxBase + ubLevel * pPart->uwMaxAddPerLevel;
}

void inventorySetItemCount(tItemName eName, UBYTE ubCount) {
	tItem *pItem = &s_sInventory.pItems[eName];
	if(ubCount > pItem->ubMax) {
		ubCount = pItem->ubMax;
	}
	pItem->ubCount = ubCount;
}
