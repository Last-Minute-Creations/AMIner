/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _STRING_ARRAY_H_
#define _STRING_ARRAY_H_

#include "json/json.h"

typedef struct _tStringArray {
	UBYTE ubCount;
	char **pStrings;
} tStringArray;

tStringArray stringArrayCreateFromDom(
	tJson *pJson, const tJsonRemap *pRemap, const char *szDom
);

tStringArray stringArrayCreateFromDomElements(
	tJson *pJson, const tJsonRemap *pRemap, UBYTE ubCount, ...
);

void stringArrayDestroy(tStringArray *pArray);

#endif // _STRING_ARRAY_H_
