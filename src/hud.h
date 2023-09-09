/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _HUD_H_
#define _HUD_H_

#include <ace/utils/extview.h>
#include <ace/utils/font.h>
#include "string_array.h"
#include "face_id.h"

#define HUD_HEIGHT 31

typedef enum _tHudPlayer {
	PLAYER_1 = 0,
	PLAYER_2
} tHudPlayer;

typedef enum _tMode {
	MODE_DRILL,
	MODE_TNT,
	MODE_NUKE,
	MODE_TELEPORT,
	MODE_COUNT
} tMode;

void hudCreate(tVPort *pVpHud, const tFont *pFont);

void hudDestroy(void);

void hudSet2pPlaying(UBYTE isPlaying);

void hudReset(UBYTE isChallenge, UBYTE is2pPlaying);

void hudSave(tFile *pFile);

UBYTE hudLoad(tFile *pFile);

void hudSetDepth(UBYTE ubPlayer, UWORD uwDepth);

void hudSetCash(UBYTE ubPlayer, LONG lCash);

void hudSetCargo(UBYTE ubPlayer, UBYTE ubCargo, UBYTE ubCargoMax);

void hudSetDrill(UBYTE ubPlayer, UWORD uwDrill, UWORD uwDrillMax);

void hudSetHull(UBYTE ubPlayer, UWORD uwHull, UWORD uwHullMax);

void hudUpdate(void);

UBYTE hudIsShowingMessage(void);

void hudShowMessage(tFaceId eFace, const char *szMsg);

void hudShowMain(void);

void hudPause(UBYTE isPaused);

UBYTE hudGetSelection(void);

void hudSelect(UBYTE ubSelection);

//------------------------------------------------------------------------- MODE

void hudSetModeCounter(tMode eMode, WORD wCount);

void hudSetMode(tHudPlayer ePlayer, tMode eMode);

void hudShowMode(void);

void hudHideMode(void);

#endif // _HUD_H_
