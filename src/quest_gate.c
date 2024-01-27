/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "quest_gate.h"
#include "collectibles.h"
#include <comm/page_questioning.h>
#include <comm/page_office.h>
#include <comm/inbox.h>
#include "hud.h"
#include "save.h"
#include "game.h"

typedef enum tQuestGateState {
	QUEST_GATE_STATE_UNEXPLODED,
	QUEST_GATE_STATE_EXPLODED,

} tQuestGateState;

static tQuestGateState s_eState;
static UBYTE s_ubFoundFragments;

void questGateReset(void) {
	s_eState = QUEST_GATE_STATE_UNEXPLODED;
	s_ubFoundFragments = 0;
}

void questGateSave(tFile *pFile) {
	saveWriteHeader(pFile, "GATE");
	fileWrite(pFile, &s_ubFoundFragments, sizeof(s_ubFoundFragments));
	fileWrite(pFile, &s_eState, sizeof(s_eState));
}

UBYTE questGateLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "GATE")) {
		return 0;
	}

	fileRead(pFile, &s_ubFoundFragments, sizeof(s_ubFoundFragments));
	fileRead(pFile, &s_eState, sizeof(s_eState));
	collectibleSetFoundCount(COLLECTIBLE_KIND_GATE, s_ubFoundFragments);
	return 1;
}

void questGateProcess(void) {

}

UBYTE questGateAddFragment(void) {
	const UBYTE ubMaxFragmentCount = questGateGetMaxFragmentCount();
	if(s_ubFoundFragments < ubMaxFragmentCount) {
		++s_ubFoundFragments;
		collectibleSetFoundCount(COLLECTIBLE_KIND_GATE, s_ubFoundFragments);
		pageQuestioningTrySetPendingQuestioning(QUESTIONING_BIT_GATE);
	}
	else {
		gameTriggerCutscene(GAME_CUTSCENE_TELEPORT);
	}

	return s_ubFoundFragments;
}

void questGateMarkExploded(void) {
	s_eState = QUEST_GATE_STATE_EXPLODED;
	collectibleSetFoundCount(COLLECTIBLE_KIND_GATE, 0);
}

UBYTE questGateIsExploded(void) {
	return s_eState == QUEST_GATE_STATE_EXPLODED;
}

UBYTE questGateGetFoundFragmentCount(void) {
	return s_ubFoundFragments;
}

UBYTE questGateGetMaxFragmentCount(void) {
	return collectibleGetMaxCount(COLLECTIBLE_KIND_GATE);
}

void questGateUnlockPrisoner(void) {
	hudShowMessage(
		FACE_ID_MIETEK,
		"Panie derektorze, wyglada na wyglodzonego.\nPrzywiezie go Pan do bazy."
	);
	pageOfficeUnlockPerson(FACE_ID_PRISONER);
	pageOfficeTryUnlockPersonSubpage(FACE_ID_PRISONER, COMM_SHOP_PAGE_OFFICE_PRISONER_DOSSIER);
	pageOfficeTryUnlockPersonSubpage(FACE_ID_PRISONER, COMM_SHOP_PAGE_OFFICE_PRISONER_WELCOME);
	inboxPushBack(COMM_SHOP_PAGE_OFFICE_PRISONER_WELCOME, 1);
}

UBYTE questGateIsPrisonerFound(void) {
	return pageOfficeHasPerson(FACE_ID_PRISONER);
}
