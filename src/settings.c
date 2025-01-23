/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settings.h"
#include "save.h"

// Default config
tSettings g_sSettings;

//------------------------------------------------------------------ PRIVATE FNS

static void settingsSave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_SETTINGS);
	fileWrite(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileWrite(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileWrite(pFile, &g_sSettings.isAtariHidden, sizeof(g_sSettings.isAtariHidden));
	fileWrite(pFile, &g_sSettings.ubSokoUnlock, sizeof(g_sSettings.ubSokoUnlock));
	fileWrite(pFile, &g_sSettings.ulAchievementsUnlocked, sizeof(g_sSettings.ulAchievementsUnlocked));
	saveWriteTag(pFile, SAVE_TAG_SETTINGS_END);
}

static UBYTE settingsLoad(tFile*pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_SETTINGS)) {
		return 0;
	}

	fileRead(pFile, &g_sSettings.is1pKbd, sizeof(g_sSettings.is1pKbd));
	fileRead(pFile, &g_sSettings.is2pKbd, sizeof(g_sSettings.is2pKbd));
	fileRead(pFile, &g_sSettings.isAtariHidden, sizeof(g_sSettings.isAtariHidden));
	fileRead(pFile, &g_sSettings.ubSokoUnlock, sizeof(g_sSettings.ubSokoUnlock));
	fileRead(pFile, &g_sSettings.ulAchievementsUnlocked, sizeof(g_sSettings.ulAchievementsUnlocked));
	return saveReadTag(pFile, SAVE_TAG_SETTINGS_END);
}

static void settingsReset(void) {
	g_sSettings.is1pKbd = 0;
	g_sSettings.is2pKbd = 1;
	g_sSettings.isAtariHidden = 1;
	g_sSettings.ubSokoUnlock = 0;
	g_sSettings.ulAchievementsUnlocked = 0;
}

//------------------------------------------------------------------- PUBLIC FNS

void settingsFileSave(void) {
	tFile *pFileSettings = diskFileOpen("settings.dat", "wb");
	if(pFileSettings) {
		settingsSave(pFileSettings);
		fileClose(pFileSettings);
		logWrite("Saved settings\n");
	}
}

void settingsFileLoad(void) {
	tFile *pFileSettings = diskFileOpen("settings.dat", "rb");
	if(pFileSettings) {
		if(settingsLoad(pFileSettings)) {
			logWrite("Loaded settings\n");
		}
		else {
			logWrite("ERR: Can't load settings - restoring defaults\n");
			settingsReset();
		}

		fileClose(pFileSettings);
	}
}

UBYTE settingsTryUnlockAchievement(UBYTE ubAchievementIndex) {
	if(g_sSettings.ulAchievementsUnlocked & BV(ubAchievementIndex)) {
		return 0;
	}
	g_sSettings.ulAchievementsUnlocked |= BV(ubAchievementIndex);
	return 1;
}
