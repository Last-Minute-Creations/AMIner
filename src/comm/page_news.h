/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _AMINER_COMM_PAGE_NEWS_H_
#define _AMINER_COMM_PAGE_NEWS_H_

#include "game.h"

typedef enum tNewsKind {
	NEWS_KIND_ACCOLADES,
} tNewsKind;

void pageNewsCreate(tNewsKind eEnding);

#endif // _AMINER_COMM_PAGE_NEWS_H_
