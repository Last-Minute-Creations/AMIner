/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hi_score.h"
#include <ace/managers/key.h>
#include "game.h"
#include "text_bob.h"
#include <ace/managers/system.h>
#include <ace/managers/timer.h>

#define SCORE_NAME_LENGTH 20
#define SCORE_COUNT 10
#define SCORE_CURSOR_BLINK_TICKS 25 // 25 ticks = 500ms

typedef struct _tHiScore {
	LONG lScore;
	char szName[SCORE_NAME_LENGTH];
} tHiScore;

static tHiScore s_pScores[SCORE_COUNT];

static tHiScore s_pPrevScores[SCORE_COUNT] = {
	{.lScore = 10, .szName = "Bestest"},
	{.lScore = 9, .szName = "Best"},
	{.lScore = 8, .szName = "Better"},
	{.lScore = 7, .szName = "Good"},
	{.lScore = 6, .szName = "Moderate"},
	{.lScore = 5, .szName = "Bad"},
	{.lScore = 4, .szName = "Awful"},
	{.lScore = 3, .szName = "Too"},
	{.lScore = 2, .szName = "Small"},
	{.lScore = 1, .szName = "Score"},
};


static UBYTE s_ubNewNameLength;
static UBYTE s_ubNewScorePos;
static UBYTE s_isEnteringHiScore;
static UBYTE s_isShift = 0;
static UBYTE s_isCursor = 0;
static ULONG s_ulCursorStart = 0;
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

static void hiScoreUpdateScoreBobs(void) {
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		// Score name
		UWORD uwScorePos = g_pMainBuffer->pCamera->uPos.uwY + 70 + i * 10;
		textBobSetText(&s_pScoreNameBobs[i], "%hhu. %s", i+1, s_pScores[i].szName);
		textBobSetPos(&s_pScoreNameBobs[i], 32+64, uwScorePos, 0, 0);
		textBobUpdate(&s_pScoreNameBobs[i]);
		// Score count
		textBobSetText(&s_pScoreCountBobs[i], "%lu", s_pScores[i].lScore);
		textBobSetPos(
			&s_pScoreCountBobs[i],
			(32+320-64) - s_pScoreCountBobs[i].uwWidth, uwScorePos, 0, 0
		);
		textBobUpdate(&s_pScoreCountBobs[i]);
	}
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
		hiScoreUpdateScoreBobs();
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
	}
	ULONG ulDelta = timerGetDelta(s_ulCursorStart, timerGet());
	if(ulDelta >= SCORE_CURSOR_BLINK_TICKS) {
		s_ulCursorStart += SCORE_CURSOR_BLINK_TICKS;
		s_isCursor = !s_isCursor;
		isUpdateNeeded = 1;
	}

	if(isUpdateNeeded) {
		textBobSetText(
			&s_pScoreNameBobs[s_ubNewScorePos], "%hhu. %s%c",
			s_ubNewScorePos+1, s_pScores[s_ubNewScorePos].szName, s_isCursor ? '_' : ' '
		);
		textBobUpdate(&s_pScoreNameBobs[s_ubNewScorePos]);
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

void hiScoreSetup(LONG lScore) {
	s_isEnteringHiScore = 0;
	s_ubNewNameLength = 0;
	for(UBYTE i = 0; i < SCORE_COUNT; ++i) {
		if(s_pScores[i].lScore < lScore) {
			s_isEnteringHiScore = 1;
			s_isShift = 0;
			s_isCursor = 0;
			s_ulCursorStart = timerGet();
			s_ubNewScorePos = i;

			// Move worse score down
			for(BYTE j = SCORE_COUNT-2; j >= s_ubNewScorePos; --j) {
				strcpy(s_pScores[j+1].szName, s_pScores[j].szName);
				s_pScores[j+1].lScore = s_pScores[j].lScore;
			}
			// Make room for new score
			s_pScores[s_ubNewScorePos].lScore = lScore;
			memset(s_pScores[s_ubNewScorePos].szName, '\0', SCORE_NAME_LENGTH);
			break;
		}
	}
	hiScoreUpdateScoreBobs();
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
