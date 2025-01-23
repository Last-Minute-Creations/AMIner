/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "save.h"
#include <string.h>
#include <ace/managers/log.h>

UBYTE saveReadTag(tFile *pFile, const char *szTag) {
	char szTagRead[5] = {'\0'};
	fileRead(pFile, szTagRead, sizeof(szTagRead) - 1);
	if(memcmp(szTagRead, szTag, sizeof(szTagRead) - 1)) {
		logWrite(
			"ERR: Save tag mismatch, got %s, expected %s\n", szTagRead, szTag
		);
		return 0;
	}

	return 1;
}

void saveWriteTag(tFile *pFile, const char *szTag) {
	fileWrite(pFile, szTag, strlen(szTag));
}
