/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logo.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/joy.h>
#include <ace/utils/palette.h>
#include "core.h"

#define STATE_FADE_IN 0
#define STATE_FADE_OUT 1
#define STATE_WAIT 2

typedef enum _tLang {
	LANG_EN,
	LANG_PL,
	LANG_COUNT
} tLang;

static const char *s_pLanguageNames[LANG_COUNT] = {
	[LANG_EN] = "English",
	[LANG_PL] = "Polish",
};

static const char *s_pLanguagePrefixes[LANG_COUNT] = {
	[LANG_EN] = "en",
	[LANG_PL] = "pl",
};

static tLang s_eLangCurr = 0;
static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pBfr;
static UBYTE s_ubFrame = 0;
static UBYTE s_ubState;

static UWORD s_pPaletteRef[32];
static UBYTE s_ubFadeoutCnt;
static tTextBitMap *s_pLineBuffer;

static void drawLanguages(void) {
	for(tLang eLang = 0; eLang < LANG_COUNT; ++eLang) {
		fontFillTextBitMap(g_pFont, s_pLineBuffer, s_pLanguageNames[eLang]);
		fontDrawTextBitMap(
			s_pBfr->pBack, s_pLineBuffer,
			160, 9 + 171 + 10 + g_pFont->uwHeight * eLang,
			eLang == s_eLangCurr ? 4 : 3, FONT_HCENTER
		);
	}
}

void logoGsCreate(void) {
	logBlockBegin("logoGsCreate()");

	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_END);

	s_pVp = vPortCreate(0,
		TAG_VPORT_BPP, 3,
		TAG_VPORT_VIEW, s_pView,
	TAG_END);

	s_pBfr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pVp,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 0,
	TAG_END);

	paletteLoad("data/lmc.plt", s_pPaletteRef, 1 << s_pVp->ubBPP);

	g_pFont = fontCreate("data/uni54.fnt");
	s_pLineBuffer = fontCreateTextBitMap(320, g_pFont->uwHeight);

	bitmapLoadFromFile(
		s_pBfr->pBack, "data/lmc.bm", (s_pVp->uwWidth - 128) / 2, 9
	);

	s_ubFadeoutCnt = 0;
	s_ubState = STATE_FADE_IN;

	logBlockEnd("logoGsCreate()");
	systemUnuse();
	viewLoad(s_pView);
}

void logoGsLoop(void) {
	UBYTE isAnyPressed = (
		keyUse(KEY_RETURN) | keyUse(KEY_ESCAPE) | keyUse(KEY_SPACE) |
		joyUse(JOY1 + JOY_FIRE) | joyUse(JOY2 + JOY_FIRE)
	);
	if(s_ubState == STATE_FADE_IN) {
		if(s_ubFadeoutCnt >= 50) {
			s_ubState = STATE_WAIT;
			s_ubFrame = 0;
			s_eLangCurr = 0;
			drawLanguages();
		}
		else {
			++s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, s_pVp->pPalette, 32, (15 * s_ubFadeoutCnt) / 50);
		}
	}
	else if(s_ubState == STATE_WAIT) {
		// ++s_ubFrame;
		if(keyUse(KEY_UP) || keyUse(KEY_W) || joyUse(JOY1_UP) || joyUse(JOY2_UP)) {
			if(s_eLangCurr == 0) {
				s_eLangCurr = LANG_COUNT - 1;
			}
			else {
				--s_eLangCurr;
			}
			drawLanguages();
		}
		else if(keyUse(KEY_DOWN) || keyUse(KEY_S) || joyUse(JOY1_DOWN) || joyUse(JOY2_DOWN)) {
			if(++s_eLangCurr == LANG_COUNT) {
				s_eLangCurr = 0;
			}
			drawLanguages();
		}
		if(isAnyPressed) {
			coreSetLangPrefix(s_pLanguagePrefixes[s_eLangCurr]);
			s_ubState = STATE_FADE_OUT;
		}
	}
	else if(s_ubState == STATE_FADE_OUT) {
		if(s_ubFadeoutCnt == 0) {
			gameChangeState(coreGsCreate, coreGsLoop, coreGsDestroy);
			return;
		}
		else {
			--s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, s_pVp->pPalette, 32, 15*s_ubFadeoutCnt/50);
		}
	}

	vPortWaitForEnd(s_pVp);
	viewUpdateCLUT(s_pView);
}

void logoGsDestroy(void) {
	systemUse();
	logBlockBegin("logoGsDestroy()");
	viewDestroy(s_pView);
	fontDestroyTextBitMap(s_pLineBuffer);
	logBlockEnd("logoGsDestroy()");
}
