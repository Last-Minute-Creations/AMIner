/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GAME_H_
#define _GAME_H_

#include <ace/managers/audio.h>
#include "bob_new.h"
#include "string_array.h"

#define GAME_BPP 5

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

void gameGsCreate(void);

void gameGsLoop(void);

void gameGsLoopChallengeEnd(void);

void gameGsDestroy(void);

void gameStart(void);

void gameChallengeEnd(void);

void gameGsLoopEnterScore(void);

void gameTryPushBob(tBobNew *pBob);

void gameAddAccolade(void);

void gameAddRebuke(void);

UBYTE gameGetAccolades(void);

UBYTE gameGetRebukes(void);

typedef enum _tPlanMessages {
	MSG_PLAN_DONE_AFK,
	MSG_PLAN_NOT_DONE,
	MSG_PLAN_REMAINING,
	MSG_PLAN_EXTENDING,
	MSG_PLAN_COUNT
} tPlanMessages;

extern tSample *g_pSampleDrill, *g_pSampleOre, *g_pSamplePenalty;
extern tStringArray g_sPlanMessages;

// Game config
extern UBYTE g_is2pPlaying;
extern UBYTE g_is1pKbd, g_is2pKbd;
extern UBYTE g_isChallenge, g_isChallengeEnd;
extern UBYTE g_isAtari;
extern tBobNew g_pBombMarkers[3];

#endif // _GAME_H_
