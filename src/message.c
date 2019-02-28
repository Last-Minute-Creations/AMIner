/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "message.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/utils/font.h>
#include "game.h"
#include "color.h"
#include "window.h"

UBYTE s_isShown;
tBitMap *s_pBuffer;
static tTextBitMap *s_pTextBitmap;

static const char *s_pMessages[] = {
	"Comrade!",
	"",
	"You have been sent to this facility by the decision",
	"of Ministry of Mining and Fossile Fuels. The outpost",
	"is small but we believe not yet fully explored",
	"and examined. As you know, development of our",
	"great Democratic Peoples Republic requires significant",
	"expenditures in the sector of heavy industry.",
	"Industry that will astound and amaze with its",
	"creations not only imperialist in the West but the",
	"whole world. Industry that is so important for our ",
	"great Army that protects our Motherland and helps",
	"to liberate more and more nations from the imperial",
	"and capitalist occupant.",
	"",
	"Comrade, your task here is very simple, yet",
	"so much important. All you need to do is to fulfill",
	"the mining plans that will be sent to you in the form",
	"of a telegram from the Ministry of Mining and Fossile",
	"Fuels. To help you achieve that goals we are giving",
	"under your command hard working employees.",
	"They are true communists and the sons of this land.",
	// PAGE 2
	"We are also giving you our latest",
	"and newest technology and machinery together with",
	"some great amount of spare parts in case you need",
	"them. ",
	""
	"Our Great Leader trusts its Comrades but likes",
	"to keep an eye on different aspects of life of his",
	"fellow countrymen. After all he is also just a simple",
	"farmer from the province. Therefore an observant",
	"from Ministry of National Security will be sent out",
	"to your facility. You can trust him like own brother",
	"and we expect you will inform him of any special",
	"findings and discoveries, should you encounter any",
	"in your underground work.",
	"",
	"Good luck Comrade!"
};
const UBYTE s_ubMessageCount = sizeof(s_pMessages) / sizeof(s_pMessages[0]);

static tUwCoordYX s_sOrigin;

static void messageDrawPage(UBYTE ubNo) {
	blitRect(s_pBuffer, s_sOrigin.sUwCoord.uwX + 4, s_sOrigin.sUwCoord.uwY + 8, WINDOW_WIDTH - 8, WINDOW_HEIGHT - 16, 0);
	UWORD uwOffsY = 16;
	for(UBYTE i = ubNo*22; i < (ubNo+1)*22 && i < s_ubMessageCount; ++i) {
		fontFillTextBitMap(g_pFont, s_pTextBitmap, s_pMessages[i]);
		fontDrawTextBitMap(
			s_pBuffer, s_pTextBitmap, s_sOrigin.sUwCoord.uwX + 8,
			s_sOrigin.sUwCoord.uwY + uwOffsY, COLOR_GOLD, FONT_COOKIE | FONT_SHADOW
		);
		uwOffsY += g_pFont->uwHeight;
	}
	uwOffsY += g_pFont->uwHeight;
	fontFillTextBitMap(g_pFont, s_pTextBitmap, "Press FIRE, SPACE or ENTER to continue");
	fontDrawTextBitMap(
		s_pBuffer, s_pTextBitmap, s_sOrigin.sUwCoord.uwX + 8,
		s_sOrigin.sUwCoord.uwY + uwOffsY, COLOR_GREEN, FONT_COOKIE | FONT_SHADOW
	);
}

static UBYTE s_ubPage = 0;

void messageGsCreate(void) {
	s_isShown = windowShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_pBuffer = g_pMainBuffer->pScroll->pBack;
	s_pTextBitmap = fontCreateTextBitMap(
		WINDOW_WIDTH, g_pFont->uwHeight
	);
	s_sOrigin = windowGetOrigin();

	s_ubPage = 0;
	messageDrawPage(0);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void messageGsLoop(void) {
	if(keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || keyUse(KEY_ESCAPE)) {
		if(s_ubPage >= 1) {
			gamePopState();
			return;
		}
		else {
			++s_ubPage;
			messageDrawPage(s_ubPage);
		}
	}
}

void messageGsDestroy(void) {
	if(!s_isShown) {
		return;
	}
	fontDestroyTextBitMap(s_pTextBitmap);
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	windowHide();
}
