/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _MENU_H_
#define _MENU_H_

#include <ace/types.h>
#include <string_array.h>

void menuGsCreate(void);

void menuGsLoop(void);

void menuGsDestroy(void);

void menuPreload(void);
void menuUnload(void);

void menuGsEnter(UBYTE isScoreShow);

extern tStringArray g_sMenuCaptions, g_sMenuEnumMode,
	g_sMenuEnumP1, g_sMenuEnumP2, g_sMenuEnumOnOff, g_sMenuEnumPlayerCount;

#endif // _MENU_H_
