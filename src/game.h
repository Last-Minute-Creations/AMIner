/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GAME_H_
#define _GAME_H_

#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/font.h>

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

void tileRefreshGrass(UWORD uwX);

void gameGsCreate(void);

void gameGsLoop(void);

void gameGsLoopChallengeEnd(void);

void gameGsDestroy(void);

void gameStart(void);

void gameChallengeEnd(void);

extern tTileBufferManager *g_pMainBuffer;
extern tFont *g_pFont;

// Game config
extern UBYTE g_is2pPlaying;
extern UBYTE g_is1pKbd, g_is2pKbd;
extern UBYTE g_isChallenge;

#endif // _GAME_H_
