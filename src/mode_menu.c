/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mode_menu.h"
#include <ace/utils/bitmap.h>
#include "vehicle.h"
#include "mode_menu.h"
#include "game.h"

#define MODE_ICON_SIZE 12
#define MODE_ICON_MARGIN 4

//----------------------------------------------------------------- PRIVATE VARS

static tBitMap *s_pModeIcons;
static tBitMap *s_pModeIconMask;
static UBYTE *s_pModeIconOffsets[MODE_OPTION_COUNT];

//------------------------------------------------------------------ PRIVATE FNS

//------------------------------------------------------------------- PUBLIC FNS

void modeMenuManagerCreate(void) {
	s_pModeIcons = bitmapCreateFromFile("data/mode_icons.bm", 0);
	s_pModeIconMask = bitmapCreateFromFile("data/mode_icon_mask.bm", 0);

	for(tModeOption eOption = 0; eOption < MODE_OPTION_COUNT; ++eOption) {
		s_pModeIconOffsets[eOption] = bobCalcFrameAddress(s_pModeIcons, eOption * MODE_ICON_SIZE);
	}
}

void modeMenuManagerDestroy(void) {
	bitmapDestroy(s_pModeIcons);
	bitmapDestroy(s_pModeIconMask);
}

void modeMenuReset(tModeMenu *pModeMenu, UBYTE ubPlayerIndex) {
	modeMenuClearOptions(0);
	modeMenuAddOption(0, MODE_OPTION_DRILL);
	pModeMenu->ubCurrent = 0;
	pModeMenu->isActive = 0;
	pModeMenu->ubPlayerIndex = ubPlayerIndex;

	for(UBYTE ubBobIndex = 0; ubBobIndex < MODE_BOBS_PER_PLAYER; ++ubBobIndex) {
		bobInit(
			&pModeMenu->pBobs[ubBobIndex],
			CEIL_TO_FACTOR(MODE_ICON_SIZE, 16), MODE_ICON_SIZE,
			1, 0, s_pModeIconMask->Planes[0], 0, 0
		);
	}
}

void modeMenuClearOptions(tModeMenu *pModeMenu) {
	pModeMenu->ubCount = 0;
}

void modeMenuAddOption(tModeMenu *pModeMenu, tModeOption eOption) {
	UBYTE ubIndex = pModeMenu->ubCount;
	pModeMenu->pModeOptions[ubIndex] = eOption;
	pModeMenu->pBobs[ubIndex].pFrameData = s_pModeIconOffsets[eOption];
	pModeMenu->ubCount = ubIndex + 1;
}

void modeMenuProcess(tModeMenu *pModeMenu, tDirection eDirection) {
	if(eDirection == DIRECTION_LEFT) {
		if(pModeMenu->ubCurrent) {
			--pModeMenu->ubCurrent;
		}
		else {
			pModeMenu->ubCurrent = pModeMenu->ubCount - 1;
		}
	}
	else if(eDirection == DIRECTION_RIGHT) {
		++pModeMenu->ubCurrent;
		if(pModeMenu->ubCurrent >= pModeMenu->ubCount) {
			pModeMenu->ubCurrent = 0;
		}
	}
}

void modeMenuEnterSelecion(tModeMenu *pModeMenu) {
	pModeMenu->isActive = 1;
}

tModeOption modeMenuExitSelection(tModeMenu *pModeMenu) {
	pModeMenu->isActive = 0;
	return modeMenuGetSelected(pModeMenu);
}

tModeOption modeMenuGetSelected(tModeMenu *pModeMenu) {
	return pModeMenu->pModeOptions[pModeMenu->ubCurrent];
}

void modeMenuTryDisplay(tModeMenu *pModeMenu) {
	if(!pModeMenu->isActive) {
		return;
	}

	const tVehicle *pVehicle = &g_pVehicles[pModeMenu->ubPlayerIndex];
	UWORD uwX = pVehicle->sBobBody.sPos.uwX + VEHICLE_WIDTH / 2;
	UWORD uwY = pVehicle->sBobBody.sPos.uwY - (MODE_ICON_SIZE + MODE_ICON_MARGIN);

	for(UBYTE ubOptionIndex = 0; ubOptionIndex < pModeMenu->ubCount; ++ubOptionIndex) {
		pModeMenu->pBobs[ubOptionIndex].sPos.uwX = uwX + ubOptionIndex * (MODE_ICON_SIZE + MODE_ICON_MARGIN);
		pModeMenu->pBobs[ubOptionIndex].sPos.uwY = uwY;
		gameTryPushBob(&pModeMenu->pBobs[ubOptionIndex]);
	}

}
