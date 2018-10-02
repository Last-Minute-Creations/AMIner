/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/font.h>
#include <ace/utils/palette.h>
#include <ace/utils/chunky.h>

#define COLOR_ACTIVE 15
#define COLOR_NACTIVE 4

typedef enum _tHudDraw {
	HUD_PREPARE_DEPTH,
	HUD_DRAW_DEPTH,
	HUD_DRAW_FUEL,
	HUD_DRAW_HEALTH,
	HUD_DRAW_CARGO,
	HUD_PREPARE_CASH,
	HUD_DRAW_CASH,
	HUD_DRAW_END
} tHudDraw;

static tVPort *s_pVpHud;
static tSimpleBufferManager *s_pHudBuffer;
static tFont *s_pFont;
static tTextBitMap *s_pLinebuffer;

static UWORD s_uwDepth, s_uwOldDepth;
static UWORD s_uwFuel, s_uwOldFuel;
static ULONG s_ulCash, s_ulOldScore;
static UBYTE s_ubCargo, s_ubOldCargo;

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

	fontDrawStr(s_pHudBuffer->pBack, s_pFont, 65, 0, "Fuel:", COLOR_ACTIVE, FONT_LAZY);
	fontDrawStr(s_pHudBuffer->pBack, s_pFont, 64, 8, "Hull:", COLOR_ACTIVE, FONT_LAZY);
	fontDrawStr(s_pHudBuffer->pBack, s_pFont, 110, 0, "Cargo:", COLOR_ACTIVE, FONT_LAZY);

	// Fuel inactive gauge
	for(UBYTE i = 0; i < 30; ++i) {
		chunkyToPlanar(COLOR_NACTIVE, 85 + 2 * (i % 10), 2 * (i / 10), s_pHudBuffer->pBack);
	}
	// Hull
	for(UBYTE i = 0; i < 30; ++i) {
		chunkyToPlanar(COLOR_NACTIVE, 85 + 2 * (i % 10), 8 + 2 * (i / 10), s_pHudBuffer->pBack);
	}
	// Cargo
	for(UBYTE i = 0; i < 50; ++i) {
		blitRect(s_pHudBuffer->pBack, 138 + 3 * (i % 10), 3 * (i / 10), 2, 2, COLOR_NACTIVE);
	}

	// Values to display
	s_uwOldDepth = 0xFFFF;
	s_uwDepth = 0;
	s_ulOldScore = 0xFFFFFFFF;
	s_ulCash = 0;
	s_ubOldCargo = 0;
	s_ubCargo = 0;
	s_uwOldFuel = 0;
	s_uwFuel = 0;
	// Restart state machine
	s_eDraw = 0;
}

void hudSetDepth(UWORD uwDepth) {
	s_uwDepth = uwDepth;
}

void hudSetScore(ULONG ulCash) {
	s_ulCash = ulCash;
}

void hudSetCargo(UBYTE ubCargo) {
	s_ubCargo = ubCargo;
}

void hudSetFuel(UWORD uwFuel) {
	s_uwFuel = (uwFuel+100-30) / 100;
}

void hudUpdate(void) {
	char szBfr[20];
	switch(s_eDraw) {
		case HUD_PREPARE_DEPTH:{
			if(s_uwDepth != s_uwOldDepth) {
				sprintf(szBfr, "Depth: %5u", s_uwDepth);
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				s_uwOldDepth = s_uwDepth;
			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_DEPTH: {
			fontDrawTextBitMap(
				s_pHudBuffer->pBack, s_pLinebuffer, 0, 0, COLOR_ACTIVE, FONT_LAZY
			);
		} break;
		case HUD_DRAW_FUEL: {
			if(s_uwOldFuel != s_uwFuel) {
				UBYTE ubColor, ubDraw;
				if(s_uwFuel > s_uwOldFuel) {
					ubDraw = s_uwOldFuel;
					++s_uwOldFuel;
					ubColor = COLOR_ACTIVE;
				}
				else {
					--s_uwOldFuel;
					ubDraw = s_uwOldFuel;
					ubColor = COLOR_NACTIVE;
				}

				chunkyToPlanar(
					ubColor,
					85 + 2 * (ubDraw % 10), 2 * (ubDraw / 10), s_pHudBuffer->pBack
				);
			}
		} break;
		case HUD_DRAW_HEALTH: {

		} break;
		case HUD_DRAW_CARGO: {
			if(s_ubOldCargo != s_ubCargo) {
				UBYTE ubColor, ubDraw;
				if(s_ubCargo > s_ubOldCargo) {
					ubDraw = s_ubOldCargo;
					++s_ubOldCargo;
					ubColor = COLOR_ACTIVE;
				}
				else {
					--s_ubOldCargo;
					ubDraw = s_ubOldCargo;
					ubColor = COLOR_NACTIVE;
				}
				blitRect(
					s_pHudBuffer->pBack,
					138 + 3 * ((ubDraw) % 10), 3 * ((ubDraw) / 10), 2, 2, ubColor
				);
			}
		} break;
		case HUD_PREPARE_CASH: {
			if(s_ulCash != s_ulOldScore) {
				sprintf(szBfr, "Cash: %5u", s_ulCash);
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				s_uwOldDepth = s_uwDepth;
			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_CASH: {
			fontDrawTextBitMap(
				s_pHudBuffer->pBack, s_pLinebuffer, 3, 8, COLOR_ACTIVE, FONT_LAZY
			);
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
