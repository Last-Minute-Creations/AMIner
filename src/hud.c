/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/chunky.h>

#define COLOR_ACTIVE 15
#define COLOR_NACTIVE 4

#define GAUGE_DRILL_X 85
#define GAUGE_CARGO_X 148
#define GAUGE_HULL_X 202
#define GAUGE_DEPTH_X 272
#define GAUGE_CASH_X 272

#define ROW_1_Y 0
#define ROW_2_Y 8

typedef enum _tHudDraw {
	HUD_PREPARE_DEPTH,
	HUD_DRAW_DEPTH,
	HUD_PREPARE_CASH,
	HUD_DRAW_CASH,
	HUD_DRAW_FUEL,
	HUD_DRAW_CARGO,
	HUD_DRAW_HEALTH,
	HUD_DRAW_END
} tHudDraw;

static tVPort *s_pVpHud;
static tSimpleBufferManager *s_pHudBuffer;
static const tFont *s_pFont;
static tTextBitMap *s_pLinebuffer;

typedef struct _tHudPlayerData {
	UWORD uwDepth, uwDepthDisp;
	ULONG ulCash, ulCashDisp;
	UWORD uwFuel, uwFuelDisp, uwFuelMax;
	UBYTE ubCargo, ubCargoDisp, ubCargoMax;
	UBYTE ubHealth, ubHealthDisp, ubHealthMax;
} tHudPlayerData;

static tHudPlayerData s_pPlayerData[2];
static tHudDraw s_eDraw;
static tHudPlayer s_ePlayer;
static UBYTE s_ubHudOffsY;
static UBYTE s_isChallenge;

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
	s_pLinebuffer = fontCreateTextBitMap(s_pHudBuffer->uBfrBounds.sUwCoord.uwX, pFont->uwHeight);

	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, 0, 0,
		"Player 1", COLOR_ACTIVE, FONT_LAZY
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, 0, s_pFont->uwHeight + 1,
		"Player 2", COLOR_ACTIVE, FONT_LAZY
	);

	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_DRILL_X - 1, 0,
		"Drill:", COLOR_ACTIVE, FONT_LAZY | FONT_RIGHT
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_CARGO_X - 1, 0,
		"Cargo:", COLOR_ACTIVE, FONT_LAZY | FONT_RIGHT
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_HULL_X - 1, 0,
		"Hull:", COLOR_ACTIVE, FONT_LAZY | FONT_RIGHT
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_CASH_X - 1, 0,
		"Cash:", COLOR_ACTIVE, FONT_LAZY | FONT_RIGHT
	);
	hudReset(0);
}

void hudReset(UBYTE isChallenge) {
	s_isChallenge = isChallenge;
	const UBYTE ubLabelWidth = fontMeasureText(s_pFont, "Depth:").sUwCoord.uwX;
	if(isChallenge) {
		// Clear depth label and use it as cash
		blitRect(
			s_pHudBuffer->pBack, GAUGE_DEPTH_X - 1 - ubLabelWidth, ROW_2_Y,
			ubLabelWidth, s_pFont->uwHeight, 0
		);
	}
	else {
		// Depth instead of 2p cash
		fontDrawStr(
			s_pHudBuffer->pBack, s_pFont, GAUGE_DEPTH_X - 1, s_pFont->uwHeight + 1,
			"Depth:", COLOR_ACTIVE, FONT_LAZY | FONT_RIGHT
		);
	}

	// Empty bars
	UWORD pBarOffsX[3] = {GAUGE_DRILL_X, GAUGE_CARGO_X, GAUGE_HULL_X};
	for(UBYTE i = 0; i < 3; ++i) {
		for(UBYTE b = 0; b < 10; ++b) {
			blitRect(
				s_pHudBuffer->pBack, pBarOffsX[i] + 3 * b, 0,
				2, 5, COLOR_NACTIVE
			);
			blitRect(
				s_pHudBuffer->pBack, pBarOffsX[i] + 3 * b, s_pFont->uwHeight + 1,
				2, 5, COLOR_NACTIVE
			);
		}
	}

	// Values to display
	for(UBYTE ubPlayer = PLAYER_1; ubPlayer <= PLAYER_2; ++ubPlayer) {
		s_pPlayerData[ubPlayer].uwDepthDisp = 0xFFFF;
		s_pPlayerData[ubPlayer].uwDepth = 0;
		s_pPlayerData[ubPlayer].ulCashDisp = 0xFFFFFFFF;
		s_pPlayerData[ubPlayer].ulCash = 0;
		s_pPlayerData[ubPlayer].ubCargoDisp = 0;
		s_pPlayerData[ubPlayer].ubCargo = 0;
		s_pPlayerData[ubPlayer].ubCargoMax = 0;
		s_pPlayerData[ubPlayer].uwFuelDisp = 0;
		s_pPlayerData[ubPlayer].uwFuel = 0;
		s_pPlayerData[ubPlayer].uwFuelMax = 0;
		s_pPlayerData[ubPlayer].ubHealthDisp = 0;
		s_pPlayerData[ubPlayer].ubHealth = 0;
		s_pPlayerData[ubPlayer].ubHealthMax = 0;
	}

	// Restart state machine
	s_eDraw = (isChallenge ? HUD_PREPARE_CASH : HUD_PREPARE_DEPTH);
	s_ePlayer = PLAYER_1;
	s_ubHudOffsY = 0;
}

