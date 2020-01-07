/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _DEFS_H_
#define _DEFS_H_

#include <ace/types.h>

extern LONG g_lInitialCash;
extern UBYTE g_ubUpgradeLevels;
extern UWORD g_uwPartDrillBase, g_uwPartDrillPerLevel;
extern UWORD g_uwPartCargoBase, g_uwPartCargoPerLevel;
extern UWORD g_uwPartHullBase, g_uwPartHullPerLevel;
extern UBYTE g_ubDrillingCost;
extern UBYTE g_ubLiterPrice, g_ubFuelInLiter, g_ubHullPrice;

extern LONG g_pUpgradeCosts[10];
extern UWORD g_pDinoDepths[9];

void defsInit(void);

void langCreate(const char *szLangPrefix);

void langDestroy(void);

#endif // _DEFS_H_
