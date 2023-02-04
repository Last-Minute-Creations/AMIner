/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "game.h"
#include <comm/gs_msg.h>
#include <comm/gs_shop.h>
#include <comm/inbox.h>
#include <comm/page_office.h>
#include "warehouse.h"
#include "vehicle.h"
#include "hud.h"
#include "defs.h"
#include "save.h"

typedef enum _tTutorialState {
	TUTORIAL_SHOW_MESSAGE_INTRO = 0,
	TUTORIAL_GO_MEET_MIETEK,
	TUTORIAL_WAIT_FOR_URZEDAS,
	TUTORIAL_GO_MEET_URZEDAS,
	TUTORIAL_WAIT_FOR_KOMISARZ,
	TUTORIAL_GO_MEET_KOMISARZ,
	TUTORIAL_WAIT_FOR_PLAN,
	TUTORIAL_GO_READ_PLAN,
	TUTORIAL_WAITING_FOR_DIG,
	TUTORIAL_WAITING_FOR_RESTOCK,
	TUTORIAL_SHOW_PLAN_FILL_MSG,
	TUTORIAL_WAITING_FOR_PLAN_DONE,
	TUTORIAL_DONE
} tTutorialState;

static tTutorialState s_eTutorialState = TUTORIAL_SHOW_MESSAGE_INTRO;
static UBYTE s_pDescriptionShownForTabs[COMM_TAB_COUNT];
static ULONG s_ulStartTime;

//------------------------------------------------------------------ PRIVATE FNS

static UBYTE tutorialProcessChallenge(void) {
	UBYTE isEarlyReturn = 0;
	switch(s_eTutorialState) {
		case TUTORIAL_SHOW_MESSAGE_INTRO:
			gsMsgInit("intro_challenge");
			statePush(g_pGameStateManager, &g_sStateMsg);
			s_eTutorialState = TUTORIAL_DONE;
			isEarlyReturn = 1;
			break;
		case TUTORIAL_DONE:
			break;
		default:
			break;
	}

	return isEarlyReturn;
}

static UBYTE tutorialIsInMainOffice(void) {
	return (
		commShopIsActive() &&
		commShopPageToTab(commShopGetCurrentPage()) == COMM_TAB_OFFICE
	);
}

