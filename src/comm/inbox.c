/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inbox.h"
#include <ace/managers/log.h>

#define INBOX_SIZE 10

static tCommShopPage s_pInbox[INBOX_SIZE];
static UWORD s_uwPendingInboxCount;
static UBYTE s_isUrgent;

void inboxCreate(void) {
	s_uwPendingInboxCount = 0;
	s_isUrgent = 0;
}

void inboxPushBack(tCommShopPage ePage, UBYTE isUrgent) {
	if(s_uwPendingInboxCount >= INBOX_SIZE) {
		logWrite("ERR: No more room for message %d in inbox\n", ePage);
	}

	s_isUrgent |= isUrgent;
	s_pInbox[s_uwPendingInboxCount++] = ePage;
}

UBYTE inboxTryPopBack(tCommShopPage *ePage) {
	if(!s_uwPendingInboxCount) {
		return 0;
	}

	*ePage = s_pInbox[--s_uwPendingInboxCount];
	s_isUrgent = 0;
	return 1;
}

UBYTE inboxIsUrgent(void) {
	return s_isUrgent;
}
