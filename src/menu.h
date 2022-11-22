/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _MENU_H_
#define _MENU_H_

#include "aminer.h"
#include <string_array.h>
#include <ace/managers/ptplayer.h>

extern tState g_sStateMenu;

void menuPreload(void);
void menuUnload(void);

void menuGsEnter(UBYTE isScoreShow);

extern char **g_pMenuCaptions, **g_pMenuEnumMode, **g_pMenuEnumP1,
	**g_pMenuEnumP2, **g_pMenuEnumOnOff, **g_pMenuEnumPlayerCount;
extern tPtplayerMod *g_pMenuMod;

#endif // _MENU_H_
