/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dino.h"
#include "collectibles.h"
#include <ace/managers/bob.h>
#include "core.h"
#include "game.h"
#include "hud.h"
#include "save.h"
#include "tile.h"
#include "comm/gs_shop.h"
#include "comm/inbox.h"
#include "comm/page_office.h"

typedef enum tDinoState {
	DINO_STATE_WAITING_FOR_FIRST_BONE,
	DINO_STATE_INCOMING_BRIEFING,
	DINO_STATE_WAITING_FOR_READING_BRIEFING,
	DINO_STATE_WAITING_FOR_LAST_BONE,
	DINO_STATE_INCOMING_ACCOLADE,
	DINO_STATE_WAITING_FOR_RECEIVING_ACCOLADE,
	DINO_STATE_DONE,
} tDinoState;

static UBYTE s_ubDinoBonesFound = 0;
static tDinoState s_eQuestState;

void dinoReset(void) {
	s_ubDinoBonesFound = 0;
	s_eQuestState = DINO_STATE_WAITING_FOR_FIRST_BONE;
	baseUpdateDinoTileset(0);
	collectibleSetFoundCount(COLLECTIBLE_KIND_DINO, 0);
}

void dinoSave(tFile *pFile) {
	saveWriteHeader(pFile, "DINO");
	fileWrite(pFile, &s_ubDinoBonesFound, sizeof(s_ubDinoBonesFound));
	fileWrite(pFile, &s_eQuestState, sizeof(s_eQuestState));
}

UBYTE dinoLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "DINO")) {
		return 0;
	}

	fileRead(pFile, &s_ubDinoBonesFound, sizeof(s_ubDinoBonesFound));
	fileRead(pFile, &s_eQuestState, sizeof(s_eQuestState));
	UBYTE isBasePopulated = s_eQuestState > DINO_STATE_WAITING_FOR_READING_BRIEFING;
	baseUpdateDinoTileset(isBasePopulated);
	collectibleSetFoundCount(COLLECTIBLE_KIND_DINO, s_ubDinoBonesFound);
	return 1;
}

void dinoProcess(void) {
	switch(s_eQuestState) {
		case DINO_STATE_WAITING_FOR_FIRST_BONE:
			break;
		case DINO_STATE_INCOMING_BRIEFING:
				pageOfficeUnlockPerson(FACE_ID_ARCH);
				pageOfficeTryUnlockPersonSubpage(FACE_ID_URZEDAS, COMM_SHOP_PAGE_OFFICE_URZEDAS_DINO_INTRO);
				pageOfficeTryUnlockPersonSubpage(FACE_ID_ARCH, COMM_SHOP_PAGE_OFFICE_ARCH_DOSSIER);
				pageOfficeTryUnlockPersonSubpage(FACE_ID_ARCH, COMM_SHOP_PAGE_OFFICE_ARCH_WELCOME);

				inboxPushBack(COMM_SHOP_PAGE_OFFICE_URZEDAS_DINO_INTRO, 1);
				inboxPushBack(COMM_SHOP_PAGE_OFFICE_ARCH_WELCOME, 1);
				hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_WAITING_URZEDAS]);
				++s_eQuestState;
			break;
		case DINO_STATE_WAITING_FOR_READING_BRIEFING:
			if(commShopIsActive()) {
				// TODO: check if on surface?
				tileReplaceBaseWithVariant(BASE_ID_DINO, BASE_ID_DINO_POPULATED);
				baseUpdateDinoTileset(1);
				++s_eQuestState;
			}
			break;
		case DINO_STATE_WAITING_FOR_LAST_BONE:
			break;
		case DINO_STATE_INCOMING_ACCOLADE:
			pageOfficeTryUnlockPersonSubpage(FACE_ID_ARCH, COMM_SHOP_PAGE_OFFICE_ARCH_ACCOLADE);

			inboxPushBack(COMM_SHOP_PAGE_OFFICE_ARCH_ACCOLADE, 1);
			hudShowMessage(FACE_ID_KRYSTYNA, g_pMsgs[MSG_HUD_GUEST]); // TODO: better message
			++s_eQuestState;
			break;
		case DINO_STATE_WAITING_FOR_RECEIVING_ACCOLADE:
			if(commShopIsActive()) {
				gameAddAccolade();
				++s_eQuestState;
			}
			break;
		case DINO_STATE_DONE:
			break;
	}
}

UBYTE dinoAddBone(void) {
	const UBYTE ubMaxFragmentCount = collectibleGetMaxCount(COLLECTIBLE_KIND_DINO);
	if(s_ubDinoBonesFound < ubMaxFragmentCount) {
		++s_ubDinoBonesFound;
		collectibleSetFoundCount(COLLECTIBLE_KIND_DINO, s_ubDinoBonesFound);
	}

	if(s_ubDinoBonesFound == 1) {
		s_eQuestState = DINO_STATE_INCOMING_BRIEFING;
	}
	else if(s_ubDinoBonesFound == ubMaxFragmentCount) {
		s_eQuestState = DINO_STATE_INCOMING_ACCOLADE;
	}

	return s_ubDinoBonesFound;
}
