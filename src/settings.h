/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ace/utils/file.h>

typedef struct tSettings {
	UBYTE ubSoundVolume;
	UBYTE ubMusicVolume;
	UBYTE is1pKbd;
	UBYTE is2pKbd;
	UBYTE isAtariHidden;
} tSettings;

extern tSettings g_sSettings;

void settingsSave(tFile *pFile);

UBYTE settingsLoad(tFile*pFile);
