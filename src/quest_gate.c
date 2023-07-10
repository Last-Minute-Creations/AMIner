/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "quest_gate.h"
#include "collectibles.h"

typedef enum tQuestGateState {
	QUEST_GATE_STATE_WAITING_FOR_START
} tQuestGateState;

static tQuestGateState s_eState;
static UBYTE s_ubFoundFragments;

void questGateReset(void) {
	s_eState = QUEST_GATE_STATE_WAITING_FOR_START;
	s_ubFoundFragments = 0;
}

void questGateSave(tFile *pFile) {

}

UBYTE questGateLoad(tFile *pFile) {
	return 1;
}

void questGateProcess(void) {

}

UBYTE questGateAddFragment(void) {
	const UBYTE ubMaxFragmentCount = collectibleGetMaxCount(COLLECTIBLE_KIND_GATE);
	if(s_ubFoundFragments < ubMaxFragmentCount) {
		++s_ubFoundFragments;
		collectibleSetFoundCount(COLLECTIBLE_KIND_GATE, s_ubFoundFragments);
	}

	return s_ubFoundFragments;
}
