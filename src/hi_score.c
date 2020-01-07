/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hi_score.h"
#include <ace/managers/key.h>
#include <ace/managers/system.h>
#include <ace/managers/timer.h>
#include <ace/utils/bitmap.h>
#include "core.h"
#include "comm.h"

#define SCORE_NAME_LENGTH 20
#define SCORE_COUNT 10
#define SCORE_CURSOR_BLINK_TICKS 25 // 25 ticks = 500ms

typedef struct _tHiScore {
	LONG lScore;
	char szName[SCORE_NAME_LENGTH];
} tHiScore;

tStringArray g_sHiScoreMessages;

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

static void hiScoreDrawPosition(UBYTE ubPos) {
	UWORD uwY = 5 + (ubPos * 10);

	// Clear BG
	commErase(0, uwY, COMM_DISPLAY_WIDTH, g_pFont->uwHeight + 1);

	// Score name
	char szBfr[SCORE_NAME_LENGTH];
	sprintf(szBfr, "%hhu. %s", ubPos + 1, s_pScores[ubPos].szName);
	commDrawText(
		16, uwY, szBfr, FONT_LAZY | FONT_COOKIE | FONT_SHADOW,
		COMM_DISPLAY_COLOR_TEXT
	);

	// Score count
	sprintf(szBfr, "%lu", s_pScores[ubPos].lScore);
	commDrawText(
		COMM_DISPLAY_WIDTH - 16, uwY, szBfr,
		FONT_LAZY | FONT_COOKIE | FONT_RIGHT | FONT_SHADOW, COMM_DISPLAY_COLOR_TEXT
	);
}

void hiScoreDrawAll(void) {
	for(UBYTE ubPos = 0; ubPos < SCORE_COUNT; ++ubPos) {
		hiScoreDrawPosition(ubPos);
	}

	// End text
	commErase(
		0, COMM_DISPLAY_HEIGHT - g_pFont->uwHeight,
		COMM_DISPLAY_WIDTH, g_pFont->uwHeight
	);
	const char *szMsg;
	if(hiScoreIsEntering()) {
		szMsg = g_sHiScoreMessages.pStrings[MSG_HI_SCORE_NEW];
	}
	else {
		szMsg = g_sHiScoreMessages.pStrings[MSG_HI_SCORE_PRESS];
	}
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, COMM_DISPLAY_HEIGHT, szMsg,
		FONT_LAZY | FONT_COOKIE | FONT_HCENTER | FONT_BOTTOM,
		COMM_DISPLAY_COLOR_TEXT
	);
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
		hiScoreDrawAll();
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
		hiScoreDrawPosition(s_ubNewScorePos);
	}
}

UBYTE hiScoreIsEntering(void) {
	return s_isEnteringHiScore;
}

void hiScoreSetup(LONG lScore, const char *szResult) {
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
}
