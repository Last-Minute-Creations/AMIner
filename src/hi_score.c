/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hi_score.h"
#include <ace/managers/key.h>
#include "game.h"
#include "text_bob.h"
#include <ace/managers/system.h>

#define SCORE_NAME_LENGTH 20
#define SCORE_COUNT 10

typedef struct _tHiScore {
	ULONG ulScore;
	char szName[SCORE_NAME_LENGTH];
} tHiScore;

static tHiScore s_pScores[SCORE_COUNT];

static tHiScore s_pPrevScores[SCORE_COUNT] = {
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
static UBYTE s_isShift = 0;
static tTextBob s_pScoreNameBobs[SCORE_COUNT];
static tTextBob s_pScoreCountBobs[SCORE_COUNT];
// static tTextBob s_sEndMessage;

void hiScoreLoad(void) {
	systemUse();
	tFile *pFile = fileOpen("scores.dat", "r");
	if(pFile) {
		logWrite("opened scores file");
		for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
			fileRead(pFile, &s_pScores[i], sizeof(s_pScores[i]));
		}
		fileClose(pFile);
		CopyMem(s_pScores, s_pPrevScores, sizeof(s_pPrevScores));
	}
	else {
		CopyMem(s_pPrevScores, s_pScores, sizeof(s_pScores));
	}
	systemUnuse();
}

static void hiScoreSave(void) {
	systemUse();
	tFile *pFile = fileOpen("scores.dat", "w");
	if(pFile) {
		for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
			fileWrite(pFile, &s_pScores[i], sizeof(s_pScores[i]));
		}
		fileClose(pFile);
		CopyMem(s_pScores, s_pPrevScores, sizeof(s_pPrevScores));
	}
	systemUnuse();
}

void hiScoreEnteringProcess(void) {
	if(keyUse(KEY_RETURN) || keyUse(KEY_NUMENTER)) {
		if(s_ubNewNameLength) {
			hiScoreSave();
		}
		else {
			// No entry provided - revert to old scores
			// not as hiScoreLoadFromFile() because it won't work if file was
			// not yet created
			CopyMem(s_pPrevScores, s_pScores, sizeof(s_pScores));
		}
		s_isEnteringHiScore = 0;
		return;
	}
	UBYTE isUpdateNeeded = 0;
	if(keyUse(KEY_LSHIFT) || keyUse(KEY_RSHIFT)) {
		s_isShift = 1;
	}
	else if(
		s_isShift &&
		keyCheck(KEY_LSHIFT) == KEY_NACTIVE &&
		keyCheck(KEY_RSHIFT) == KEY_NACTIVE
	) {
		s_isShift = 0;
	}
	else if(keyUse(g_sKeyManager.ubLastKey)) {
		isUpdateNeeded = 1;
		WORD wInput = g_pToAscii[g_sKeyManager.ubLastKey];
		if(s_isShift) {
			wInput -= 'a' - 'A';
		}
		if(
			(wInput >= 'A' && wInput <= 'Z') ||
			(wInput >= 'a' && wInput <= 'z') ||
			(wInput >= '0' && wInput <= '9')
		) {
			if(s_ubNewNameLength < SCORE_NAME_LENGTH) {
				s_pScores[s_ubNewScorePos].szName[s_ubNewNameLength] = wInput;
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
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		bobNewPush(&s_pScoreNameBobs[i].sBob);
		bobNewPush(&s_pScoreCountBobs[i].sBob);
	}
	// if(!s_isEnteringHiScore) {
	// 	bobNewPush(&s_sEndMessage.sBob);
	// }
}

UBYTE hiScoreIsEntering(void) {
	return s_isEnteringHiScore;
}

void hiScoreSetup(ULONG ulScore) {
	s_isEnteringHiScore = 0;
	s_ubNewNameLength = 0;
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		if(s_pScores[i].ulScore < ulScore) {
			s_isEnteringHiScore = 1;
			s_isShift = 0;
			s_ubNewScorePos = i;

			// Move worse score down
			for(BYTE j = SCORE_COUNT-2; j >= s_ubNewScorePos; --j) {
				strcpy(s_pScores[j+1].szName, s_pScores[j].szName);
				s_pScores[j+1].ulScore = s_pScores[j].ulScore;
			}
			// Make room for new score
			s_pScores[s_ubNewScorePos].ulScore = ulScore;
			memset(s_pScores[s_ubNewScorePos].szName, '\0', SCORE_NAME_LENGTH);
			break;
		}
	}
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		// Score name
		UWORD uwScorePos = g_pMainBuffer->pCamera->uPos.uwY + 70 + i * 10;
		textBobSetText(&s_pScoreNameBobs[i], "%hhu. %s", i+1, s_pScores[i].szName);
		textBobSetPos(&s_pScoreNameBobs[i], 32+64, uwScorePos, 0, 0);
		textBobUpdate(&s_pScoreNameBobs[i]);
		// Score count
		textBobSetText(&s_pScoreCountBobs[i], "%lu", s_pScores[i].ulScore);
		textBobSetPos(
			&s_pScoreCountBobs[i],
			(32+320-64) - s_pScoreCountBobs[i].uwWidth, uwScorePos, 0, 0
		);
		textBobUpdate(&s_pScoreCountBobs[i]);
	}
	// End text
	// UWORD uwCenterX = 160 + g_pMainBuffer->pCamera->uPos.uwX;
	// textBobSetPos(
	// 	&s_sEndMessage, uwCenterX, g_pMainBuffer->pCamera->uPos.uwY + 200, 0, 1
	// );
	// textBobUpdate(&s_sEndMessage);
}

void hiScoreBobsCreate(void) {
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		textBobCreate(&s_pScoreNameBobs[i], g_pFont, "10. ZZZZZZZZZZZZZZZZZZZZZ");
		textBobSetColor(&s_pScoreNameBobs[i], 14);
		textBobCreate(&s_pScoreCountBobs[i], g_pFont, "655356");
		textBobSetColor(&s_pScoreCountBobs[i], 14);
	}
	// textBobCreate(&s_sEndMessage, g_pFont, "Press button to continue");
	// textBobSetColor(&s_sEndMessage, 14);
}

void hiScoreBobsDestroy(void) {
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		textBobDestroy(&s_pScoreNameBobs[i]);
		textBobDestroy(&s_pScoreCountBobs[i]);
	}
	// textBobDestroy(&s_sEndMessage);
}
