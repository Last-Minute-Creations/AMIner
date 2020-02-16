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

typedef void (*tCbLogo)(void);
typedef UBYTE (*tCbFadeOut)(void);

void lmcFadeIn(void);
void lmcWait(void);
UBYTE lmcFadeOut(void);
void langFadeIn(void);
void langWait(void);
UBYTE langFadeOut(void);

static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pBfr;
static UBYTE s_ubFrame = 0;
static UBYTE s_ubState;

static UWORD s_pPaletteRef[32];
static UBYTE s_ubFadeoutCnt;
static tTextBitMap *s_pLineBuffer;
static tCbLogo s_cbFadeIn = 0, s_cbWait = 0;
static tCbFadeOut s_cbFadeOut = 0;
static UBYTE s_isAnyPressed = 0;

void logoGsCreate(void) {
	logBlockBegin("logoGsCreate()");

	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_END);

	s_pVp = vPortCreate(0,
		TAG_VPORT_BPP, 5,
		TAG_VPORT_VIEW, s_pView,
	TAG_END);

	s_pBfr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pVp,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 0,
	TAG_END);

	g_pFont = fontCreate("data/uni54.fnt");
	s_pLineBuffer = fontCreateTextBitMap(320, g_pFont->uwHeight);

	s_ubFadeoutCnt = 0;
	s_ubState = STATE_FADE_IN;

	s_cbFadeIn = lmcFadeIn;
	s_cbFadeOut = lmcFadeOut;
	s_cbWait = lmcWait;

	logBlockEnd("logoGsCreate()");
	systemUnuse();
	viewLoad(s_pView);
}

