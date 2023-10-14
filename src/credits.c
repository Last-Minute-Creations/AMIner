/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "credits.h"
#include <ace/utils/string.h>
#include <comm/comm.h>
#include "aminer.h"
#include "core.h"
#include "assets.h"

static const UBYTE s_pE8Sequence[] = {
	0x0,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
	0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
};

static const char *s_pVerses[][8] = {
	// verse 1 pt1
	{
		"1 Comrade, ", "if ", "you ", "don't",
		"do ", "what ", "they ", "say"
	},
	{
		"2 I ", "said, ", "comrade, ", "if",
		"you ", "do ", "not ", "obey"
	},
	{
		"3 They ", "will ", "send ", "you",
		"to ", "Kamchatka ", "Resort", 0
	},
	{
		"4 Where ", "in ", "short ", "time",
		"you'll ", "be ", "no ", "more"
	},
	// verse 1 pt2
	{
		"5 Comrade, ", "if ", "you ", "don't",
		"do ", "what ", "they ", "say"
	},
	{
		"6 I ", "said, ", "comrade, ", "if",
		"you ", "do ", "not ", "obey"
	},
	{
		"7 They ", "will ", "send ", "you",
		"to ", "Kamchatka ", "Resort", 0
	},
	{
		"8 Where ", "in ", "short ", "time",
		"you'll ", "be ", "no ", "more"
	},

	// ref pt1
	{
		"9 It's ", "fun ", "to ", "live ",
		"in ", "the ", "Co-mu-nist ", "world"
	},
	{
		"A It's ", "fun ", "to ", "live ",
		"in ", "the ", "Co-mu-nist ", "world"
	},
	// ref pt2
	{
		"B Commissar ", "is ", "your ", "friend",
		0, 0, 0, 0
	},
	{
		"C Can ", "be ", "pain ", "on ",
		"rear ", "end", 0, 0
	},
	{
		"D Better ", "tell ", "him ", "all ",
		"that ", "you ", "know", 0
	},
};

static const char *s_pCreditsLines[] = {
	"Aminer by Last Minute Creations",
	"lastminutecreations.itch.io/aminer",
	"",
	"Softiron: graphics, game design",
	"Luc3k: sounds, music",
	"KaiN: code",
	"",
	"Special thanks to: Rav.En, JakubH",
	"and two little RKLE18 testers!"
};

static tUbCoordYX s_pWordPositions[ARRAY_SIZE(s_pVerses)][8];

static UBYTE s_ubSeq;
static UBYTE s_ubVerseCurr;
static UBYTE s_ubVersePrev;
static UBYTE s_ubWordCurr;
static UBYTE s_ubWordPrev;

static void creditsE8Handler(UBYTE ubE8) {
	switch(ubE8) {
		case 0x0:
			s_ubVerseCurr = 0xFF;
			break;
		case 0x1:
			++s_ubVerseCurr;
			s_ubWordCurr = 0xFF;
			s_ubWordPrev = 0xFF;
			break;
		case 0x2:
			++s_ubWordCurr;
			break;
		default:
			break;
	}
}

