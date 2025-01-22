/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settings.h"
#include "save.h"

// Default config
tSettings g_sSettings = {
	.is1pKbd = 0,
	.is2pKbd = 1,
	.isAtariHidden = 1,
	.ubSokoUnlock = 0,
};

void settingsSave(tFile *pFile) {
	saveWriteHeader(pFile, "STGS");
	fileWrite(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileWrite(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileWrite(pFile, &g_sSettings.isAtariHidden, sizeof(g_sSettings.isAtariHidden));
	fileWrite(pFile, &g_sSettings.ubSokoUnlock, sizeof(g_sSettings.ubSokoUnlock));
}

UBYTE settingsLoad(tFile*pFile) {
	if(!saveReadHeader(pFile, "STGS")) {
		return 0;
	}

	fileRead(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileRead(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileRead(pFile, &g_sSettings.isAtariHidden, sizeof(g_sSettings.isAtariHidden));
	fileRead(pFile, &g_sSettings.ubSokoUnlock, sizeof(g_sSettings.ubSokoUnlock));
	return 1;
}
