/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "defs.h"
#include "json/json.h"
#include <ace/managers/log.h>
#include <fixmath/fix16.h>
#include <comm/comm.h>
#include <comm/page_workshop.h>
#include <comm/page_warehouse.h>
#include "hi_score.h"
#include "menu.h"
#include "game.h"
#include "tutorial.h"
#include "vehicle.h"
#include "tile.h"
#include "inventory.h"
#include "hud.h"

LONG g_lInitialCash;
UBYTE g_ubUpgradeLevels;
UBYTE g_ubDrillingCost;
UBYTE g_ubLiterPrice, g_ubFuelInLiter, g_ubHullPrice;
UBYTE g_ubPlansPerAccolade;
UBYTE g_ubAccoladesInMainStory;
UBYTE g_ubRebukesInMainStory;
fix16_t g_fPlanIncreaseRatioSingleplayer;
fix16_t g_fPlanIncreaseRatioMultiplayer;

LONG g_pUpgradeCosts[10];
UWORD g_pDinoDepths[DEFS_QUEST_DINO_BONE_COUNT];
UWORD g_pGateDepths[DEFS_QUEST_GATE_PART_COUNT];
UWORD g_pMineralPlans[MINERAL_TYPE_COUNT];
UBYTE g_ubMinePercentForPlans;
UBYTE g_ubTrailingMineralCountPercent;
ULONG g_ulExtraPlanMoney;

const char * s_pLangDom[] = {
	// Hi score
	[MSG_HI_SCORE_NEW] = "hiScore.new",
	[MSG_HI_SCORE_PRESS] = "hiScore.press",
	[MSG_HI_SCORE_WIN_SCORE] = "hiScore.score",
	[MSG_HI_SCORE_WIN_P1] = "hiScore.winP1",
	[MSG_HI_SCORE_WIN_P2] = "hiScore.winP2",
	[MSG_HI_SCORE_DRAW] = "hiScore.draw",
	// Tutorial
	[MSG_TUTORIAL_GO_MEET_MIETEK] =  "tutorial.start",
	[MSG_TUTORIAL_ON_DUG] =  "tutorial.onDugOut",
	[MSG_TUTORIAL_IN_SHOP] = "tutorial.inShop",
	[MSG_TUTORIAL_DESCRIPTION_TAB_OFFICE] = "tutorial.descriptionOffice",
	[MSG_TUTORIAL_DESCRIPTION_TAB_WORKSHOP] = "tutorial.descriptionWorkshop",
	[MSG_TUTORIAL_DESCRIPTION_TAB_WAREHOUSE] = "tutorial.descriptionWarehouse",
	// Shop
	[MSG_COMM_TIME_REMAINING] = "shop.timeRemaining",
	[MSG_COMM_ACCOLADES] = "shop.accolades",
	[MSG_COMM_REBUKES] = "shop.rebukes",
	[MSG_COMM_MK] = "shop.mk",
	[MSG_COMM_UPGRADE_TO_MK] = "shop.upgradeToMk",
	[MSG_COMM_STOCK] = "shop.stock",
	[MSG_COMM_BUY] = "shop.buy",
	[MSG_COMM_EXIT] = "shop.exit",
	[MSG_COMM_CONFIRM] = "shop.confirm",
	[MSG_COMM_ALREADY_MAX] = "shop.alreadyMax",
	[MSG_COMM_ALREADY_FULL] = "shop.alreadyFull",
	// Challenge
	[MSG_CHALLENGE_CHECKPOINT] = "challenge.checkpoint",
	[MSG_CHALLENGE_TELEPORT] = "challenge.teleport",
	// Misc
	[MSG_MISC_DRILL_DEPLETED] = "misc.drillDepleted",
	[MSG_MISC_CARGO_FULL] = "misc.cargoFull",
	[MSG_MISC_RESTOCK] = "misc.restock",
	[MSG_MISC_FOUND_BONE] = "misc.foundBone",
	[MSG_MISC_FOUND_GATE] = "misc.foundGate",
	// HUD: UI
	[MSG_HUD_P1] = "hud.p1",
	[MSG_HUD_P2] = "hud.p2",
	[MSG_HUD_DRILL] = "hud.drill",
	[MSG_HUD_CARGO] = "hud.cargo",
	[MSG_HUD_HULL] = "hud.hull",
	[MSG_HUD_CASH] = "hud.cash",
	[MSG_HUD_DEPTH] = "hud.depth",
	[MSG_HUD_PAUSED] = "hud.paused",
	[MSG_HUD_RESUME] = "hud.resume",
	[MSG_HUD_SAVE_QUIT] = "hud.saveQuit",
	// HUD: Misc
	// HUD: Plan
	[MSG_HUD_NEW_PLAN] = "hudMessages.newPlan",
	[MSG_HUD_PLAN_REMAINING] = "hudMessages.remaining",
	[MSG_HUD_PLAN_EXTENDING] = "hudMessages.extending",
	[MSG_HUD_REBUKE] = "hudMessages.rebuke",
	[MSG_HUD_GUEST] = "hudMessages.guest",
	[MSG_HUD_WAITING_URZEDAS] = "hudMessages.waitingUrzedas",
	// Loading
	[MSG_LOADING_GEN_TERRAIN] = "loading.genTerrain",
	[MSG_LOADING_GEN_BASES] = "loading.genBases",
	[MSG_LOADING_FINISHING] = "loading.finishing",
	// Office
	[MSG_PAGE_MAIN] = "officePages.main",
	[MSG_PAGE_LIST_MIETEK] = "officePages.listMietek",
	[MSG_PAGE_LIST_KRYSTYNA] = "officePages.listKrystyna",
	[MSG_PAGE_LIST_KOMISARZ] = "officePages.listPutin",
	[MSG_PAGE_LIST_URZEDAS] = "officePages.listUrzedas",
	[MSG_PAGE_LIST_ARCH] = "officePages.listArch",
	[MSG_PAGE_LIST_PRISONER] = "officePages.listPrisoner",
	[MSG_PAGE_LIST_AGENT] = "officePages.listAgent",
	[MSG_PAGE_MIETEK_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_KRYSTYNA_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_KRYSTYNA_ACCOUNTING] = "officePages.krystynaAccounting",
	[MSG_PAGE_URZEDAS_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_URZEDAS_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_URZEDAS_FIRST_PLAN] = "officePages.urzedasFirstPlan",
	[MSG_PAGE_URZEDAS_PLAN_COMPLETE] = "officePages.planComplete",
	[MSG_PAGE_URZEDAS_PLAN_DELAYED] = "officePages.planDelayed",
	[MSG_PAGE_URZEDAS_DINO_INTRO] = "officePages.urzedasDinoIntro",
	[MSG_PAGE_URZEDAS_BRIBE] = "officePages.urzedasBribe",
	[MSG_PAGE_URZEDAS_FAVOR] = "officePages.urzedasFavor",
	[MSG_PAGE_KOMISARZ_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_KOMISARZ_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_KOMISARZ_REBUKE_1] = "officePages.komisarzRebuke_1",
	[MSG_PAGE_KOMISARZ_REBUKE_2] = "officePages.komisarzRebuke_2",
	[MSG_PAGE_KOMISARZ_REBUKE_3] = "officePages.komisarzRebuke_3",
	[MSG_PAGE_KOMISARZ_QUESTIONING] = "officePages.komisarzQuestioning",
	[MSG_PAGE_ARCH_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_ARCH_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_ARCH_ACCOLADE] = "officePages.archAccolade",
	// Count
	[MSG_COUNT] = 0
};