static UBYTE tutorialProcessStory(void) {
	if(commShopIsActive()) {
		tCommShopPage ePage = commShopGetCurrentPage();
		tCommTab eTab = commShopPageToTab(ePage);
		if(eTab != COMM_TAB_COUNT) {
			if(!s_pDescriptionShownForTabs[eTab]) {
				if(!hudIsShowingMessage()) {
					s_pDescriptionShownForTabs[eTab] = 1;
					hudShowMessage(
						FACE_ID_MIETEK,
						g_pMsgs[MSG_TUTORIAL_DESCRIPTION_TAB_OFFICE + eTab - COMM_TAB_OFFICE]
					);
				}
			}
		}
	}

	UBYTE isEarlyReturn = 0;
	switch(s_eTutorialState) {
		case TUTORIAL_SHOW_MESSAGE_INTRO:
			gsMsgInit("intro_campaign");
			statePush(g_pGameStateManager, &g_sStateMsg);
			++s_eTutorialState;
			isEarlyReturn = 1;
			break;
		case TUTORIAL_GO_MEET_MIETEK:
			pageOfficeUnlockPersonSubpage(FACE_ID_MIETEK, COMM_SHOP_PAGE_OFFICE_MIETEK_WELCOME);
			inboxPushBack(COMM_SHOP_PAGE_OFFICE_MIETEK_WELCOME, 0);
			hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_GO_MEET_MIETEK]);
			s_ulStartTime = gameGetTime();
			++s_eTutorialState;
			break;
		case TUTORIAL_WAIT_FOR_URZEDAS:
			if(gameIsElapsedDays(s_ulStartTime, 3)) {
				pageOfficeUnlockPerson(FACE_ID_URZEDAS);
				pageOfficeUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_DOSSIER);
				pageOfficeUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_WELCOME);
				pageOfficeUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_BRIBE);
				inboxPushBack(COMM_SHOP_PAGE_OFFICE_URZEDAS_WELCOME, 0);
				hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_GUEST	]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_GO_MEET_URZEDAS:
			if(tutorialIsInMainOffice()) { // Message read
				s_ulStartTime = gameGetTime();
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAIT_FOR_KOMISARZ:
			if(gameIsElapsedDays(s_ulStartTime, 3)) {
				pageOfficeUnlockPerson(FACE_ID_KOMISARZ);
				pageOfficeUnlockPersonSubpage(FACE_ID_KOMISARZ, COMM_SHOP_PAGE_OFFICE_KOMISARZ_DOSSIER);
				pageOfficeUnlockPersonSubpage(FACE_ID_KOMISARZ, COMM_SHOP_PAGE_OFFICE_KOMISARZ_WELCOME);
				inboxPushBack(COMM_SHOP_PAGE_OFFICE_KOMISARZ_WELCOME, 0);
				hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_GUEST]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_GO_MEET_KOMISARZ:
			if(tutorialIsInMainOffice()) { // Message read
				s_ulStartTime = gameGetTime();
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAIT_FOR_PLAN:
			if(gameIsElapsedDays(s_ulStartTime, 3)) {
				pageOfficeUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_FIRST_PLAN);
				inboxPushBack(COMM_SHOP_PAGE_OFFICE_URZEDAS_FIRST_PLAN, 1);
				hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_WAITING_URZEDAS]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_GO_READ_PLAN:
			if(tutorialIsInMainOffice()) { // Message read
					planStart(warehouseGetCurrentPlan());
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_DIG: {
			const tPlan *pPlan = warehouseGetCurrentPlan();
			UWORD uwTotal = (
				g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] +
				(g_is2pPlaying ? g_pVehicles[1].pStock[MINERAL_TYPE_SILVER] : 0) +
				warehouseGetStock(MINERAL_TYPE_SILVER) +
				pPlan->pMinerals[MINERAL_TYPE_SILVER].uwCurrentCount
			);
			if(uwTotal >= pPlan->pMinerals[MINERAL_TYPE_SILVER].uwTargetCount) {
				if(!commShopIsActive()) {
					hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_ON_DUG]);
					++s_eTutorialState;
				}
				else {
					s_eTutorialState = TUTORIAL_SHOW_PLAN_FILL_MSG;
				}
			}
		} break;
		case TUTORIAL_WAITING_FOR_RESTOCK: {
			UWORD uwCarried = (
				g_pVehicles[0].pStock[MINERAL_TYPE_SILVER] +
				g_pVehicles[1].pStock[MINERAL_TYPE_SILVER]
			);
			if(uwCarried == 0) {
				++s_eTutorialState;
			}
		} break;
		case TUTORIAL_SHOW_PLAN_FILL_MSG:
			if(commShopIsActive() && !hudIsShowingMessage()) {
				hudShowMessage(FACE_ID_MIETEK, g_pMsgs[MSG_TUTORIAL_IN_SHOP]);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_WAITING_FOR_PLAN_DONE:
			if(warehouseGetCurrentPlan()->uwIndex > 0) {
				pageOfficeUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_COMPLETE);
				inboxPushBack(COMM_SHOP_PAGE_OFFICE_URZEDAS_PLAN_COMPLETE, 0);
				++s_eTutorialState;
			}
			break;
		case TUTORIAL_DONE:
			break;
	}
	return isEarlyReturn;
}

//------------------------------------------------------------------- PUBLIC FNS

void tutorialReset(void) {
	for(UBYTE i = 0; i < COMM_TAB_COUNT; ++i) {
		s_pDescriptionShownForTabs[i] = 0;
	}

	s_eTutorialState = TUTORIAL_SHOW_MESSAGE_INTRO;
}

void tutorialSave(tFile *pFile) {
	saveWriteHeader(pFile, "TUTR");
	fileWrite(pFile, &s_eTutorialState, sizeof(s_eTutorialState));
	fileWrite(pFile, s_pDescriptionShownForTabs, sizeof(s_pDescriptionShownForTabs[0]) * COMM_TAB_COUNT);
	fileWrite(pFile, &s_ulStartTime, sizeof(s_ulStartTime));
}

UBYTE tutorialLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "TUTR")) {
		return 0;
	}

	fileRead(pFile, &s_eTutorialState, sizeof(s_eTutorialState));
	fileRead(pFile, s_pDescriptionShownForTabs, sizeof(s_pDescriptionShownForTabs[0]) * COMM_TAB_COUNT);
	fileRead(pFile, &s_ulStartTime, sizeof(s_ulStartTime));

	return 1;
}

UBYTE tutorialProcess(void) {
	if(g_isChallenge) {
		return tutorialProcessChallenge();
	}
	else {
		return tutorialProcessStory();
	}
}