void hudSetDepth(UBYTE ubPlayer, UWORD uwDepth) {
	s_pPlayerData[ubPlayer].uwDepth = uwDepth;
}

void hudSetScore(UBYTE ubPlayer, ULONG ulCash) {
	s_pPlayerData[ubPlayer].ulCash = ulCash;
}

void hudSetCargo(UBYTE ubPlayer, UBYTE ubCargo, UBYTE ubCargoMax) {
	s_pPlayerData[ubPlayer].ubCargo = ubCargo;
	s_pPlayerData[ubPlayer].ubCargoMax = ubCargoMax;
}

void hudSetFuel(UBYTE ubPlayer, UWORD uwFuel, UWORD uwFuelMax) {
	s_pPlayerData[ubPlayer].uwFuel = uwFuel;
	s_pPlayerData[ubPlayer].uwFuelMax = uwFuelMax;
}

void hudSetHealth(UBYTE ubPlayer, UBYTE ubHealth, UBYTE ubHealthMax) {
	s_pPlayerData[ubPlayer].ubHealth = ubHealth;
	s_pPlayerData[ubPlayer].ubHealthMax = ubHealthMax;
}

static void hudDrawBarPercent(
	UWORD uwOffsX, UBYTE ubOffsY, UBYTE ubPercent, UBYTE ubColor
) {
	UBYTE ubBarIdx = ubPercent / 10;
	UBYTE ubBarY = 4 - ((ubPercent % 10) + 1) / 2;
	UWORD uwX = uwOffsX + ubBarIdx * 3;
	chunkyToPlanar(ubColor, uwX + 0, ubOffsY + ubBarY, s_pHudBuffer->pBack);
	chunkyToPlanar(ubColor, uwX + 1, ubOffsY + ubBarY, s_pHudBuffer->pBack);
}

