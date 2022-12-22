/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _DEFS_H_
#define _DEFS_H_

#include <ace/types.h>
#include "string_array.h"
#include <json/utf8_remap.h>

typedef enum _tMsg {
	// Plan
	MSG_PLAN_DONE_AFK,
	MSG_PLAN_NOT_DONE,
	MSG_PLAN_REMAINING,
	MSG_PLAN_EXTENDING,
	MSG_PLAN_FINAL_PLAN,
	// Hi score
	MSG_HI_SCORE_NEW,
	MSG_HI_SCORE_PRESS,
	MSG_HI_SCORE_WIN_SCORE,
	MSG_HI_SCORE_WIN_P1,
	MSG_HI_SCORE_WIN_P2,
	MSG_HI_SCORE_DRAW,
	// Tutorial
	MSG_TUTORIAL_GO_DIG,
	MSG_TUTORIAL_ON_DUG,
	MSG_TUTORIAL_NEAR_SHOP,
	MSG_TUTORIAL_IN_SHOP,
	MSG_TUTORIAL_ON_MOVE_TO_PLAN,
	// Comm
	MSG_COMM_TIME_REMAINING,
	MSG_COMM_ACCOLADES,
	MSG_COMM_REBUKES,
	MSG_COMM_MK,
	MSG_COMM_UPGRADE_TO_MK,
	MSG_COMM_STOCK,
	MSG_COMM_BUY,
	MSG_COMM_EXIT,
	MSG_COMM_CONFIRM,
	MSG_COMM_ALREADY_MAX,
	MSG_COMM_ALREADY_FULL,
	// Challenge
	MSG_CHALLENGE_CHECKPOINT,
	MSG_CHALLENGE_TELEPORT,
	// Misc
	MSG_MISC_DRILL_DEPLETED,
	MSG_MISC_CARGO_FULL,
	MSG_MISC_RESTOCK,
	MSG_MISC_FOUND_BONE,
	// HUD
	MSG_HUD_P1,
	MSG_HUD_P2,
	MSG_HUD_DRILL,
	MSG_HUD_CARGO,
	MSG_HUD_HULL,
	MSG_HUD_CASH,
	MSG_HUD_DEPTH,
	MSG_HUD_PAUSED,
	MSG_HUD_RESUME,
	MSG_HUD_QUIT,
	// Loading
	MSG_LOADING_GEN_TERRAIN,
	MSG_LOADING_GEN_BASES,
	MSG_LOADING_FINISHING,
	// Office
	MSG_PAGE_MAIN,
	MSG_PAGE_LIST_MIETEK,
	MSG_PAGE_LIST_KRYSTYNA,
	MSG_PAGE_LIST_PUTIN,
	MSG_PAGE_LIST_URZEDAS,
	MSG_PAGE_DOSSIER_KRYSTYNA,
	MSG_PAGE_DOSSIER_URZEDAS,
	MSG_PAGE_BRIBE,
	MSG_PAGE_FAVOR,
	MSG_PAGE_ACCOUNTING,
	// Count
	MSG_COUNT
} tMsg;

extern LONG g_lInitialCash;
extern UBYTE g_ubUpgradeLevels, g_ubPlansPerAccolade;
extern UBYTE g_ubAccoladesInMainStory;
extern UBYTE g_ubRebukesInMainStory;
extern UBYTE g_ubDrillingCost;
extern UBYTE g_ubLiterPrice, g_ubFuelInLiter, g_ubHullPrice;

extern LONG g_pUpgradeCosts[10];
extern UWORD g_pDinoDepths[9];

extern char **g_pMsgs;

void defsInit(void);

void langCreate(const char *szLangPrefix);

void langDestroy(void);

extern const tCodeRemap g_pRemap[19];

#endif // _DEFS_H_
