/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "credits.h"
#include <ace/utils/string.h>
#include <comm/comm.h>
#include "aminer.h"
#include "core.h"

static const UBYTE s_pE8Sequence[] = {
	0x81,
	0x82,
	0x83,
	0x84,
	0x85,
	0x86,
	0x87,
	0x88,
	0x89,
	0x8A,
	0x8B,
	0x8C,
	0x8D,
	0x80,
};

static const char *s_pLyricsLines[][8] = {
	// verse 1 pt1
	{
		"Comrade, ", "if ", "you ", "don't",
		"do ", "what ", "they ", "say"
	},
	{
		"I ", "said, ", "comrade, ", "if",
		"you ", "do ", "not ", "obey"
	},
	{
		"They ", "will ", "send ", "you",
		"to ", "Kamchatka ", "Resort", 0
	},
	{
		"Where ", "in ", "short ", "time",
		"you'll ", "be ", "no ", "more"
	},
	// verse 1 pt2
	{
		"Comrade, ", "if ", "you ", "don't",
		"do ", "what ", "they ", "say"
	},
	{
		"I ", "said, ", "comrade, ", "if",
		"you ", "do ", "not ", "obey"
	},
	{
		"They ", "will ", "send ", "you",
		"to ", "Kamchatka ", "Resort", 0
	},
	{
		"Where ", "in ", "short ", "time",
		"you'll ", "be ", "no ", "more"
	},

	// ref pt1
	{
		"It's ", "fun ", "to ", "live ",
		"in ", "the ", "Co-mu-nist ", "world"
	},
	{
		"It's ", "fun ", "to ", "live ",
		"in ", "the ", "Co-mu-nist ", "world"
	},
	// ref pt2
	{
		"Commissar ", "is ", "your ", "friend",
		0, 0, 0, 0
	},
	{
		"Can ", "be ", "pain ", "on ",
		"rear ", "end", 0, 0
	},
	{
		"Better ", "tell ", "him ", "all ",
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
#define CREDITS_LINE_COUNT (sizeof(s_pCreditsLines) / sizeof(s_pCreditsLines[0]))

static UBYTE s_ubE8;
static UBYTE s_ubPrevE8;
static UBYTE s_ubSeq;

static void creditsGsCreate(void) {
	commEraseAll();
	UWORD uwOffsY = 0;
	UBYTE ubLineHeight = commGetLineHeight() - 2;
	for(UBYTE i = 0; i < CREDITS_LINE_COUNT; ++i) {
		commDrawText(0, uwOffsY, s_pCreditsLines[i], FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT);
		uwOffsY += ubLineHeight;
	}

	s_ubE8 = 0;
	s_ubSeq = 0;
}

static void creditsGsLoop(void) {
	commProcess();

	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		statePop(g_pGameStateManager);
	}

	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	if(s_ubE8 && s_ubE8 != s_ubPrevE8) {
		UWORD uwKaraokeY = COMM_DISPLAY_HEIGHT - commGetLineHeight() * 2;
		commErase(0, uwKaraokeY, COMM_DISPLAY_WIDTH, commGetLineHeight() * 2);
		if(s_ubE8 != 0x80) {
			UBYTE ubLyricLine = s_ubE8 - 0x81;
			char szLineBuffer[100];

			char *pEnd = &szLineBuffer[0];
			szLineBuffer[0] = '\0';
			for(UBYTE i = 0; i < 4; ++i) {
				if(s_pLyricsLines[ubLyricLine][i]) {
					pEnd = stringCopy(s_pLyricsLines[ubLyricLine][i], pEnd);
				}
			}
			if(!stringIsEmpty(szLineBuffer)) {
				commDrawText(
					COMM_DISPLAY_WIDTH / 2, uwKaraokeY, szLineBuffer,
					FONT_COOKIE | FONT_HCENTER, COMM_DISPLAY_COLOR_TEXT_DARK
				);
			}

			uwKaraokeY += commGetLineHeight();
			pEnd = &szLineBuffer[0];
			szLineBuffer[0] = '\0';
			for(UBYTE i = 0; i < 4; ++i) {
				if(s_pLyricsLines[ubLyricLine][4 + i]) {
					pEnd = stringCopy(s_pLyricsLines[ubLyricLine][4 + i], pEnd);
				}
			}
			if(!stringIsEmpty(szLineBuffer)) {
				commDrawText(
					COMM_DISPLAY_WIDTH / 2, uwKaraokeY, szLineBuffer,
					FONT_COOKIE | FONT_HCENTER, COMM_DISPLAY_COLOR_TEXT_DARK
				);
			}
		}
		s_ubPrevE8 = s_ubE8;
	}

	static UBYTE ubCnt = 0;
	if(++ubCnt >= 40) {
		ubCnt = 0;
		if(++s_ubSeq >= 14) {
			s_ubSeq = 0;
		}
		s_ubE8 = s_pE8Sequence[s_ubSeq];
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