char **g_pMsgs;

const tCodeRemap g_pRemap[19] = {
	{323, 145}, // "Ń"
	{377, 144}, // "Ź"
	{260, 143}, // "Ą"
	{346, 142}, // "Ś"
	{280, 141}, // "Ę"
	{262, 140}, // "Ć"
	{321, 139}, // "Ł"
	{211, 138}, // "Ó"
	{379, 137}, // "Ż"
	{324, 136}, // "ń"
	{378, 135}, // "ź"
	{261, 134}, // "ą"
	{347, 133}, // "ś"
	{281, 132}, // "ę"
	{263, 131}, // "ć"
	{322, 130}, // "ł"
	{243, 129}, // "ó"
	{380, 128}, // "ż"
	{0, 0} // Terminator
};

void defsInit(void) {
	tJson *pJson = jsonCreate("data/game.json");

	g_lInitialCash = jsonTokToUlong(pJson, jsonGetDom(pJson, "initialCash"));
	g_ubPlansPerAccolade = jsonTokToUlong(pJson, jsonGetDom(pJson, "plansPerAccolade"));
	g_ubAccoladesInMainStory = jsonTokToUlong(pJson, jsonGetDom(pJson, "accoladesInMainStory"));
	g_ubRebukesInMainStory = jsonTokToUlong(pJson, jsonGetDom(pJson, "rebukesInMainStory"));
	g_fPlanIncreaseRatioSingleplayer = jsonTokToFix(pJson, jsonGetDom(pJson, "planIncreaseRatioSingleplayer"));
	g_fPlanIncreaseRatioMultiplayer = jsonTokToFix(pJson, jsonGetDom(pJson, "planIncreaseRatioMultiplayer"));

	// Upgrade costs
	UWORD uwIdxUpgradeCosts = jsonGetDom(pJson, "upgradeCosts");
	g_ubUpgradeLevels = MIN(10, pJson->pTokens[uwIdxUpgradeCosts].size);

	for(UBYTE i = 0; i < pJson->pTokens[uwIdxUpgradeCosts].size; ++i) {
		UWORD uwIdxCost = jsonGetElementInArray(pJson, uwIdxUpgradeCosts, i);
		g_pUpgradeCosts[i] = jsonTokToUlong(pJson, uwIdxCost);
	}

	g_ubDrillingCost = jsonTokToUlong(pJson, jsonGetDom(pJson, "drillingCost"));

	// Restock
	g_ubLiterPrice = jsonTokToUlong(pJson, jsonGetDom(pJson, "restock.literPrice"));
	g_ubFuelInLiter = jsonTokToUlong(pJson, jsonGetDom(pJson, "restock.fuelInLiter"));
	g_ubHullPrice = jsonTokToUlong(pJson, jsonGetDom(pJson, "restock.hullPrice"));

	// Dino parts
	UWORD uwIdxDinoDepths = jsonGetDom(pJson, "dinoDepths");
	UBYTE ubDepthCount = pJson->pTokens[uwIdxDinoDepths].size;
	if(ubDepthCount != 9) {
		logWrite("Dino part count mismatch: got %d, expected 9\n", ubDepthCount);
	}
	for(UBYTE i = 0; i < ubDepthCount; ++i) {
		UWORD uwIdx = jsonGetElementInArray(pJson, uwIdxDinoDepths, i);
		g_pDinoDepths[i] = jsonTokToUlong(pJson, uwIdx);
	}

	// Gate parts
	UWORD uwIdxGateDepths = jsonGetDom(pJson, "dinoDepths");
	ubDepthCount = pJson->pTokens[uwIdxGateDepths].size;
	if(ubDepthCount != 16) {
		logWrite("Dino part count mismatch: got %d, expected 16\n", ubDepthCount);
	}
	for(UBYTE i = 0; i < ubDepthCount; ++i) {
		UWORD uwIdx = jsonGetElementInArray(pJson, uwIdxGateDepths, i);
		g_pGateDepths[i] = jsonTokToUlong(pJson, uwIdx);
	}


	// Minerals
	UWORD uwIdxMineralPlans = jsonGetDom(pJson, "mineralPlans");
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		UWORD uwIdx = jsonGetElementInArray(pJson, uwIdxMineralPlans, i);
		g_pMineralPlans[i] = jsonTokToUlong(pJson, uwIdx);
	}

	g_ubMinePercentForPlans = jsonTokToUlong(pJson, jsonGetDom(pJson, "minePercentForPlans"));
	g_ubTrailingMineralCountPercent = jsonTokToUlong(pJson, jsonGetDom(pJson, "trailingMineralCountPercent"));
	g_ulExtraPlanMoney = jsonTokToUlong(pJson, jsonGetDom(pJson, "extraPlanMoney"));

	UWORD pPartsBase[INVENTORY_PART_COUNT] = {
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.parts.drill.base")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.parts.cargo.base")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.parts.hull.base"))
	};

	UWORD pPartsAddPerLevel[INVENTORY_PART_COUNT] = {
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.parts.drill.addPerLevel")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.parts.cargo.addPerLevel")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.parts.hull.addPerLevel"))
	};

	UWORD pItemsPrice[INVENTORY_ITEM_COUNT] = {
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.items.tnt.price")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.items.nuke.price")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.items.teleport.price"))
	};

	UBYTE pItemsMax[INVENTORY_ITEM_COUNT] = {
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.items.tnt.max")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.items.nuke.max")),
		jsonTokToUlong(pJson, jsonGetDom(pJson, "inventory.items.teleport.max"))
	};

	inventoryInit(pPartsBase, pPartsAddPerLevel, pItemsPrice, pItemsMax);

	jsonDestroy(pJson);
}

