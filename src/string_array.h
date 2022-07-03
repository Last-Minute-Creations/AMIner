/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _STRING_ARRAY_H_
#define _STRING_ARRAY_H_

#include "json/json.h"

/**
 * Array of string pointers terimnated with nullptr
 */

#define STRING_ARRAY_EMPTY_POS ((char*)-1)
#define STRING_ARRAY_TERMINATOR ((char*)0)

char **stringArrayCreateFromDom(
	tJson *pJson, const tCodeRemap *pRemap, const char *szDom
);

char **stringArrayCreateFromDomElements(
	tJson *pJson, const tCodeRemap *pRemap, const char **pNames
);

void stringArrayDestroy(char **pArray);

UBYTE stringArrayGetCount(const char **pArray);

#endif // _STRING_ARRAY_H_
