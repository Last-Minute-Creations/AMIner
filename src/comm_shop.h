/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_SHOP_H_
#define _COMM_SHOP_H_

#include <ace/types.h>
#include "string_array.h"

#define WORKSHOP_ITEM_COUNT 4
#define WAREHOUSE_COL_COUNT 4

extern tStringArray g_sShopNames, g_sShopMsgs, g_sWarehouseColNames;

void commShopGsCreate(void);

void commShopGsLoop(void);

void commShopGsDestroy(void);

UBYTE commShopIsActive(void);

void officeResetPpl(void);

#endif // _COMM_SHOP_H_
