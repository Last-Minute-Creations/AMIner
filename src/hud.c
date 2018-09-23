/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/font.h>
#include <ace/utils/palette.h>

typedef enum _tHudDraw {
	HUD_PREPARE_DEPTH,
	HUD_DRAW_DEPTH,
	HUD_PREPARE_FUEL,
	HUD_DRAW_FUEL,
	HUD_PREPARE_HEALTH,
	HUD_DRAW_HEALTH,
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
    TAG_VPORT_HEIGHT, 16,
  TAG_END);

  s_pHudBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpHud,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
  TAG_END);

	paletteLoad("data/aminer.plt", s_pVpHud->pPalette, 16);

	s_pFont = fontCreate("data/silkscreen5.fnt");
	s_pLinebuffer = fontCreateTextBitMap(s_pHudBuffer->uBfrBounds.sUwCoord.uwX, 5);

	fontDrawStr(s_pHudBuffer->pBack, s_pFont, 80, 0, "Fuel:", 15, FONT_LAZY);
	fontDrawStr(s_pHudBuffer->pBack, s_pFont, 200, 0, "Hull:", 15, FONT_LAZY);
	blitLine(s_pHudBuffer->pBack, 0, 15, s_pVpHud->uwWidth, 15, 1, 0xF0F0, 0);

	s_uwOldDepth = 0xFFFF;
	s_uwDepth = 0;
	s_eDraw = 0;
}

void hudSetDepth(UWORD uwDepth) {
	s_uwDepth = uwDepth;
}

void hudUpdate(void) {
	char szBfr[20];
	static UBYTE isDepthPrepared = 0;
	switch(s_eDraw) {
		case HUD_PREPARE_DEPTH:{
			if(s_uwDepth != s_uwOldDepth) {
				sprintf(szBfr, "Depth: %5u", s_uwDepth);
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				s_uwOldDepth = s_uwDepth;
				isDepthPrepared = 1;
			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_DEPTH: {
			if(isDepthPrepared) {
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLinebuffer, 0, 0, 15, FONT_LAZY
				);
				isDepthPrepared = 0;
			}
		} break;
		case HUD_PREPARE_FUEL: {
			if(0) {

			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_FUEL: {

		} break;
		case HUD_PREPARE_HEALTH: {
			if(0) {

			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_HEALTH: {

		} break;
		default: {

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
