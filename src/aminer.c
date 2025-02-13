/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "aminer.h"
#define GENERIC_MAIN_LOOP_CONDITION g_pGameStateManager->pCurrent
#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/ptplayer.h>
#include "logo.h"
#include "sorry.h"
#include "assets.h"

tStateManager *g_pGameStateManager;

void genericCreate(void) {
	keyCreate();
	joyOpen();
	g_pGameStateManager = stateManagerCreate();

	// Bare minimum
	ptplayerCreate(1);
	ptplayerSetChannelsForPlayer(0b0111);
	ptplayerSetMasterVolume(8);
	g_pFont = fontCreateFromPath("data/uni54.fnt");

	if(memGetChipSize() < (1024+512) * 1024) {
		statePush(g_pGameStateManager, &g_sStateSorry);
	}
	else {
		statePush(g_pGameStateManager, &g_sStateLogo);
	}
}

void genericProcess(void) {
	gameExit();
	keyProcess();
	joyProcess();
	stateProcess(g_pGameStateManager);
}

void genericDestroy(void) {
	fontDestroy(g_pFont);
	ptplayerDestroy();

	stateManagerDestroy(g_pGameStateManager);
	keyDestroy();
	joyClose();
}
