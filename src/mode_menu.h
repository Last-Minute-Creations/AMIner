/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _MODE_MENU_H_
#define _MODE_MENU_H_

#include <ace/managers/bob.h>
#include "direction.h"

//------------------------------------------------------------------------ TYPES

#define MODE_BOBS_PER_PLAYER 3

typedef enum tModeOption {
	MODE_OPTION_EXCLAMATION,
	// MODE_OPTION_QUESTION,
	MODE_OPTION_DRILL,
	MODE_OPTION_TNT,
	MODE_OPTION_TELEPORT,
	// MODE_OPTION_TRAVEL_UP,
	// MODE_OPTION_TRAVEL_DOWN,
	// MODE_OPTION_TRAVEL_RETURN,
	MODE_OPTION_COUNT
} tModeOption;

typedef struct tModeMenu {
	tBob sBob;
	tModeOption pModeOptions[MODE_BOBS_PER_PLAYER];
	UBYTE ubCount;
	UBYTE ubCurrent;
	UBYTE isActive;
	UBYTE ubPlayerIndex;
} tModeMenu;

void modeMenuManagerCreate(void);

void modeMenuManagerDestroy(void);

void modeMenuReset(tModeMenu *pModeMenu, UBYTE ubPlayerIndex);

void modeMenuClearOptions(tModeMenu *pModeMenu);

void modeMenuAddOption(tModeMenu *pModeMenu, tModeOption eOption);

void modeMenuProcess(tModeMenu *pModeMenu, tDirection eDirection);

void modeMenuEnterSelection(tModeMenu *pModeMenu);

tModeOption modeMenuExitSelection(tModeMenu *pModeMenu);

tModeOption modeMenuGetSelected(tModeMenu *pModeMenu);

void modeMenuTryDisplay(tModeMenu *pModeMenu);

#endif // _MODE_MENU_H_