static void creditsGsCreate(void) {
	commEraseAll();
	UWORD uwOffsY = 0;
	UBYTE ubLineHeight = commGetLineHeight() - 2;
	for(UBYTE i = 0; i < ARRAY_SIZE(s_pCreditsLines); ++i) {
		commDrawText(0, uwOffsY, s_pCreditsLines[i], FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
		uwOffsY += ubLineHeight;
	}

	s_ubSeq = 0xFF;
	s_ubVerseCurr = 0xFF;
	s_ubVersePrev = 0xFF;
	s_ubWordCurr = 0;
	s_ubWordPrev = 0;

	for(UBYTE ubVerse = 0; ubVerse < ARRAY_SIZE(s_pVerses); ++ubVerse) {
		// First row
		UWORD uwKaraokeY = COMM_DISPLAY_HEIGHT - commGetLineHeight() * 2;
		UWORD uwLineLength = 0;
		for(UBYTE i = 0; i < 4; ++i) {
			if(stringIsEmpty(s_pVerses[ubVerse][i])) {
				break;
			}
			s_pWordPositions[ubVerse][i].ubX = uwLineLength;
			s_pWordPositions[ubVerse][i].ubY = uwKaraokeY;
			uwLineLength += fontMeasureText(g_pFont, s_pVerses[ubVerse][i]).uwX;
		}
		for(UBYTE i = 0; i < 4; ++i) {
			s_pWordPositions[ubVerse][i].ubX += (COMM_DISPLAY_WIDTH -  uwLineLength) / 2;
		}

		// Second row
		uwKaraokeY += commGetLineHeight();
		uwLineLength = 0;
		for(UBYTE i = 4; i < 8; ++i) {
			if(stringIsEmpty(s_pVerses[ubVerse][i])) {
				break;
			}
			s_pWordPositions[ubVerse][i].ubX = uwLineLength;
			s_pWordPositions[ubVerse][i].ubY = uwKaraokeY;
			uwLineLength += fontMeasureText(g_pFont, s_pVerses[ubVerse][i]).uwX;
		}
		for(UBYTE i = 4; i < 8; ++i) {
			s_pWordPositions[ubVerse][i].ubX += (COMM_DISPLAY_WIDTH -  uwLineLength) / 2;
		}
	}
}

static void creditsGsLoop(void) {
	commProcess();

	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		statePop(g_pGameStateManager);
	}

	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	UWORD uwKaraokeY = COMM_DISPLAY_HEIGHT - commGetLineHeight() * 2;
	if(s_ubVersePrev != s_ubVerseCurr) {
		s_ubVersePrev = s_ubVerseCurr;
		commErase(0, uwKaraokeY, COMM_DISPLAY_WIDTH, commGetLineHeight() * 2);
		if(s_ubVerseCurr == 0xFF) {
			return;
		}

		char szLineBuffer[100];
		char *pEnd = &szLineBuffer[0];
		szLineBuffer[0] = '\0';
		for(UBYTE i = 0; i < 4; ++i) {
			if(s_pVerses[s_ubVerseCurr][i]) {
				pEnd = stringCopy(s_pVerses[s_ubVerseCurr][i], pEnd);
			}
		}
		if(!stringIsEmpty(szLineBuffer)) {
			commDrawText(
				s_pWordPositions[s_ubVerseCurr][0].ubX,
				s_pWordPositions[s_ubVerseCurr][0].ubY, szLineBuffer,
				FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK
			);
		}

		pEnd = &szLineBuffer[0];
		szLineBuffer[0] = '\0';
		for(UBYTE i = 4; i < 8; ++i) {
			if(s_pVerses[s_ubVerseCurr][i]) {
				pEnd = stringCopy(s_pVerses[s_ubVerseCurr][i], pEnd);
			}
		}
		if(!stringIsEmpty(szLineBuffer)) {
			commDrawText(
				s_pWordPositions[s_ubVerseCurr][4].ubX,
				s_pWordPositions[s_ubVerseCurr][4].ubY, szLineBuffer,
				FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK
			);
		}
	}
	else if(s_ubWordPrev != s_ubWordCurr) {
		// Allow it catch up word by word if it lags
		++s_ubWordPrev;
		if(!stringIsEmpty(s_pVerses[s_ubVerseCurr][s_ubWordPrev])) {
			tUbCoordYX sPos = s_pWordPositions[s_ubVerseCurr][s_ubWordPrev];
			commDrawText(
				sPos.ubX, sPos.ubY, s_pVerses[s_ubVerseCurr][s_ubWordPrev],
				FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT
			);
		}
	}

	static UBYTE ubCnt = 0;
	if(++ubCnt >= 10) {
		ubCnt = 0;
		if(++s_ubSeq >= ARRAY_SIZE(s_pE8Sequence)) {
			s_ubSeq = 0;
		}
		creditsE8Handler(s_pE8Sequence[s_ubSeq]);
	}
}

static void creditsGsDestroy(void) {

}

void creditsReset(UBYTE isOnGameEnd) {

}

tState g_sStateMenuCredits = {
	.cbCreate = creditsGsCreate, .cbLoop = creditsGsLoop,
	.cbDestroy = creditsGsDestroy
};
