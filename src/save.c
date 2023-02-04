/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "save.h"
#include <string.h>
#include <ace/managers/log.h>

UBYTE saveReadHeader(tFile *pFile, const char *szHeader) {
	char szHeaderRead[5] = {'\0'};
	fileRead(pFile, szHeaderRead, sizeof(szHeaderRead) - 1);
	if(memcmp(szHeaderRead, szHeader, sizeof(szHeaderRead) - 1)) {
		logWrite(
			"ERR: Save header mismatch, got %s, expected %s\n", szHeaderRead, szHeader
		);
		return 0;
	}

	return 1;
}

void saveWriteHeader(tFile *pFile, const char *szHeader) {
	fileWrite(pFile, szHeader, strlen(szHeader));
}
