/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inbox.h"
#include <ace/managers/log.h>
#include "../save.h"

#define INBOX_SIZE 20

static tCommShopPage s_pInbox[INBOX_SIZE];
static UWORD s_uwPendingInboxCount;
static UWORD s_uwPopPos;
static tInboxState s_eState;

void inboxReset(void) {
	s_uwPendingInboxCount = 0;
	s_uwPopPos = 0;
	s_eState = INBOX_STATE_NONE;
}

void inboxSave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_INBOX);
	fileWriteLongs(pFile, (ULONG*)s_pInbox, INBOX_SIZE);
	fileWriteWords(pFile, &s_uwPendingInboxCount, 1);
	fileWriteWords(pFile, &s_uwPopPos, 1);
	fileWriteLongs(pFile, (ULONG*)&s_eState, 1);
	saveWriteTag(pFile, SAVE_TAG_INBOX_END);
}

UBYTE inboxLoad(tFile *pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_INBOX)) {
		return 0;
	}

	fileReadLongs(pFile, (ULONG*)s_pInbox, INBOX_SIZE);
	fileReadWords(pFile, &s_uwPendingInboxCount, 1);
	fileReadWords(pFile, &s_uwPopPos, 1);
	fileReadLongs(pFile, (ULONG*)&s_eState, 1);
	return saveReadTag(pFile, SAVE_TAG_INBOX_END);
}


void inboxPushBack(tCommShopPage ePage, UBYTE isUrgent) {
	if(s_uwPendingInboxCount >= INBOX_SIZE) {
		logWrite("ERR: No more room for message %d in inbox\n", ePage);
	}

	if(isUrgent) {
		s_eState = INBOX_STATE_URGENT;
	}
	else if(s_eState == INBOX_STATE_NONE) {
		s_eState = INBOX_STATE_PENDING;
	}
	s_pInbox[s_uwPendingInboxCount++] = ePage;
}

UBYTE inboxTryPopFront(tCommShopPage *ePage) {
	if(s_uwPopPos >= s_uwPendingInboxCount) {
		s_uwPopPos = 0;
		s_uwPendingInboxCount = 0;
		return 0;
	}

	*ePage = s_pInbox[s_uwPopPos++];
	s_eState = INBOX_STATE_NONE;
	return 1;
}

tInboxState inboxGetState(void) {
	return s_eState;
}
