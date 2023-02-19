/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include <ace/types.h>
#include <ace/utils/file.h>

typedef enum _tPartName {
	INVENTORY_PART_DRILL,
	INVENTORY_PART_CARGO,
	INVENTORY_PART_HULL,
	INVENTORY_PART_COUNT
} tPartName;

typedef enum _tItemName {
	INVENTORY_ITEM_TNT,
	INVENTORY_ITEM_NUKE,
	INVENTORY_ITEM_TELEPORT,
	INVENTORY_ITEM_COUNT
} tItemName;

typedef struct _tPart {
	UBYTE ubLevel;
	UWORD uwMaxBase;
	UWORD uwMaxAddPerLevel;
	UWORD uwMax;
} tPart;

typedef struct _tItem {
	UBYTE ubCount;
	UBYTE ubMax;
	UWORD uwPrice;
} tItem;

typedef struct _tInventory {
	tPart pParts[INVENTORY_PART_COUNT];
	tItem pItems[INVENTORY_ITEM_COUNT];
} tInventory;

void inventoryReset(void);

void inventorySave(tFile *pFile);

UBYTE inventoryLoad(tFile *pFile);

const tItem *inventoryGetItemDef(tItemName eName);

const tPart *inventoryGetPartDef(tPartName eName);

void inventorySetPartLevel(tPartName eName, UBYTE ubLevel);

void inventorySetItemCount(tItemName eName, UBYTE ubCount);

void inventoryInit(
	const UWORD *pPartsBase, const UWORD *pPartsAddPerLevel,
	const UWORD *pItemsPrice, const UBYTE *pItemsMax
);

#endif // _INVENTORY_H_