void logoGsLoop(void) {
	s_isAnyPressed = (
		keyUse(KEY_RETURN) | keyUse(KEY_ESCAPE) | keyUse(KEY_SPACE) |
		joyUse(JOY1 + JOY_FIRE) | joyUse(JOY2 + JOY_FIRE)
	);
	if(s_ubState == STATE_FADE_IN) {
		if(s_ubFadeoutCnt >= 50) {
			s_ubState = STATE_WAIT;
			s_ubFrame = 0;
		}
		else {
			if(s_cbFadeIn) {
				s_cbFadeIn();
			}
			++s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, s_pVp->pPalette, 32, (15 * s_ubFadeoutCnt) / 50);
		}
	}
	else if(s_ubState == STATE_WAIT) {
		if(s_cbWait) {
			s_cbWait();
		}
	}
	else if(s_ubState == STATE_FADE_OUT) {
		if(s_ubFadeoutCnt == 0) {
			if(s_cbFadeOut && s_cbFadeOut()) {
				return;
			}
			else {
				s_ubState = STATE_FADE_IN;
			}
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


//-------------------------------------------------------------------------- LMC

#define LOGO_WIDTH 128
#define LOGO_HEIGHT 171

void lmcFadeIn(void) {
	if(s_ubFadeoutCnt == 0) {
		systemUse();
		paletteLoad("data/lmc.plt", s_pPaletteRef, 1 << s_pVp->ubBPP);
		tBitMap *pLogo = bitmapCreateFromFile("data/lmc.bm", 0);
		systemUnuse();

		blitCopy(
			pLogo, 0, 0, s_pBfr->pBack,
			(s_pVp->uwWidth - LOGO_WIDTH) / 2, (s_pVp->uwHeight - LOGO_HEIGHT) / 2,
			LOGO_WIDTH, LOGO_HEIGHT, MINTERM_COOKIE, 0xFF
		);

		systemUse();
		bitmapDestroy(pLogo);
		systemUnuse();
	}
	else if(s_isAnyPressed) {
		s_ubState = STATE_FADE_OUT;
	}
}

void lmcWait(void) {
	++s_ubFrame;
	if(s_ubFrame >= 100 || s_isAnyPressed) {
		s_ubState = STATE_FADE_OUT;
	}
}

UBYTE lmcFadeOut(void) {
	s_cbFadeIn = langFadeIn;
	s_cbFadeOut = langFadeOut;
	s_cbWait = langWait;
	return 0;
}

//------------------------------------------------------------------------- LANG

static const char *s_pLanguageNames[LANG_COUNT] = {
	[LANG_EN] = "Johnny English",
	[LANG_PL] = "Mietek Polisz",
};

static const char *s_pLanguagePrefixes[LANG_COUNT] = {
	[LANG_EN] = "en",
	[LANG_PL] = "pl",
};

#define FACE_WIDTH (72)
#define FACE_HEIGHT (60)
#define FACE_DIST (30)
#define FACE_OFFS_Y ((256 - FACE_HEIGHT) / 2)
#define FACE_OFFS_X ((320 - (2 * FACE_WIDTH + FACE_DIST)) / 2)
#define FACE_DELTA_X (FACE_WIDTH + FACE_DIST)

static tLang s_eLangCurr = 0;

static void drawLangNames(void) {
	for(tLang eLang = 0; eLang < LANG_COUNT; ++eLang) {
		fontFillTextBitMap(g_pFont, s_pLineBuffer, s_pLanguageNames[eLang]);
		fontDrawTextBitMap(
			s_pBfr->pBack, s_pLineBuffer,
			FACE_OFFS_X + eLang * FACE_DELTA_X + FACE_WIDTH / 2,
			FACE_OFFS_Y + FACE_HEIGHT + 2,
			eLang == s_eLangCurr ? 4 : 3, FONT_HCENTER
		);
	}
}

void langFadeIn(void) {
	if(s_ubFadeoutCnt == 0) {
		systemUse();
		paletteLoad("data/aminer.plt", s_pPaletteRef, 1 << s_pVp->ubBPP);
		tBitMap *pFaces = bitmapCreateFromFile("data/lang_select.bm", 0);
		systemUnuse();

		// Set first color to black
		s_pPaletteRef[0] = 0x0000;

		// Clear that lousy logo
		blitRect(
			s_pBfr->pBack,
			(s_pVp->uwWidth - LOGO_WIDTH) / 2, (s_pVp->uwHeight - LOGO_HEIGHT) / 2,
			LOGO_WIDTH, LOGO_HEIGHT, 0
		);

		for(tLang eLang = 0; eLang < LANG_COUNT; ++eLang) {
			blitCopy(
				pFaces, 0, eLang * FACE_HEIGHT, s_pBfr->pBack,
				FACE_OFFS_X + eLang * FACE_DELTA_X, FACE_OFFS_Y,
				FACE_WIDTH, FACE_HEIGHT, MINTERM_COOKIE, 0xFF
			);
		}

		systemUse();
		bitmapDestroy(pFaces);
		systemUnuse();

		s_eLangCurr = 0;
		fontFillTextBitMap(g_pFont, s_pLineBuffer, "Choose the j\x84zyk");
		fontDrawTextBitMap(
			s_pBfr->pBack, s_pLineBuffer,
			320 / 2, FACE_OFFS_Y - (g_pFont->uwHeight + 2),
			14, FONT_LAZY | FONT_HCENTER
		);

		drawLangNames();
	}
}

void langWait(void) {
	if(
		keyUse(KEY_LEFT) || keyUse(KEY_A) ||
		joyUse(JOY1_LEFT) || joyUse(JOY2_LEFT)
	) {
		if(s_eLangCurr == 0) {
			s_eLangCurr = LANG_COUNT - 1;
		}
		else {
			--s_eLangCurr;
		}
		drawLangNames();
	}
	else if(
		keyUse(KEY_RIGHT) || keyUse(KEY_D) ||
		joyUse(JOY1_RIGHT) || joyUse(JOY2_RIGHT)
	) {
		if(++s_eLangCurr == LANG_COUNT) {
			s_eLangCurr = 0;
		}
		drawLangNames();
	}
	if(s_isAnyPressed) {
		coreSetLangPrefix(s_pLanguagePrefixes[s_eLangCurr]);
		s_ubState = STATE_FADE_OUT;
	}
}

UBYTE langFadeOut(void) {
	gameChangeState(coreGsCreate, coreGsLoop, coreGsDestroy);
	return 1;
}
