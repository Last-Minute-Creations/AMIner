/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "credits.h"
#include <ace/managers/ptplayer.h>
#include <ace/utils/string.h>
#include <comm/comm.h>
#include "aminer.h"
#include "core.h"
#include "assets.h"

#define LINES_PER_PAGE 2
#define STRINGS_PER_LINE 6

// static const UBYTE s_pE8Sequence[] = {
// 	0x0,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,

// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,

// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,

// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// 	0x1, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4,
// };

static const char *s_pVerses[][LINES_PER_PAGE][STRINGS_PER_LINE] = {
	// verse 1 pt1
	{
		{"1 Com", "rade, ", "if you ", "don't", 0, 0,},
		{"do ", "what ", "they ", "say", 0, 0}
	},
	{
		{"2 I ", "said, ", "com", "rade, ", "if", 0,},
		{"you ", "do ", "not ", "o", "bey", 0}
	},
	{
		{"3 They ", "will ", "send ", "you", 0, 0,},
		{"to ", "Kam", "chat", "ka ", "Re","sort"}
	},
	{
		{"4 Where ", "in ", "short ", "time", 0, 0,},
		{"you'll ", "be ", "no ", "more", 0, 0,}
	},
	// verse 1 pt2
	{
		{"5 Com", "rade, ", "if you ", "don't", 0, 0,},
		{"do ", "what ", "they ", "say", 0, 0}
	},
	{
		{"6 I ", "said, ", "com", "rade, ", "if", 0,},
		{"you ", "do ", "not ", "o", "bey", 0}
	},
	{
		{"7 They ", "will ", "send ", "you", 0, 0,},
		{"to ", "Kam", "chat", "ka ", "Re","sort"}
	},
	{
		{"8 Where ", "in ", "short ", "time", 0, 0,},
		{"you'll ", "be ", "no ", "more", 0, 0}
	},

	// ref pt1
	{
			{"9 It's ", "fun ", "to ", "live ", 0, 0,},
			{"in ", "the ", "Co", "mu", "nist ", "world"}
	},
	{
			{"A It's ", "fun ", "to ", "live ",  0, 0,},
			{"in ", "the ", "Co", "mu", "nist ", "world"}
	},
	// ref pt2
	{
			{"B Co", "mmi", "ssar ", "is ", "your ", "friend",},
			{0, 0, 0, 0, 0, 0}
	},
	{
			{"C Can ", "be ", "pain ", "on ", 0, 0,},
			{"rear ", "end", 0, 0, 0, 0}
	},
	{
			{"D Be", "tter ", "tell ", "him ", "all ", 0,},
			{"that ", "you ", "know", 0, 0, 0}
	},
	// ref pt1
	{
			{"E It's ", "fun ", "to ", "live ", 0, 0,},
			{"in ", "the ", "Co", "mu", "nist ", "world"}
	},
	{
			{"F It's ", "fun ", "to ", "live ",  0, 0,},
			{"in ", "the ", "Co", "mu", "nist ", "world"}
	},
	// ref pt2
	{
			{"10 Co", "mmi", "ssar ", "is ", "your ", "friend",},
			{0, 0, 0, 0, 0, 0}
	},
	{
			{"11 Can ", "be ", "pain ", "on ", 0, 0,},
			{"rear ", "end", 0, 0, 0, 0}
	},
	{
			{"12 Be", "tter ", "tell ", "him ", "all ", 0,},
			{"that ", "you ", "know", 0, 0, 0}
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

static tUbCoordYX s_pWordPositions[ARRAY_SIZE(s_pVerses)][LINES_PER_PAGE][STRINGS_PER_LINE];

// static UBYTE s_ubSeq;
static UBYTE s_ubVerseCurr;
static UBYTE s_ubVersePrev;
static UBYTE s_ubWordSeqCurr;
static UBYTE s_ubWordSeqPrev;
static UBYTE s_ubPrevE8;
static UBYTE s_ubLineHighlight;
static UBYTE s_ubStringHighlight;

static void creditsOnE8(UBYTE ubE8) {
	switch(ubE8) {
		case 0x0:
			s_ubVerseCurr = 0xFF;
			s_ubWordSeqCurr = 0xFF;
			s_ubWordSeqPrev = 0xFF;
			break;
		case 0x1:
		case 0x2:
			if(ubE8 != s_ubPrevE8) {
				++s_ubVerseCurr;
				s_ubWordSeqCurr = 0xFF;
				s_ubWordSeqPrev = 0xFF;
			}
			break;
		case 0x3:
		case 0x4:
			if(ubE8 != s_ubPrevE8) {
				++s_ubWordSeqCurr;
			}
			break;
		case 0x8:
			// Dummy command to get around loops etc
			break;
		default:
			break;
	}
	s_ubPrevE8 = ubE8;
}

static void creditsGsCreate(void) {
	commEraseAll();
	UWORD uwOffsY = 0;
	UBYTE ubLineHeight = commGetLineHeight() - 2;
	for(UBYTE i = 0; i < ARRAY_SIZE(s_pCreditsLines); ++i) {
		commDrawText(0, uwOffsY, s_pCreditsLines[i], FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
		uwOffsY += ubLineHeight;
	}

	// s_ubSeq = 0xFF;
	s_ubVerseCurr = 0xFF;
	s_ubVersePrev = 0xFF;
	s_ubWordSeqCurr = 0xFF;
	s_ubWordSeqPrev = 0xFF;
	s_ubPrevE8 = 0xFF;
	s_ubLineHighlight = 0;
	s_ubStringHighlight = 0xFF;

	for(UBYTE ubVerse = 0; ubVerse < ARRAY_SIZE(s_pVerses); ++ubVerse) {
		// First row
		UWORD uwKaraokeY = COMM_DISPLAY_HEIGHT - commGetLineHeight() * 2;
		for(UBYTE ubLine = 0; ubLine < LINES_PER_PAGE; ++ubLine) {
			UWORD uwLineLength = 0;
			for(UBYTE i = 0; i < STRINGS_PER_LINE; ++i) {
				if(stringIsEmpty(s_pVerses[ubVerse][ubLine][i])) {
					break;
				}
				s_pWordPositions[ubVerse][ubLine][i].ubX = uwLineLength;
				s_pWordPositions[ubVerse][ubLine][i].ubY = uwKaraokeY;
				uwLineLength += fontMeasureText(g_pFont, s_pVerses[ubVerse][ubLine][i]).uwX;
			}
			for(UBYTE i = 0; i < STRINGS_PER_LINE; ++i) {
				s_pWordPositions[ubVerse][ubLine][i].ubX += (COMM_DISPLAY_WIDTH -  uwLineLength) / 2;
			}
			uwKaraokeY += commGetLineHeight();
		}
	}

	ptplayerLoadMod(g_pCreditsMod, g_pModSampleData, 0);
	ptplayerSetE8Callback(creditsOnE8);
	ptplayerEnableMusic(1);
	ptplayerConfigureSongRepeat(0, 0);
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
		for(UBYTE ubLine = 0; ubLine < LINES_PER_PAGE; ++ubLine) {
			char *pEnd = &szLineBuffer[0];
			szLineBuffer[0] = '\0';
			for(UBYTE i = 0; i < STRINGS_PER_LINE; ++i) {
				if(s_pVerses[s_ubVerseCurr][ubLine][i]) {
					pEnd = stringCopy(s_pVerses[s_ubVerseCurr][ubLine][i], pEnd);
				}
			}
			if(!stringIsEmpty(szLineBuffer)) {
				commDrawText(
					s_pWordPositions[s_ubVerseCurr][ubLine][0].ubX,
					s_pWordPositions[s_ubVerseCurr][ubLine][0].ubY, szLineBuffer,
					FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK
				);
			}
		}

		s_ubLineHighlight = 0;
		s_ubStringHighlight = 0xFF;
	}
	else if(s_ubWordSeqPrev != s_ubWordSeqCurr) {
		// Allow it catch up word by word if it lags
		++s_ubWordSeqPrev;

		++s_ubStringHighlight;
		if(stringIsEmpty(s_pVerses[s_ubVerseCurr][s_ubLineHighlight][s_ubStringHighlight])) {
			++s_ubLineHighlight;
			s_ubStringHighlight = 0;
		}

		if(s_ubLineHighlight >= LINES_PER_PAGE || s_ubStringHighlight >= STRINGS_PER_LINE) {
			return;
		}

		if(!stringIsEmpty(s_pVerses[s_ubVerseCurr][s_ubLineHighlight][s_ubStringHighlight])) {
			tUbCoordYX sPos = s_pWordPositions[s_ubVerseCurr][s_ubLineHighlight][s_ubStringHighlight];
			commDrawText(
				sPos.ubX, sPos.ubY, s_pVerses[s_ubVerseCurr][s_ubLineHighlight][s_ubStringHighlight],
				FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT
			);
		}
	}

	// static UBYTE ubCnt = 0;
	// if(++ubCnt >= 10) {
	// 	ubCnt = 0;
	// 	if(++s_ubSeq >= ARRAY_SIZE(s_pE8Sequence)) {
	// 		s_ubSeq = 0;
	// 	}
	// 	creditsOnE8(s_pE8Sequence[s_ubSeq]);
	// }
}

static void creditsGsDestroy(void) {
	ptplayerStop();
	ptplayerSetE8Callback(0);
}

void creditsReset(UBYTE isOnGameEnd) {

}

tState g_sStateMenuCredits = {
	.cbCreate = creditsGsCreate, .cbLoop = creditsGsLoop,
	.cbDestroy = creditsGsDestroy
};
