/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hi_score.h"
#include <ace/managers/key.h>
#include "game.h"
#include "text_bob.h"

#define SCORE_NAME_LENGTH 20

typedef struct _tHiScore {
	ULONG ulScore;
	char szName[SCORE_NAME_LENGTH];
} tHiScore;

static tHiScore s_pScores[10] = {
	{.ulScore = 10, .szName = "Bestest"},
	{.ulScore = 9, .szName = "Best"},
	{.ulScore = 8, .szName = "Better"},
	{.ulScore = 7, .szName = "Good"},
	{.ulScore = 6, .szName = "Moderate"},
	{.ulScore = 5, .szName = "Bad"},
	{.ulScore = 4, .szName = "Awful"},
	{.ulScore = 3, .szName = "Too"},
	{.ulScore = 2, .szName = "Small"},
	{.ulScore = 1, .szName = "Score"},
};


static UBYTE s_ubNewNameLength;
static UBYTE s_ubNewScorePos;
static UBYTE s_isEnteringHiScore;
static tTextBob s_pScoreNameBobs[10];
static tTextBob s_pScoreCountBobs[10];

void hiScoreEnteringProcess(void) {
	if(keyUse(KEY_RETURN) || keyUse(KEY_NUMENTER)) {
		// if(s_ubNewNameLength) {
		// 	scoreSave();
		// }
		s_isEnteringHiScore = 0;
		return;
	}
	if(keyUse(g_sKeyManager.ubLastKey)) {
		UBYTE isUpdateNeeded = 1;
		char c = g_pToAscii[g_sKeyManager.ubLastKey];
		if(
			(c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9')
		) {
			if(s_ubNewNameLength < SCORE_NAME_LENGTH) {
				s_pScores[s_ubNewScorePos].szName[s_ubNewNameLength] = c;
				++s_ubNewNameLength;
			}
		}
		else if(g_sKeyManager.ubLastKey == KEY_BACKSPACE && s_ubNewNameLength){
			--s_ubNewNameLength;
			s_pScores[s_ubNewScorePos].szName[s_ubNewNameLength] = '\0';
		}
		else {
			isUpdateNeeded = 0;
		}
		if(isUpdateNeeded) {
			textBobSetText(
				&s_pScoreNameBobs[s_ubNewScorePos], "%hhu. %s",
				s_ubNewScorePos+1, s_pScores[s_ubNewScorePos].szName
			);
			textBobUpdate(&s_pScoreNameBobs[s_ubNewScorePos]);
		}
	}
}

void hiScoreBobsDisplay(void) {
	for(UBYTE i = 0; i < 10; ++i) {
		bobNewPush(&s_pScoreNameBobs[i].sBob);
		bobNewPush(&s_pScoreCountBobs[i].sBob);
	}
}

UBYTE hiScoreIsEntering(void) {
	return s_isEnteringHiScore;
}

void hiScoreSetup(ULONG ulScore) {
	s_isEnteringHiScore = 0;
	for(UBYTE i = 0; i < 10; ++i) {
		if(s_pScores[i].ulScore < ulScore) {
			s_isEnteringHiScore = 1;
			s_ubNewScorePos = i;
			s_pScores[i].ulScore = ulScore;
			memset(s_pScores[i].szName, '\0', SCORE_NAME_LENGTH);
			break;
		}
	}

	for(UBYTE i = 0; i < 10; ++i) {
		// Score name
		UWORD uwScorePos = g_pMainBuffer->pCamera->uPos.sUwCoord.uwY + 70 + i * 10;
		textBobSetText(&s_pScoreNameBobs[i], "%hhu. %s", i+1, s_pScores[i].szName);
		textBobSetColor(&s_pScoreNameBobs[i], 14);
		textBobSetPosition(&s_pScoreNameBobs[i], 32+64, uwScorePos, 0, 0);
		textBobUpdate(&s_pScoreNameBobs[i]);
		// Score count
		textBobSetText(&s_pScoreCountBobs[i], "%lu", s_pScores[i].ulScore);
		textBobSetColor(&s_pScoreCountBobs[i], 14);
		textBobSetPosition(
			&s_pScoreCountBobs[i],
			(32+320-64) - s_pScoreCountBobs[i].uwWidth, uwScorePos, 0, 0
		);
		textBobUpdate(&s_pScoreCountBobs[i]);
	}
	s_ubNewNameLength = 0;
}

void hiScoreBobsCreate(void) {
	for(UBYTE i = 0; i < 10; ++i) {
		textBobCreate(&s_pScoreNameBobs[i], g_pFont, "10. ZZZZZZZZZZZZZZZZZZZZZ");
		textBobCreate(&s_pScoreCountBobs[i], g_pFont, "655356");
	}
}

void hiScoreBobsDestroy(void) {
	for(UBYTE i = 0; i < 10; ++i) {
		textBobDestroy(&s_pScoreNameBobs[i]);
		textBobDestroy(&s_pScoreCountBobs[i]);
	}
}
