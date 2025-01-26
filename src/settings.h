/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <ace/utils/file.h>

#define SETTINGS_SOKO_UNLOCK_ON 0xD0

typedef struct tSettings {
	UBYTE ubSoundVolume;
	UBYTE ubMusicVolume;
	UBYTE is1pKbd;
	UBYTE is2pKbd;
	UBYTE isAtariHidden;
	UBYTE ubSokoUnlock;
	ULONG ulAchievementsUnlocked;
} tSettings;

extern tSettings g_sSettings;

void settingsFileSave(void);

void settingsFileLoad(void);

UBYTE settingsTryUnlockAchievement(UBYTE ubAchievementIndex);

#endif // SETTINGS_H
