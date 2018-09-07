/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/font.h>
#include <ace/utils/palette.h>

typedef enum _tHudDraw {
	HUD_DRAW_DEPTH,
	HUD_DRAW_END
} tHudDraw;

static tVPort *s_pVpHud;
static tSimpleBufferManager *s_pHudBuffer;
static tFont *s_pFont;
static tTextBitMap *s_pLinebuffer;

static UWORD s_uwDepth, s_uwOldDepth;

tHudDraw s_eDraw;

void hudCreate(tView *pView) {
  s_pVpHud = vPortCreate(0,
    TAG_VPORT_VIEW, pView,
    TAG_VPORT_BPP, 4,
    TAG_VPORT_HEIGHT, 32,
  TAG_END);

  s_pHudBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpHud,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
  TAG_END);

	paletteLoad("data/aminer.plt", s_pVpHud->pPalette, 16);

	s_pFont = fontCreate("data/silkscreen5.fnt");
	s_pLinebuffer = fontCreateTextBitMap(s_pHudBuffer->uBfrBounds.sUwCoord.uwX, 5);

	s_uwOldDepth = 0xFFFF;
	s_uwDepth = 0;
	s_eDraw = 0;
}

void hudSetDepth(UWORD uwDepth) {
	s_uwDepth = uwDepth;
}

void hudUpdate(void) {
	char szBfr[20];
	if(s_eDraw == HUD_DRAW_DEPTH) {
		if(s_uwDepth != s_uwOldDepth) {
			sprintf(szBfr, "Depth: %5u", s_uwDepth);
			fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
			fontDrawTextBitMap(s_pHudBuffer->pBack, s_pLinebuffer, 0, 0, 15, FONT_LAZY);
			s_uwOldDepth = s_uwDepth;
		}
	}
	++s_eDraw;
	if(s_eDraw >= HUD_DRAW_END) {
		s_eDraw = 0;
	}
}

void hudDestroy(void) {
	fontDestroyTextBitMap(s_pLinebuffer);
	fontDestroy(s_pFont);
}
