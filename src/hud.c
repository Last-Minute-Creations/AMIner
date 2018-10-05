/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/chunky.h>

#define COLOR_ACTIVE 15
#define COLOR_NACTIVE 4

#define FUEL_LABEL_X 58
#define HULL_LABEL_X 57
#define CARGO_LABEL_X 101

#define FUEL_GAUGE_X 78
#define HULL_GAUGE_X 78
#define CARGO_GAUGE_X 129

#define ROW_1_Y 0
#define ROW_2_Y 8

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
static const tFont *s_pFont;
static tTextBitMap *s_pLinebuffer;

typedef struct _tHudPlayerData {
	UWORD uwDepth, uwOldDepth;
	UWORD uwFuel, uwOldFuel;
	ULONG ulCash, ulOldCash;
	UBYTE ubCargo, ubOldCargo;
} tHudPlayerData;

static tHudPlayerData s_pPlayerData[2];
static tHudDraw s_eDraw;
static tHudPlayer s_ePlayer;
static UWORD s_uwHudOffsX;

void hudCreate(tView *pView, const tFont *pFont) {
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

	s_pFont = pFont;
	s_pLinebuffer = fontCreateTextBitMap(s_pHudBuffer->uBfrBounds.sUwCoord.uwX, 5);

	for(UBYTE ubPlayer = PLAYER_1; ubPlayer <= PLAYER_2; ++ubPlayer) {
		fontDrawStr(
			s_pHudBuffer->pBack, s_pFont, s_uwHudOffsX + FUEL_LABEL_X, 0,
			"Fuel:", COLOR_ACTIVE, FONT_LAZY
		);
		fontDrawStr(
			s_pHudBuffer->pBack, s_pFont, s_uwHudOffsX + HULL_LABEL_X, 8,
			"Hull:", COLOR_ACTIVE, FONT_LAZY
		);
		fontDrawStr(
			s_pHudBuffer->pBack, s_pFont, s_uwHudOffsX + CARGO_LABEL_X, 0,
			"Cargo:", COLOR_ACTIVE, FONT_LAZY
		);

		// Fuel inactive gauge
		for(UBYTE i = 0; i < 30; ++i) {
			chunkyToPlanar(
				COLOR_NACTIVE,
				s_uwHudOffsX + FUEL_GAUGE_X + 2 * (i % 10), ROW_1_Y + 2 * (i / 10),
				s_pHudBuffer->pBack
			);
		}
		// Hull
		for(UBYTE i = 0; i < 30; ++i) {
			chunkyToPlanar(
				COLOR_NACTIVE,
				s_uwHudOffsX + HULL_GAUGE_X + 2 * (i % 10), ROW_2_Y + 2 * (i / 10),
				s_pHudBuffer->pBack
			);
		}
		// Cargo
		for(UBYTE i = 0; i < 50; ++i) {
			blitRect(
				s_pHudBuffer->pBack,
				s_uwHudOffsX + CARGO_GAUGE_X + 3 * (i % 10), ROW_1_Y + 3 * (i / 10),
				2, 2, COLOR_NACTIVE
			);
		}

		s_uwHudOffsX = 160;
	}

	// Values to display - P1
	for(UBYTE ubPlayer = PLAYER_1; ubPlayer <= PLAYER_2; ++ubPlayer) {
		s_pPlayerData[ubPlayer].uwOldDepth = 0xFFFF;
		s_pPlayerData[ubPlayer].uwDepth = 0;
		s_pPlayerData[ubPlayer].ulOldCash = 0xFFFFFFFF;
		s_pPlayerData[ubPlayer].ulCash = 0;
		s_pPlayerData[ubPlayer].ubOldCargo = 0;
		s_pPlayerData[ubPlayer].ubCargo = 0;
		s_pPlayerData[ubPlayer].uwOldFuel = 0;
		s_pPlayerData[ubPlayer].uwFuel = 0;
	}

	// Restart state machine
	s_eDraw = 0;
	s_ePlayer = PLAYER_1;
	s_uwHudOffsX = 0;
}

