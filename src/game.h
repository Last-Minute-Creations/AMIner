/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GAME_H_
#define _GAME_H_

#include "aminer.h"
#include <ace/managers/ptplayer.h>
#include "bob_new.h"
#include "string_array.h"

#define GAME_BPP 5
#define GAME_MOD_COUNT 10
#define GAME_TIME_PER_DAY 140

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

void gameGsLoopChallengeEnd(void);

void gameStart(UBYTE isChallenge);

void gameTriggerSave(void);

void gameSave(tFile *pFile);

UBYTE gameLoad(tFile *pFile);

void gameGsLoopEnterScore(void);

void gameTryPushBob(tBobNew *pBob);

void gameAdvanceAccolade(void);

void gameAddRebuke(void);

UBYTE gameGetAccolades(void);

UBYTE gameGetRebukes(void);

void gameElapseTime(UWORD uwTime);

void gameElapseDay(void);

ULONG gameGetTime(void);

UBYTE gameIsElapsedDays(ULONG ulStart, UBYTE ubDays);

extern tPtplayerSfx *g_pSfxDrill, *g_pSfxOre, *g_pSfxPenalty;
extern tPtplayerMod *g_pGameMods[GAME_MOD_COUNT];

// Game config
extern UBYTE g_is2pPlaying;
extern UBYTE g_isChallenge;
extern UBYTE g_isAtari;
extern tBobNew g_pBombMarkers[3];
extern tState g_sStateGame;

#endif // _GAME_H_
