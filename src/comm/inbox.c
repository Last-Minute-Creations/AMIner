/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inbox.h"
#include <ace/managers/log.h>
#include "../save.h"

#define INBOX_SIZE 10

static tCommShopPage s_pInbox[INBOX_SIZE];
static UWORD s_uwPendingInboxCount;
static UWORD s_uwPopPos;
static UBYTE s_isUrgent;

void inboxReset(void) {
	s_uwPendingInboxCount = 0;
	s_uwPopPos = 0;
	s_isUrgent = 0;
}

void inboxSave(tFile *pFile) {
	saveWriteHeader(pFile, "INBX");
	fileWrite(pFile, s_pInbox, sizeof(s_pInbox));
	fileWrite(pFile, &s_uwPendingInboxCount, sizeof(s_uwPendingInboxCount));
	fileWrite(pFile, &s_uwPopPos, sizeof(s_uwPopPos));
	fileWrite(pFile, &s_isUrgent, sizeof(s_isUrgent));
}

UBYTE inboxLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "INBX")) {
		return 0;
	}

	fileRead(pFile, s_pInbox, sizeof(s_pInbox));
	fileRead(pFile, &s_uwPendingInboxCount, sizeof(s_uwPendingInboxCount));
	fileRead(pFile, &s_uwPopPos, sizeof(s_uwPopPos));
	fileRead(pFile, &s_isUrgent, sizeof(s_isUrgent));
	return 1;
}


void inboxPushBack(tCommShopPage ePage, UBYTE isUrgent) {
	if(s_uwPendingInboxCount >= INBOX_SIZE) {
		logWrite("ERR: No more room for message %d in inbox\n", ePage);
	}

	s_isUrgent |= isUrgent;
	s_pInbox[s_uwPendingInboxCount++] = ePage;
}

UBYTE inboxTryPopFront(tCommShopPage *ePage) {
	if(s_uwPopPos >= s_uwPendingInboxCount) {
		s_uwPopPos = 0;
		s_uwPendingInboxCount = 0;
		return 0;
	}

	*ePage = s_pInbox[s_uwPopPos++];
	s_isUrgent = 0;
	return 1;
}

UBYTE inboxIsUrgent(void) {
	return s_isUrgent;
}
