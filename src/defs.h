/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _DEFS_H_
#define _DEFS_H_

#include <ace/types.h>
#include "string_array.h"
#include "mineral.h"
#include <json/utf8_remap.h>

#define DEFS_QUEST_DINO_BONE_COUNT 9
#define DEFS_MINE_DIGGABLE_WIDTH 10
#define DEFS_QUEST_GATE_PART_COUNT 16
#define DEFS_QUEST_CRATE_COUNT 10

typedef enum _tMsg {
	// Plan
	MSG_HUD_NEW_PLAN,
	MSG_HUD_PLAN_REMAINING,
	MSG_HUD_PLAN_EXTENDING,
	MSG_HUD_WAITING_KOMISARZ,
	MSG_HUD_GUEST,
	MSG_HUD_WAITING_URZEDAS,
	MSG_HUD_SCI_WELCOME,
	// Hi score
	MSG_HI_SCORE_NEW,
	MSG_HI_SCORE_PRESS,
	MSG_HI_SCORE_WIN_SCORE,
	MSG_HI_SCORE_WIN_P1,
	MSG_HI_SCORE_WIN_P2,
	MSG_HI_SCORE_DRAW,
	// Tutorial
	MSG_TUTORIAL_GO_MEET_MIETEK,
	MSG_TUTORIAL_ON_DUG,
	MSG_TUTORIAL_IN_SHOP,
	MSG_TUTORIAL_DESCRIPTION_TAB_OFFICE,
	MSG_TUTORIAL_DESCRIPTION_TAB_WORKSHOP,
	MSG_TUTORIAL_DESCRIPTION_TAB_WAREHOUSE,
	// Comm
	MSG_COMM_TIME_REMAINING,
	MSG_COMM_ACCOLADES,
	MSG_COMM_REBUKES,
	MSG_COMM_HEAT,
	MSG_COMM_DAYS,
	MSG_COMM_PLANS,
	MSG_COMM_MK,
	MSG_COMM_UPGRADE_TO_MK,
	MSG_COMM_STOCK,
	MSG_COMM_BUY,
	MSG_COMM_EXIT,
	MSG_COMM_CONFIRM,
	MSG_COMM_MARKET,
	MSG_COMM_ALREADY_MAX,
	MSG_COMM_ALREADY_FULL,
	MSG_COMM_MARKET_TRADE_COL,
	MSG_COMM_MARKET_FOR_COL,
	// Challenge
	MSG_CHALLENGE_CHECKPOINT,
	MSG_CHALLENGE_TELEPORT,
	// Misc
	MSG_MISC_DRILL_DEPLETED,
	MSG_MISC_CARGO_FULL,
	MSG_MISC_RESTOCK,
	MSG_MISC_FOUND_BONE,
	MSG_MISC_FOUND_GATE,
	MSG_MISC_FOUND_CRATE,
	MSG_MISC_FOUND_CAPSULE,
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
	MSG_HUD_SAVE_QUIT,
	MSG_HUD_DINO_COMPLETE,
	MSG_HUD_DINO_FOUND_BONE,
	MSG_HUD_RADIO_START_0,
	MSG_HUD_RADIO_START_1,
	MSG_HUD_RADIO_START_2,
	MSG_HUD_RADIO_HALF_0,
	MSG_HUD_RADIO_HALF_1,
	MSG_HUD_RADIO_HALF_2,
	MSG_HUD_RADIO_FULL_0,
	MSG_HUD_RADIO_FULL_1,
	MSG_HUD_RADIO_FULL_2,
	// Loading
	MSG_LOADING_GEN_TERRAIN,
	MSG_LOADING_GEN_BASES,
	MSG_LOADING_FINISHING,
	// Office - must be same order as COMM_SHOP_PAGE_OFFICE_* in gs_shop.h
	MSG_PAGE_MAIN,
	MSG_PAGE_LIST_MIETEK,
	MSG_PAGE_LIST_KRYSTYNA,
	MSG_PAGE_LIST_KOMISARZ,
	MSG_PAGE_LIST_URZEDAS,
	MSG_PAGE_LIST_ARCH,
	MSG_PAGE_LIST_PRISONER,
	MSG_PAGE_LIST_AGENT,
	MSG_PAGE_LIST_SCI,

	MSG_PAGE_MIETEK_WELCOME,
	MSG_PAGE_MIETEK_FIRST_PLAN,
	MSG_PAGE_MIETEK_PLAN_COMPLETE,

	MSG_PAGE_KRYSTYNA_DOSSIER,
	MSG_PAGE_KRYSTYNA_ACCOUNTING,

	MSG_PAGE_URZEDAS_DOSSIER,
	MSG_PAGE_URZEDAS_WELCOME,
	MSG_PAGE_URZEDAS_FIRST_PLAN,
	MSG_PAGE_URZEDAS_PLAN_COMPLETE,
	MSG_PAGE_URZEDAS_PLAN_DELAYED,
	MSG_PAGE_URZEDAS_BRIBE,
	MSG_PAGE_URZEDAS_FAVOR,

	MSG_PAGE_KOMISARZ_DOSSIER,
	MSG_PAGE_KOMISARZ_WELCOME,
	MSG_PAGE_KOMISARZ_DINO_INTRO,
	MSG_PAGE_KOMISARZ_REBUKE_1,
	MSG_PAGE_KOMISARZ_REBUKE_2,
	MSG_PAGE_KOMISARZ_REBUKE_3,
	MSG_PAGE_KOMISARZ_ARCH_ACCOLADE,
	MSG_PAGE_KOMISARZ_QUESTIONING,
	MSG_PAGE_KOMISARZ_REPORTING_LIST,
	MSG_PAGE_KOMISARZ_REPORTING_GATE,
	MSG_PAGE_KOMISARZ_REPORTING_TELEPORT_PARTS,
	MSG_PAGE_KOMISARZ_REPORTING_AGENT,

	MSG_PAGE_ARCH_DOSSIER,
	MSG_PAGE_ARCH_WELCOME,
	MSG_PAGE_ARCH_PLAN_FAIL,
	MSG_PAGE_ARCH_ACCOLADE,
	MSG_PAGE_ARCH_GATE_OPENED,
	MSG_PAGE_ARCH_GATE_DESTROYED,

	MSG_PAGE_PRISONER_DOSSIER,
	MSG_PAGE_PRISONER_WELCOME,
	MSG_PAGE_PRISONER_GATE_DESTROYED,
	MSG_PAGE_PRISONER_RADIO_1,
	MSG_PAGE_PRISONER_RADIO_2,
	MSG_PAGE_PRISONER_RADIO_3,

	MSG_PAGE_AGENT_WELCOME,
	MSG_PAGE_AGENT_SCIENTISTS,
	MSG_PAGE_AGENT_SELL_CRATE,
	MSG_PAGE_AGENT_ESCAPE,

	MSG_PAGE_SCIENTIST_WELCOME,
	MSG_PAGE_SCIENTIST_FIRST_CRATE,
	// Count
	MSG_COUNT
} tMsg;

