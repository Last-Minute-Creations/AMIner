/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _AMINER_COMM_INBOX_H_
#define _AMINER_COMM_INBOX_H_

#include "gs_shop.h"

typedef struct tInboxMessage {
	tCommShopPage ePage;
	// TODO: something to tell shop which next page to show - for going to default state, triggering rebuke game over
} tInboxMessage;

void inboxPushBack(tCommShopPage ePage, UBYTE isUrgent);

UBYTE inboxTryPopBack(tCommShopPage *ePage);

UBYTE inboxIsUrgent(void);

#endif // _AMINER_COMM_INBOX_H_