void hudSetDepth(UBYTE ubPlayer, UWORD uwDepth) {
	s_pPlayerData[ubPlayer].uwDepth = uwDepth;
}

void hudSetScore(UBYTE ubPlayer, ULONG ulCash) {
	s_pPlayerData[ubPlayer].ulCash = ulCash;
}

void hudSetCargo(UBYTE ubPlayer, UBYTE ubCargo) {
	s_pPlayerData[ubPlayer].ubCargo = ubCargo;
}

void hudSetFuel(UBYTE ubPlayer, UWORD uwFuel) {
	s_pPlayerData[ubPlayer].uwFuel = (uwFuel+100-30) / 100;
}

void hudUpdate(void) {
	tHudPlayerData * const pData = &s_pPlayerData[s_ePlayer];
	char szBfr[20];
	switch(s_eDraw) {
		case HUD_PREPARE_DEPTH:{
			if(pData->uwDepth != pData->uwOldDepth) {
				sprintf(szBfr, "Depth: %5u", pData->uwDepth);
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				pData->uwOldDepth = pData->uwDepth;
			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_DEPTH: {
			fontDrawTextBitMap(
				s_pHudBuffer->pBack, s_pLinebuffer,
				s_uwHudOffsX, ROW_1_Y, COLOR_ACTIVE, FONT_LAZY | FONT_COOKIE
			);
		} break;
		case HUD_DRAW_FUEL: {
			if(pData->uwOldFuel != pData->uwFuel) {
				UBYTE ubColor, ubDraw;
				if(pData->uwFuel > pData->uwOldFuel) {
					ubDraw = pData->uwOldFuel;
					++pData->uwOldFuel;
					ubColor = COLOR_ACTIVE;
				}
				else {
					--pData->uwOldFuel;
					ubDraw = pData->uwOldFuel;
					ubColor = COLOR_NACTIVE;
				}
				chunkyToPlanar(
					ubColor, s_uwHudOffsX + FUEL_GAUGE_X + 2 * (ubDraw % 10),
					ROW_1_Y + 2 * (ubDraw / 10), s_pHudBuffer->pBack
				);
			}
		} break;
		case HUD_DRAW_HEALTH: {

		} break;
		case HUD_DRAW_CARGO: {
			if(pData->ubOldCargo != pData->ubCargo) {
				UBYTE ubColor, ubDraw;
				if(pData->ubCargo > pData->ubOldCargo) {
					ubDraw = pData->ubOldCargo;
					++pData->ubOldCargo;
					ubColor = COLOR_ACTIVE;
				}
				else {
					--pData->ubOldCargo;
					ubDraw = pData->ubOldCargo;
					ubColor = COLOR_NACTIVE;
				}
				blitRect(
					s_pHudBuffer->pBack, s_uwHudOffsX + CARGO_GAUGE_X + 3 * ((ubDraw) % 10),
					ROW_1_Y + 3 * ((ubDraw) / 10), 2, 2, ubColor
				);
			}
		} break;
		case HUD_PREPARE_CASH: {
			if(pData->ulCash != pData->ulOldCash) {
				sprintf(szBfr, "Cash: %5u", pData->ulCash);
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				pData->ulOldCash = pData->ulCash;
			}
			else {
				// Skip drawing
				++s_eDraw;
			}
		} break;
		case HUD_DRAW_CASH: {
			fontDrawTextBitMap(
				s_pHudBuffer->pBack, s_pLinebuffer,
				s_uwHudOffsX + 3, ROW_2_Y, COLOR_ACTIVE, FONT_LAZY | FONT_COOKIE
			);
		} break;
		default: {

		}
	}
	++s_eDraw;
	if(s_eDraw >= HUD_DRAW_END) {
		// Cycle players and start again
		if(s_ePlayer == PLAYER_1) {
			s_ePlayer = PLAYER_2;
			s_uwHudOffsX = 160;
		}
		else {
			s_ePlayer = PLAYER_1;
			s_uwHudOffsX = 0;
		}
		s_eDraw = 0;
	}
}

void hudDestroy(void) {
	fontDestroyTextBitMap(s_pLinebuffer);
}
