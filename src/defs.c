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
#include "assets.h"

const LONG g_lInitialCash = 0;
const UBYTE g_ubUpgradeLevels = 4;
const UBYTE g_ubDrillingCost = 15;
const UBYTE g_ubLiterPrice = 5, g_ubFuelInLiter = 100, g_ubHullPrice = 2;
const UBYTE g_ubPlansPerAccolade = 10;
const UBYTE g_ubAccoladesInMainStory = 5;
const fix16_t g_fPlanIncreaseRatioSingleplayer = F16(0.25);
const fix16_t g_fPlanIncreaseRatioMultiplayer = F16(0.5);

const UWORD g_pUpgradeCosts[4] = {100, 300, 600, 1000};
const UWORD g_pDinoDepths[DEFS_QUEST_DINO_BONE_COUNT] = {
	80, 130, 170, 190, 220, 230, 250, 270, 290,
};
const UWORD g_pGateDepths[DEFS_QUEST_GATE_PART_COUNT] = {
	235, 255, 275, 300, 330, 360, 390, 420, 450, 480, 510, 540, 570, 600, 630, 660
};
const UWORD g_pCrateDepths[DEFS_QUEST_CRATE_COUNT] = {
	350, 430, 470, 530, 590, 640, 680, 720, 770, 810,
};
const UWORD g_uwCapsuleDepth = 1020;
const UWORD g_uwPrisonerDepth = 220;
const UWORD g_pMineralPlans[MINERAL_TYPE_COUNT] = {0, 10, 30, 40, 20, 65535};
const UBYTE g_ubMinePercentForPlans = 75;
const UBYTE g_ubTrailingMineralCountPercent = 20;
const ULONG g_ulExtraPlanMoney = 50;