void langCreate(const char *szLangPrefix) {
	logBlockBegin("langCreate(szLangPrefix: %s)", szLangPrefix);
	char szPath[100];
	sprintf(szPath, "data/txt_%s/strings.json", szLangPrefix);
	tJson *pJson = jsonCreate(szPath);
	if(!pJson) {
		logWrite("ERR: %s not found\n", szPath);
		return;
	}

	// Shop names
	g_pShopNames = stringArrayCreateFromDom(pJson, g_pRemap, "shopNames");
	g_pCommPageNames = stringArrayCreateFromDom(pJson, g_pRemap, "commPageNames");
	g_pWarehouseColNames = stringArrayCreateFromDom(pJson, g_pRemap, "warehouseColNames");
	g_pMenuEnumPlayerCount = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumPlayerCount");
	g_pMenuEnumP1 = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumP1");
	g_pMenuEnumP2 = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumP2");
	g_pMenuEnumOnOff = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumOnOff");
	g_pMenuCaptions = stringArrayCreateFromDom(pJson, g_pRemap, "menu.captions");
	g_pMineralNames = stringArrayCreateFromDom(pJson, g_pRemap, "minerals");

	g_pMsgs = stringArrayCreateFromDomElements(pJson, g_pRemap, s_pLangDom);

	jsonDestroy(pJson);
	logBlockEnd("langCreate()");
}

void langDestroy(void) {
	logBlockBegin("langDestroy()");
	stringArrayDestroy(g_pShopNames);
	stringArrayDestroy(g_pCommPageNames);
	stringArrayDestroy(g_pWarehouseColNames);
	stringArrayDestroy(g_pMenuEnumPlayerCount);
	stringArrayDestroy(g_pMenuEnumP1);
	stringArrayDestroy(g_pMenuEnumP2);
	stringArrayDestroy(g_pMenuEnumOnOff);
	stringArrayDestroy(g_pMenuCaptions);
	stringArrayDestroy(g_pMineralNames);
	stringArrayDestroy(g_pMsgs);
	logBlockEnd("langDestroy()");
}
