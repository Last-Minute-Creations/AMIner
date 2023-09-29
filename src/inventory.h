/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include <ace/types.h>
#include <ace/utils/file.h>

#define UPGRADE_LEVEL_COUNT 4

typedef enum tPartKind {
	INVENTORY_PART_DRILL,
	INVENTORY_PART_CARGO,
	INVENTORY_PART_HULL,
	INVENTORY_PART_TNT,
	INVENTORY_PART_TELEPORT,
	INVENTORY_PART_COUNT
} tPartKind;

typedef struct tPartDef {
	UBYTE ubLevel;
	UWORD uwMaxBase;
	UWORD uwMaxAddPerLevel;
	UWORD uwMax;
} tPartDef;

typedef struct _tInventory {
	tPartDef pParts[INVENTORY_PART_COUNT];
} tInventory;

void inventoryReset(void);

void inventorySave(tFile *pFile);

UBYTE inventoryLoad(tFile *pFile);

const tPartDef *inventoryGetPartDef(tPartKind eName);

void inventorySetPartLevel(tPartKind eName, UBYTE ubLevel);

void inventoryInit(const UWORD *pPartsBase, const UWORD *pPartsAddPerLevel);

#endif // _INVENTORY_H_