const char * const s_pLangDom[] = {
	// Hi score
	[MSG_HI_SCORE_NEW] = "hiScore.new",
	[MSG_HI_SCORE_PRESS] = "hiScore.press",
	[MSG_HI_SCORE_WIN_SCORE] = "hiScore.score",
	[MSG_HI_SCORE_WIN_P1] = "hiScore.winP1",
	[MSG_HI_SCORE_WIN_P2] = "hiScore.winP2",
	[MSG_HI_SCORE_DRAW] = "hiScore.draw",
	// Tutorial
	[MSG_TUTORIAL_INTRODUCTION] =  "tutorial.introduction",
	[MSG_TUTORIAL_GO_MEET_MIETEK] =  "tutorial.start",
	[MSG_TUTORIAL_ON_DUG] =  "tutorial.onDugOut",
	[MSG_TUTORIAL_IN_SHOP] = "tutorial.inShop",
	[MSG_TUTORIAL_IN_WAREHOUSE] = "tutorial.inWarehouse",
	[MSG_TUTORIAL_DESCRIPTION_TAB_OFFICE] = "tutorial.descriptionOffice",
	[MSG_TUTORIAL_DESCRIPTION_TAB_WORKSHOP] = "tutorial.descriptionWorkshop",
	[MSG_TUTORIAL_DESCRIPTION_TAB_WAREHOUSE] = "tutorial.descriptionWarehouse",
	// Achievements
	[MSG_ACHIEVEMENT_TITLE_LAST_RIGHTEOUS] = "achievements[0].name",
	[MSG_ACHIEVEMENT_TITLE_WORK_LEADER] = "achievements[1].name",
	[MSG_ACHIEVEMENT_TITLE_CONFIDENT] = "achievements[2].name",
	[MSG_ACHIEVEMENT_TITLE_SLACKER] = "achievements[3].name",
	[MSG_ACHIEVEMENT_TITLE_ARCHEO_ENTUSIAST] = "achievements[4].name",
	[MSG_ACHIEVEMENT_TITLE_ARCHEO_VICTIM] = "achievements[5].name",
	[MSG_ACHIEVEMENT_TITLE_BATTLE_OF_CENTURY] = "achievements[6].name",
	[MSG_ACHIEVEMENT_TITLE_LOST_WISDOM] = "achievements[7].name",
	[MSG_ACHIEVEMENT_TITLE_MORE_COAL] = "achievements[8].name",
	[MSG_ACHIEVEMENT_TITLE_RECORD_HOLDER] = "achievements[9].name",
	[MSG_ACHIEVEMENT_TITLE_CO_OP] = "achievements[10].name",
	[MSG_ACHIEVEMENT_TITLE_NO_WITNESSES] = "achievements[11].name",
	[MSG_ACHIEVEMENT_TITLE_TIME_PRESSURE] = "achievements[12].name",
	[MSG_ACHIEVEMENT_TITLE_PROTESTS] = "achievements[13].name",
	[MSG_ACHIEVEMENT_TITLE_ESCAPE] = "achievements[14].name",
	[MSG_ACHIEVEMENT_TITLE_NO_ACCOLADES] = "achievements[15].name",
	[MSG_ACHIEVEMENT_TITLE_NO_REBUKES] = "achievements[16].name",
	[MSG_ACHIEVEMENT_TITLE_PLATINUM] = "achievements[17].name",
	[MSG_ACHIEVEMENT_DESC_LAST_RIGHTEOUS] = "achievements[0].desc",
	[MSG_ACHIEVEMENT_DESC_WORK_LEADER] = "achievements[1].desc",
	[MSG_ACHIEVEMENT_DESC_CONFIDENT] = "achievements[2].desc",
	[MSG_ACHIEVEMENT_DESC_SLACKER] = "achievements[3].desc",
	[MSG_ACHIEVEMENT_DESC_ARCHEO_ENTUSIAST] = "achievements[4].desc",
	[MSG_ACHIEVEMENT_DESC_ARCHEO_VICTIM] = "achievements[5].desc",
	[MSG_ACHIEVEMENT_DESC_BATTLE_OF_CENTURY] = "achievements[6].desc",
	[MSG_ACHIEVEMENT_DESC_LOST_WISDOM] = "achievements[7].desc",
	[MSG_ACHIEVEMENT_DESC_MORE_COAL] = "achievements[8].desc",
	[MSG_ACHIEVEMENT_DESC_RECORD_HOLDER] = "achievements[9].desc",
	[MSG_ACHIEVEMENT_DESC_CO_OP] = "achievements[10].desc",
	[MSG_ACHIEVEMENT_DESC_NO_WITNESSES] = "achievements[11].desc",
	[MSG_ACHIEVEMENT_DESC_TIME_PRESSURE] = "achievements[12].desc",
	[MSG_ACHIEVEMENT_DESC_PROTESTS] = "achievements[13].desc",
	[MSG_ACHIEVEMENT_DESC_ESCAPE] = "achievements[14].desc",
	[MSG_ACHIEVEMENT_DESC_NO_ACCOLADES] = "achievements[15].desc",
	[MSG_ACHIEVEMENT_DESC_NO_REBUKES] = "achievements[16].desc",
	[MSG_ACHIEVEMENT_DESC_PLATINUM] = "achievements[17].desc",
	// Shop
	[MSG_COMM_PRESS_SKIP] = "shop.pressSkip",
	[MSG_COMM_TIME_REMAINING] = "shop.timeRemaining",
	[MSG_COMM_ACCOLADES] = "shop.accolades",
	[MSG_COMM_REBUKES] = "shop.rebukes",
	[MSG_COMM_HEAT] = "shop.heat",
	[MSG_COMM_DAYS] = "shop.days",
	[MSG_COMM_PLANS] = "shop.plans",
	[MSG_COMM_PROTESTS_LABEL] = "shop.protestsLabel",
	[MSG_COMM_PROTESTS_OK] = "shop.protestsOk",
	[MSG_COMM_PROTESTS_WARNING] = "shop.protestsWarning",
	[MSG_COMM_PROTESTS_PROTESTS] = "shop.protestsProtests",
	[MSG_COMM_PROTESTS_STRIKE] = "shop.protestsStrike",
	[MSG_COMM_PROTESTS_RIOTS] = "shop.protestsRiots",
	[MSG_COMM_MK] = "shop.mk",
	[MSG_COMM_UPGRADE_TO_MK] = "shop.upgradeToMk",
	[MSG_COMM_STOCK] = "shop.stock",
	[MSG_COMM_PRICE] = "shop.price",
	[MSG_COMM_BUY] = "shop.buy",
	[MSG_COMM_ACCEPT] = "shop.accept",
	[MSG_COMM_GATE_OPENING] = "shop.gateOpening",
	[MSG_COMM_DETONATE] = "shop.detonate",
	[MSG_COMM_DO_NOTHING] = "shop.doNothing",
	[MSG_COMM_ESCAPE_OFFER] = "shop.escapeOffer",
	[MSG_COMM_ESCAPE] = "shop.escape",
	[MSG_COMM_EXIT] = "shop.exit",
	[MSG_COMM_MARKET] = "shop.market",
	// Workshop
	[MSG_COMM_WORKSHOP_TNT_0] = "workshop.tnt0",
	[MSG_COMM_WORKSHOP_TNT_1] = "workshop.tnt1",
	[MSG_COMM_WORKSHOP_TNT_2] = "workshop.tnt2",
	[MSG_COMM_WORKSHOP_TNT_3] = "workshop.tnt3",
	[MSG_COMM_WORKSHOP_TELEPORT_0] = "workshop.teleport0",
	[MSG_COMM_WORKSHOP_TELEPORT_1] = "workshop.teleport1",
	[MSG_COMM_WORKSHOP_TELEPORT_2] = "workshop.teleport2",
	[MSG_COMM_WORKSHOP_PLATFORM_0] = "workshop.platform0",
	[MSG_COMM_WORKSHOP_PLATFORM_1] = "workshop.platform1",
	[MSG_COMM_WORKSHOP_PLATFORM_2] = "workshop.platform2",
	// Unlock
	[MSG_COMM_UNLOCK_OFFICE_WORKSHOP] = "shop.unlockOfficeWorkshop",
	[MSG_COMM_UNLOCK_WAREHOUSE] = "shop.unlockWarehouse",
	// Market
	[MSG_COMM_MARKET_TRADE_COL] = "market.tradeCol",
	[MSG_COMM_MARKET_FOR_COL] = "market.forCol",
	// Crates
	[MSG_CRATES_PREMISE_TELEPORT] = "crates.premiseTeleport",
	[MSG_CRATES_PREMISE_CAPSULE] = "crates.premiseCapsule",
	[MSG_CRATES_PREMISE_SELL] = "crates.premiseSell",
	[MSG_CRATES_NOT_YET_GIVE] = "crates.notYetGive",
	[MSG_CRATES_NOT_YET_SELL] = "crates.notYetSell",
	[MSG_CRATES_GIVE] = "crates.give",
	[MSG_CRATES_SELL] = "crates.sell",
	[MSG_CRATES_USE] = "crates.use",
	[MSG_CRATES_REMAINING] = "crates.remaining",
	// Bonus
	[MSG_BONUS_COMPLETE] = "bonus.complete",
	[MSG_BONUS_LEVEL] = "bonus.level",
	[MSG_BONUS_STEPS] = "bonus.steps",
	// Questioning
	[MSG_QUESTIONING_GATE] = "questioning.gate",
	[MSG_QUESTIONING_CRATE] = "questioning.crate",
	[MSG_QUESTIONING_DESCRIPTION] = "questioning.description",
	[MSG_QUESTIONING_TRUTH] = "questioning.truth",
	[MSG_QUESTIONING_LIE] = "questioning.lie",
	// Tricks
	[MSG_TRICKS_NO_PLAN] = "tricks.noPlan",
	[MSG_TRICKS_ACCOUNING_STRIKE] = "tricks.accoutingStrike",
	[MSG_TRICKS_ACCOUNTING_PREMISE] = "tricks.accountingPremise",
	[MSG_TRICKS_ACCOUNTING_DETAILS] = "tricks.accountingDetails",
	[MSG_TRICKS_BRIBE_PREMISE] = "tricks.bribePremise",
	[MSG_TRICKS_BRIBE_DETAILS] = "tricks.bribeDetails",
	[MSG_TRICKS_BRIBE_NO_CASH] = "tricks.bribeNoCash",
	[MSG_TRICKS_BRIBE_USED] = "tricks.bribeUsed",
	[MSG_TRICKS_BRIBE_PRICE] = "tricks.bribePrice",
	// Challenge
	[MSG_CHALLENGE_CHECKPOINT] = "challenge.checkpoint",
	[MSG_CHALLENGE_TELEPORT] = "challenge.teleport",
	// Misc
	[MSG_MISC_DRILL_DEPLETED] = "misc.drillDepleted",
	[MSG_MISC_CARGO_FULL] = "misc.cargoFull",
	[MSG_MISC_RESTOCK] = "misc.restock",
	[MSG_MISC_FOUND_BONE] = "misc.foundBone",
	[MSG_MISC_FOUND_GATE] = "misc.foundGate",
	[MSG_MISC_FOUND_CRATE] = "misc.foundCrate",
	[MSG_MISC_FOUND_CAPSULE] = "misc.foundCapsule",
	// HUD: UI
	[MSG_HUD_P1] = "hud.p1",
	[MSG_HUD_P2] = "hud.p2",
	[MSG_HUD_DRILL] = "hud.drill",
	[MSG_HUD_CARGO] = "hud.cargo",
	[MSG_HUD_HULL] = "hud.hull",
	[MSG_HUD_CASH] = "hud.cash",
	[MSG_HUD_DEPTH] = "hud.depth",
	[MSG_HUD_TIME] = "hud.time",
	[MSG_HUD_PAUSED] = "hud.paused",
	[MSG_HUD_RESUME] = "hud.resume",
	[MSG_HUD_SAVE_QUIT] = "hud.saveQuit",
	[MSG_HUD_QUIT] = "hud.quit",
	// HUD: Misc
	[MSG_HUD_DINO_COMPLETE] = "hudMessages.dinoComplete",
	[MSG_HUD_DINO_FOUND_BONE] = "hudMessages.dinoFoundBone",
	[MSG_HUD_RADIO_START_0] = "hudMessages.radioStart[0]",
	[MSG_HUD_RADIO_START_1] = "hudMessages.radioStart[1]",
	[MSG_HUD_RADIO_START_2] = "hudMessages.radioStart[2]",
	[MSG_HUD_RADIO_HALF_0] = "hudMessages.radioHalf[0]",
	[MSG_HUD_RADIO_HALF_1] = "hudMessages.radioHalf[1]",
	[MSG_HUD_RADIO_HALF_2] = "hudMessages.radioHalf[2]",
	[MSG_HUD_RADIO_FULL_0] = "hudMessages.radioFull[0]",
	[MSG_HUD_RADIO_FULL_1] = "hudMessages.radioFull[1]",
	[MSG_HUD_RADIO_FULL_2] = "hudMessages.radioFull[2]",
	// HUD: Plan
	[MSG_HUD_ACHIEVEMENT_UNLOCKED] = "hudMessages.achievementUnlocked",
	[MSG_HUD_NEW_PLAN] = "hudMessages.newPlan",
	[MSG_HUD_PLAN_REMAINING] = "hudMessages.remaining",
	[MSG_HUD_PLAN_EXTENDING] = "hudMessages.extending",
	[MSG_HUD_DEADLINE_REMAINING] = "hudMessages.deadlineRemaining",
	[MSG_HUD_WAITING_KOMISARZ] = "hudMessages.waitingKomisarz",
	[MSG_HUD_GUEST] = "hudMessages.guest",
	[MSG_HUD_WAITING_URZEDAS] = "hudMessages.waitingUrzedas",
	[MSG_HUD_SCI_WELCOME] = "hudMessages.sciWelcome",
	[MSG_HUD_PRISONER_FOUND] = "hudMessages.prisonerFound",
	[MSG_HUD_CAPSULE_FOUND] = "hudMessages.capsuleFound",
	// Loading
	[MSG_LOADING_GEN_TERRAIN] = "loading.genTerrain",
	[MSG_LOADING_GEN_BASES] = "loading.genBases",
	[MSG_LOADING_FINISHING] = "loading.finishing",
	// Office
	[MSG_PAGE_BACK] = "officePages.commonBack",
	[MSG_PAGE_LIST_MIETEK] = "officePages.listMietek",
	[MSG_PAGE_LIST_KRYSTYNA] = "officePages.listKrystyna",
	[MSG_PAGE_LIST_KOMISARZ] = "officePages.listPutin",
	[MSG_PAGE_LIST_URZEDAS] = "officePages.listUrzedas",
	[MSG_PAGE_LIST_ARCH] = "officePages.listArch",
	[MSG_PAGE_LIST_PRISONER] = "officePages.listPrisoner",
	[MSG_PAGE_LIST_AGENT] = "officePages.listAgent",
	[MSG_PAGE_LIST_SCI] = "officePages.listSci",
	[MSG_PAGE_LIST_CRYO] = "officePages.listCryo",
	[MSG_PAGE_LIST_JAY] = "officePages.listJay",
	[MSG_PAGE_MIETEK_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_MIETEK_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_MIETEK_FIRST_PLAN] = "officePages.mietekFirstPlan",
	[MSG_PAGE_MIETEK_PLAN_COMPLETE] = "officePages.mietekPlanComplete",
	[MSG_PAGE_KRYSTYNA_PROTEST_WARNING] = "officePages.mietekProtestWarning",
	[MSG_PAGE_MIETEK_PROTEST_START] = "officePages.mietekProtestStart",
	[MSG_PAGE_MIETEK_PROTEST_STRIKE] = "officePages.mietekProtestStrike",
	[MSG_PAGE_MIETEK_CAPSULE_FOUND] = "officePages.mietekCapsuleFound",
	[MSG_PAGE_KRYSTYNA_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_KRYSTYNA_ACCOUNTING] = "officePages.krystynaAccounting",
	[MSG_PAGE_URZEDAS_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_URZEDAS_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_URZEDAS_FIRST_PLAN] = "officePages.urzedasFirstPlan",
	[MSG_PAGE_URZEDAS_PLAN_COMPLETE] = "officePages.planComplete",
	[MSG_PAGE_URZEDAS_PLAN_DELAYED] = "officePages.planDelayed",
	[MSG_PAGE_URZEDAS_PLAN_ACCOLADE] = "officePages.planAccolade",
	[MSG_PAGE_URZEDAS_BRIBE] = "officePages.urzedasBribe",
	[MSG_PAGE_KOMISARZ_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_KOMISARZ_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_KOMISARZ_DINO_INTRO] = "officePages.komisarzDinoIntro",
	[MSG_PAGE_KOMISARZ_REBUKE_1] = "officePages.komisarzRebuke_1",
	[MSG_PAGE_KOMISARZ_REBUKE_2] = "officePages.komisarzRebuke_2",
	[MSG_PAGE_KOMISARZ_REBUKE_3] = "officePages.komisarzRebuke_3",
	[MSG_PAGE_KOMISARZ_QUESTIONING] = "officePages.komisarzQuestioning",
	[MSG_PAGE_KOMISARZ_REPORTING_LIST] = "officePages.komisarzReportList",
	[MSG_PAGE_KOMISARZ_REPORTING_GATE] = "officePages.komisarzReportGate",
	[MSG_PAGE_KOMISARZ_REPORTING_TELEPORT_PARTS] = "officePages.komisarzReportParts",
	[MSG_PAGE_KOMISARZ_REPORTING_AGENT] = "officePages.komisarzReportAgent",
	[MSG_PAGE_KOMISARZ_ARCH_ACCOLADE] = "officePages.komisarzArchAccolade",
	[MSG_PAGE_KOMISARZ_QUESTIONING_ACCOLADE] = "officePages.questioningAccolade",
	[MSG_KOMISARZ_GATE_OPENING] = "officePages.gateOpening",
	[MSG_PAGE_ARCH_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_ARCH_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_ARCH_PLAN_FAIL] = "officePages.archPlanFail",
	[MSG_PAGE_ARCH_GATE_DESTROYED] = "officePages.commonGateDestroyed",
	[MSG_PAGE_ARCH_ACCOLADE] = "officePages.archAccolade",
	[MSG_PAGE_ARCH_GATE_OPENED_DINO_INCOMPLETE] = "officePages.archGateOpened",
	[MSG_PAGE_ARCH_GATE_OPENED_DINO_COMPLETE] = "officePages.archGateOpened",
	[MSG_PAGE_PRISONER_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_PRISONER_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_PRISONER_GATE_DESTROYED] = "officePages.commonGateDestroyed",
	[MSG_PAGE_PRISONER_RADIO_1] = "officePages.prisonerRadio1",
	[MSG_PAGE_PRISONER_RADIO_2] = "officePages.prisonerRadio2",
	[MSG_PAGE_PRISONER_RADIO_3] = "officePages.prisonerRadio3",
	[MSG_PAGE_AGENT_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_AGENT_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_AGENT_SCIENTISTS] = "officePages.agentSci",
	[MSG_PAGE_AGENT_SELL_CRATE] = "officePages.agentSellCrates",
	[MSG_PAGE_AGENT_ESCAPE] = "officePages.commonEscape",
	[MSG_PAGE_AGENT_EPILOGUE] = "officePages.commonEpilogue",
	[MSG_PAGE_SCIENTIST_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_SCIENTIST_WELCOME] = "officePages.commonWelcome",
	[MSG_PAGE_SCIENTIST_FIRST_CRATE] = "officePages.sciFirstCrate",
	[MSG_PAGE_SCIENTIST_CRATE_TELEPORTER] = "officePages.sciCrateTeleporter",
	[MSG_PAGE_SCIENTIST_ESCAPE] = "officePages.commonEscape",
	[MSG_PAGE_SCIENTIST_EPILOGUE] = "officePages.commonEpilogue",

	[MSG_PAGE_CRYO_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_CRYO_TRAMIEL] = "officePages.cryoTramiel",
	[MSG_PAGE_CRYO_CONSOLE] = "officePages.cryoConsole",
	[MSG_PAGE_CRYO_SUCCESS] = "officePages.cryoConsole",

	[MSG_PAGE_JAY_DOSSIER] = "officePages.commonDossier",
	[MSG_PAGE_JAY_CONGRATS] = "officePages.jayCongrats",

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

void defsCreateLocale(const char *szLangPrefix) {
	logBlockBegin("defsCreateLocale(szLangPrefix: %s)", szLangPrefix);
	char szPath[30];
	sprintf(szPath, SUBFILE_PREFIX "txt_%s/strings.json", szLangPrefix);
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
	logBlockEnd("defsCreateLocale()");
}

void defsDestroyLocale(void) {
	logBlockBegin("defsDestroyLocale()");
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
	logBlockEnd("defsDestroyLocale()");
}
