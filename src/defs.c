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

LONG g_lInitialCash;
UBYTE g_ubUpgradeLevels;
UWORD g_uwPartDrillBase, g_uwPartDrillPerLevel;
UWORD g_uwPartCargoBase, g_uwPartCargoPerLevel;
UWORD g_uwPartHullBase, g_uwPartHullPerLevel;
UBYTE g_ubDrillingCost;
UBYTE g_ubLiterPrice, g_ubFuelInLiter, g_ubHullPrice;

LONG g_pUpgradeCosts[10];
UWORD g_pDinoDepths[9];

void defsInit(void) {
	tJson *pJson = jsonCreate("data/game.json");

	g_lInitialCash = jsonTokToUlong(pJson, jsonGetDom(pJson, "initialCash"), 10);

	// Upgrade costs
	UWORD uwIdxUpgradeCosts = jsonGetDom(pJson, "upgradeCosts");
	g_ubUpgradeLevels = MIN(10, pJson->pTokens[uwIdxUpgradeCosts].size);

	for(UBYTE i = 0; i < pJson->pTokens[uwIdxUpgradeCosts].size; ++i) {
		UWORD uwIdxCost = jsonGetElementInArray(pJson, uwIdxUpgradeCosts, i);
		g_pUpgradeCosts[i] = jsonTokToUlong(pJson, uwIdxCost, 10);
	}

	// Hull
	g_uwPartHullBase = jsonTokToUlong(pJson, jsonGetDom(pJson, "hull.base"), 10);
	g_uwPartHullPerLevel = jsonTokToUlong(pJson, jsonGetDom(pJson, "hull.addPerLevel"), 10);

	// Drill
	g_uwPartDrillBase = jsonTokToUlong(pJson, jsonGetDom(pJson, "drill.base"), 10);
	g_uwPartDrillPerLevel = jsonTokToUlong(pJson, jsonGetDom(pJson, "drill.addPerLevel"), 10);

	// Cargo
	g_uwPartCargoBase = jsonTokToUlong(pJson, jsonGetDom(pJson, "cargo.base"), 10);
	g_uwPartCargoPerLevel = jsonTokToUlong(pJson, jsonGetDom(pJson, "cargo.addPerLevel"), 10);

	g_ubDrillingCost = jsonTokToUlong(pJson, jsonGetDom(pJson, "drillingCost"), 10);

	// Restock
	g_ubLiterPrice = jsonTokToUlong(pJson, jsonGetDom(pJson, "restock.literPrice"), 10);
	g_ubFuelInLiter = jsonTokToUlong(pJson, jsonGetDom(pJson, "restock.fuelInLiter"), 10);
	g_ubHullPrice = jsonTokToUlong(pJson, jsonGetDom(pJson, "restock.hullPrice"), 10);

	// Dino parts
	UWORD uwIdxDinoDepths = jsonGetDom(pJson, "dinoDepths");
	UBYTE ubDepthCount = pJson->pTokens[uwIdxDinoDepths].size;
	if(ubDepthCount != 9) {
		logWrite("Dino part count mismatch: got %d, expected 9\n", ubDepthCount);
	}
	for(UBYTE i = 0; i < ubDepthCount; ++i) {
		UWORD uwIdxCost = jsonGetElementInArray(pJson, uwIdxDinoDepths, i);
		g_pDinoDepths[i] = jsonTokToUlong(pJson, uwIdxCost, 10);
	}

	jsonDestroy(pJson);
}

void langCreate(const char *szLangPrefix) {
	logBlockBegin("langCreate()");
	char szPath[100];
	sprintf(szPath, "data/strings_%s.json", szLangPrefix);
	tJson *pJson = jsonCreate(szPath);
	if(!pJson) {
		logWrite("ERR: %s not found\n", szPath);
		return;
	}

	// Shop names
	g_sShopNames = stringArrayCreateFromDom(pJson, "shopNames");
	g_sWarehouseColNames = stringArrayCreateFromDom(pJson, "warehouseColNames");

	// Plan messages
	g_sPlanMessages = stringArrayCreateFromDomElements(
		pJson, 2, "planMessages.doneAfk", "planMessages.notDone"
	);

	// Hi score messages
	g_sHiScoreMessages = stringArrayCreateFromDomElements(
		pJson, 2, "hiScoreMessages.new", "hiScoreMessages.press"
	);

	g_sMenuEnumMode = stringArrayCreateFromDom(pJson, "menu.enumMode");
	g_sMenuEnumPlayerCount = stringArrayCreateFromDom(pJson, "menu.enumPlayerCount");
	g_sMenuEnumP1 = stringArrayCreateFromDom(pJson, "menu.enumP1");
	g_sMenuEnumP2 = stringArrayCreateFromDom(pJson, "menu.enumP2");
	g_sMenuEnumOnOff = stringArrayCreateFromDom(pJson, "menu.enumOnOff");
	g_sMenuCaptions = stringArrayCreateFromDom(pJson, "menu.captions");
	g_sMineralNames = stringArrayCreateFromDom(pJson, "minerals");
	g_sLoadMsgs = stringArrayCreateFromDom(pJson, "loadMsgs");

	g_sTutorialMsgs = stringArrayCreateFromDomElements(
		pJson, 5, "tutorial.start", "tutorial.onDugOut", "tutorial.nearShop",
		"tutorial.inShop", "tutorial.onMoveToPlan"
	);

	g_sMessages = stringArrayCreateFromDomElements(
		pJson, MSG_COUNT, "challengeCheckpoint", "challengeTeleport",
		"drillDepleted", "cargoFull", "restock", "foundBone"
	);

	jsonDestroy(pJson);
	logBlockEnd("langCreate()");
}

void langDestroy(void) {
	logBlockBegin("langDestroy()");
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
	logBlockEnd("langDestroy()");
}