extern LONG g_lInitialCash;
extern UBYTE g_ubUpgradeLevels;
extern UBYTE g_ubPlansPerAccolade;
extern UBYTE g_ubAccoladesInMainStory;
extern UBYTE g_ubRebukesInMainStory;
extern UBYTE g_ubDrillingCost;
extern UBYTE g_ubLiterPrice, g_ubFuelInLiter, g_ubHullPrice;
extern fix16_t g_fPlanIncreaseRatioSingleplayer;
extern fix16_t g_fPlanIncreaseRatioMultiplayer;
extern UBYTE g_ubMinePercentForPlans;
extern UBYTE g_ubTrailingMineralCountPercent;
extern ULONG g_ulExtraPlanMoney;

extern LONG g_pUpgradeCosts[10];
extern UWORD g_pDinoDepths[DEFS_QUEST_DINO_BONE_COUNT];
extern UWORD g_pGateDepths[DEFS_QUEST_GATE_PART_COUNT];
extern UWORD g_pCrateDepths[DEFS_QUEST_CRATE_COUNT];
extern UWORD g_uwCapsuleDepth;
extern UWORD g_uwPrisonerDepth;
extern UWORD g_pMineralPlans[MINERAL_TYPE_COUNT];

extern char **g_pMsgs;

void defsInit(void);

void defsCreateLocale(const char *szLangPrefix);

void defsDestroyLocale(void);

extern const tCodeRemap g_pRemap[19];

#endif // _DEFS_H_
