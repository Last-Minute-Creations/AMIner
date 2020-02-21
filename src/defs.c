/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "defs.h"
#include "json/json.h"
#include <ace/managers/log.h>

#include "comm_shop.h"
#include "hi_score.h"
#include "menu.h"
#include "game.h"
#include "mineral.h"
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

LONG g_pUpgradeCosts[10];
UWORD g_pDinoDepths[9];

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
		UWORD uwIdxCost = jsonGetElementInArray(pJson, uwIdxDinoDepths, i);
		g_pDinoDepths[i] = jsonTokToUlong(pJson, uwIdxCost);
	}

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
	logBlockBegin("langCreate()");
	char szPath[100];
	sprintf(szPath, "data/txt_%s/strings.json", szLangPrefix);
	tJson *pJson = jsonCreate(szPath);
	if(!pJson) {
		logWrite("ERR: %s not found\n", szPath);
		return;
	}

	// Shop names
	g_sShopNames = stringArrayCreateFromDom(pJson, g_pRemap, "shopNames");
	g_sWarehouseColNames = stringArrayCreateFromDom(pJson, g_pRemap, "warehouseColNames");

	// Plan messages
	g_sPlanMessages = stringArrayCreateFromDomElements(
		pJson, g_pRemap, MSG_PLAN_COUNT, "planMessages.doneAfk",
		"planMessages.notDone", "planMessages.remaining", "planMessages.extending"
	);

	// Hi score messages
	g_sHiScoreMessages = stringArrayCreateFromDomElements(
		pJson, g_pRemap, MSG_HI_SCORE_COUNT, "hiScore.new", "hiScore.press",
		"hiScore.score", "hiScore.winP1", "hiScore.winP2", "hiScore.draw"
	);

	g_sMenuEnumMode = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumMode");
	g_sMenuEnumPlayerCount = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumPlayerCount");
	g_sMenuEnumP1 = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumP1");
	g_sMenuEnumP2 = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumP2");
	g_sMenuEnumOnOff = stringArrayCreateFromDom(pJson, g_pRemap, "menu.enumOnOff");
	g_sMenuCaptions = stringArrayCreateFromDom(pJson, g_pRemap, "menu.captions");
	g_sMineralNames = stringArrayCreateFromDom(pJson, g_pRemap, "minerals");
	g_sLoadMsgs = stringArrayCreateFromDom(pJson, g_pRemap, "loadMsgs");

	g_sTutorialMsgs = stringArrayCreateFromDomElements(
		pJson, g_pRemap, 5, "tutorial.start", "tutorial.onDugOut", "tutorial.nearShop",
		"tutorial.inShop", "tutorial.onMoveToPlan"
	);

	g_sShopMsgs = stringArrayCreateFromDomElements(
		pJson, g_pRemap, 11, "shop.timeRemaining", "shop.accolades", "shop.rebukes",
		"shop.mk", "shop.upgradeToMk", "shop.stock", "shop.buy", "shop.exit",
		"shop.confirm", "shop.alreadyMax", "shop.alreadyFull"
	);

	g_sMessages = stringArrayCreateFromDomElements(
		pJson, g_pRemap, MSG_COUNT, "challengeCheckpoint", "challengeTeleport",
		"drillDepleted", "cargoFull", "restock", "foundBone"
	);

	g_sMsgHud = stringArrayCreateFromDomElements(
		pJson, g_pRemap, MSG_HUD_COUNT, "hud.p1", "hud.p2", "hud.drill",
		"hud.cargo", "hud.hull", "hud.cash", "hud.depth",
		"hud.paused", "hud.resume", "hud.quit"
	);

	g_sOfficePageNames = stringArrayCreateFromDomElements(
		pJson, g_pRemap, 10,
		"officePages.main", "officePages.listMietek", "officePages.listKrystyna",
		"officePages.listPutin", "officePages.listUrzedas",
		"officePages.dossierKrystyna", "officePages.dossierUrzedas",
		"officePages.bribe", "officePages.favor",
		"officePages.accounting"
	);

	jsonDestroy(pJson);
	logBlockEnd("langCreate()");
}

void langDestroy(void) {
	logBlockBegin("langDestroy()");
	stringArrayDestroy(&g_sOfficePageNames);
	stringArrayDestroy(&g_sShopMsgs);
	stringArrayDestroy(&g_sShopNames);
	stringArrayDestroy(&g_sWarehouseColNames);

	stringArrayDestroy(&g_sPlanMessages);
	stringArrayDestroy(&g_sHiScoreMessages);

	stringArrayDestroy(&g_sMenuEnumMode);
	stringArrayDestroy(&g_sMenuEnumPlayerCount);
	stringArrayDestroy(&g_sMenuEnumP1);
	stringArrayDestroy(&g_sMenuEnumP2);
	stringArrayDestroy(&g_sMenuEnumOnOff);
	stringArrayDestroy(&g_sMenuCaptions);
	stringArrayDestroy(&g_sMineralNames);
	stringArrayDestroy(&g_sLoadMsgs);

	stringArrayDestroy(&g_sTutorialMsgs);
	stringArrayDestroy(&g_sMessages);
	stringArrayDestroy(&g_sMsgHud);
	logBlockEnd("langDestroy()");
}