void hudUpdate(void) {
	tHudPlayerData * const pData = &s_pPlayerData[s_ePlayer];
	char szBfr[20];
	UBYTE ubPercent;
	UWORD uwDepth;
	ULONG ulCash;
	static UBYTE isDrawPending = 0;
	switch(s_eDraw) {
		case HUD_PREPARE_DEPTH:
			uwDepth = (s_pPlayerData[0].uwDepth + s_pPlayerData[1].uwDepth) / 2;
			if(uwDepth != pData->uwDepthDisp) {
				sprintf(szBfr, "%u.%um", uwDepth / 10, uwDepth % 10);
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				pData->uwDepthDisp = uwDepth;
				s_eDraw = HUD_DRAW_DEPTH;
				isDrawPending = 1;
				break;
			}
		case HUD_DRAW_DEPTH:
			if(isDrawPending) {
				blitRect(
					s_pHudBuffer->pBack, GAUGE_DEPTH_X, ROW_2_Y,
					320 - GAUGE_DEPTH_X, s_pFont->uwHeight, 0
				);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLinebuffer,
					GAUGE_DEPTH_X, ROW_2_Y, COLOR_ACTIVE, FONT_LAZY
				);
				s_eDraw = HUD_PREPARE_CASH;
				isDrawPending = 0;
				break;
			}
		case HUD_PREPARE_CASH:
			if(s_isChallenge) {
				ulCash = s_pPlayerData[s_ePlayer].ulCash;
			}
			else {
				ulCash = s_pPlayerData[0].ulCash + s_pPlayerData[1].ulCash;
			}
			if(ulCash != pData->ulCashDisp) {
				UWORD m = (ulCash / 1000U) / 1000U;
				UWORD k = (ulCash / 1000U) % 1000U;
				UWORD u = ulCash % 1000U;
				if(ulCash >= 1000000U) {
					sprintf(szBfr, "%lu.%03lu.%03lu", m, k, u);
				}
				else if(ulCash >= 1000U) {
					sprintf(szBfr, "%lu.%03lu", k, u);
				}
				else {
					sprintf(szBfr, "%lu", ulCash);
				}
				fontFillTextBitMap(s_pFont, s_pLinebuffer, szBfr);
				pData->ulCashDisp = ulCash;
				s_eDraw = HUD_DRAW_CASH;
				isDrawPending = 1;
				break;
			}
		case HUD_DRAW_CASH:
			if(isDrawPending) {
				UBYTE ubY = (s_isChallenge ? s_ubHudOffsY : ROW_1_Y);
				blitRect(
					s_pHudBuffer->pBack, GAUGE_CASH_X, ubY,
					320 - GAUGE_CASH_X, s_pFont->uwHeight, 0
				);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLinebuffer,
					GAUGE_CASH_X, ubY, COLOR_ACTIVE, FONT_LAZY
				);
				s_eDraw = HUD_DRAW_FUEL;
				isDrawPending = 0;
				break;
			}
		case HUD_DRAW_FUEL:
			ubPercent = 0;
			if(pData->uwFuelMax) {
				ubPercent = (100 * pData->uwFuel) / pData->uwFuelMax;
			}
			if(pData->uwFuelDisp != ubPercent) {
				UBYTE ubColor, ubDraw;
				if(ubPercent > pData->uwFuelDisp) {
					ubDraw = pData->uwFuelDisp;
					++pData->uwFuelDisp;
					ubColor = COLOR_ACTIVE;
				}
				else {
					--pData->uwFuelDisp;
					ubDraw = pData->uwFuelDisp;
					ubColor = COLOR_NACTIVE;
				}
				hudDrawBarPercent(GAUGE_DRILL_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eDraw = HUD_DRAW_CARGO;
				break;
			}
		case HUD_DRAW_CARGO:
			ubPercent = 0;
			if(pData->ubCargoMax) {
				ubPercent = (100 * pData->ubCargo) / pData->ubCargoMax;
			}
			if(pData->ubCargoDisp != ubPercent) {
				UBYTE ubColor, ubDraw;
				if(ubPercent > pData->ubCargoDisp) {
					ubDraw = pData->ubCargoDisp;
					++pData->ubCargoDisp;
					ubColor = COLOR_ACTIVE;
				}
				else {
					--pData->ubCargoDisp;
					ubDraw = pData->ubCargoDisp;
					ubColor = COLOR_NACTIVE;
				}
				hudDrawBarPercent(GAUGE_CARGO_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eDraw = HUD_DRAW_HEALTH;
				break;
			}
		case HUD_DRAW_HEALTH:
			// s_eDraw = HUD_DRAW_END; // do end immediately after this state
			ubPercent = 0;
			if(pData->ubHealthMax) {
				ubPercent = (100 * pData->ubHealth) / pData->ubHealthMax;
			}
			if(pData->ubHealthDisp != ubPercent) {

				// break; // do end immediately after this state
			}
		case HUD_DRAW_END:
		default:
			// Cycle players and start again
			if(s_ePlayer == PLAYER_1) {
				s_ePlayer = PLAYER_2;
				s_ubHudOffsY = s_pFont->uwHeight + 1;
				s_eDraw = (s_isChallenge ? HUD_PREPARE_CASH : HUD_DRAW_FUEL);
			}
			else {
				s_ePlayer = PLAYER_1;
				s_ubHudOffsY = 0;
				s_eDraw = (s_isChallenge ? HUD_PREPARE_CASH : HUD_PREPARE_DEPTH);
			}
	}
}

void hudDestroy(void) {
	fontDestroyTextBitMap(s_pLinebuffer);
}
