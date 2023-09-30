/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GAME_H_
#define _GAME_H_

#include "aminer.h"
#include <ace/managers/ptplayer.h>
#include <ace/managers/bob.h>
#include "string_array.h"
#include "steer.h"

#define GAME_BPP 5
#define GAME_TIME_PER_DAY 140

typedef struct tGameSummary {
	ULONG ulGameTime;
	LONG lCash;
	UWORD uwMaxDepth;
	UBYTE ubRebukes;
	UBYTE ubAccolades;
	UBYTE ubHeatPercent;
	UBYTE ubPlanIndex;
} tGameSummary;

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

void gameGsLoopChallengeEnd(void);

void gameStart(UBYTE isChallenge, tSteer sSteerP1, tSteer sSteerP2);

void gameTriggerSave(void);

UBYTE gameLoad(tFile *pFile);

UBYTE gameLoadSummary(tFile *pFile, tGameSummary *pSummary);

void gameGsLoopEnterScore(void);

void gameInitBombMarkerBobs(void);

void gameTryPushBob(tBob *pBob);

void gameAdvanceAccolade(void);

void gameAddAccolade(void);

void gameAddRebuke(void);

UBYTE gameGetAccolades(void);

UBYTE gameGetRebukes(void);

void gameElapseTime(UWORD uwTime);

void gameElapseDay(void);

ULONG gameGetTime(void);

UBYTE gameIsElapsedDays(ULONG ulStart, UBYTE ubDays);

tSteer *gameGetSteers(void);

void gameCancelModeForPlayer(UBYTE ubPlayer);

void gameUpdateMaxDepth(UWORD uwTileY);

// Game config
extern UBYTE g_is2pPlaying;
extern UBYTE g_isChallenge;
extern UBYTE g_isAtari;
extern tState g_sStateGame;

#endif // _GAME_H_
