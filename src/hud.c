/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/chunky.h>

#define COLOR_BG 11
#define COLOR_BAR_FULL 14
#define COLOR_BAR_EMPTY 12

#define HUD_HEIGHT 31
#define HUD_ORIGIN_X 11
#define HUD_ORIGIN_Y 9
#define HUD_FACE_SIZE 16

#define GAUGE_DRILL_X (HUD_ORIGIN_X + 65)
#define GAUGE_CARGO_X (HUD_ORIGIN_X + 128)
#define GAUGE_HULL_X (HUD_ORIGIN_X + 182)
#define GAUGE_DEPTH_X (HUD_ORIGIN_X + 248)
#define GAUGE_CASH_X (HUD_ORIGIN_X + 248)

#define ROW_1_Y (HUD_ORIGIN_Y + 0)
#define ROW_2_Y (HUD_ORIGIN_Y + 9)
#define HUD_MSG_LEN_MAX 250
#define HUD_MSG_WAIT_CNT 150

typedef enum _tHudState {
	// State machine for drawing main HUD
	STATE_MAIN_PREPARE_DEPTH,
	STATE_MAIN_DRAW_DEPTH,
	STATE_MAIN_PREPARE_CASH,
	STATE_MAIN_DRAW_CASH,
	STATE_MAIN_DRAW_FUEL,
	STATE_MAIN_DRAW_CARGO,
	STATE_MAIN_DRAW_HULL,
	STATE_MAIN_DRAW_END,
	// State machine for drawing message
	STATE_MSG_NOISE_IN,
	STATE_MSG_DRAW_FACE,
	STATE_MSG_PREPARE_LETTER,
	STATE_MSG_DRAW_LETTER,
	STATE_MSG_WAIT_OUT,
	STATE_MSG_NOISE_OUT,
	STATE_MSG_END,
	// State machine for pause screen
	STATE_PAUSE_INITIAL_DRAW,
	STATE_PAUSE_LOOP,
	// State machine for drawing aux HUD
} tHudState;

typedef struct _tHudPlayerData {
	UWORD uwDepth, uwDepthDisp;
	LONG lCash, lCashDisp;
	UWORD uwDrill, uwDrillMax;
	UBYTE ubCargo, ubCargoMax;
	UWORD uwHull, uwHullMax;
	UBYTE ubHullDisp, ubCargoDisp, ubDrillDisp;
} tHudPlayerData;

static tVPort *s_pVpHud;
static tSimpleBufferManager *s_pHudBuffer;
static const tFont *s_pFont;
static tTextBitMap *s_pLineBuffer;
static tHudPlayerData s_pPlayerData[2];
static tHudState s_eState;
static tHudPlayer s_ePlayer;
static UBYTE s_ubHudOffsY;
static UBYTE s_isChallenge, s_is2pPlaying;
static UWORD s_uwFrameDelay, s_uwStateCounter;
static UWORD s_uwMsgLen;
static tUwCoordYX s_sMsgCharPos;
static UBYTE s_ubMsgCharIdx;
static char s_szMsg[HUD_MSG_LEN_MAX];
static char s_szLetter[2] = {'\0'};
static tBitMap *s_pFaces;
static UBYTE s_ubFaceToDraw;
static UBYTE s_ubSelection, s_ubSelectionPrev;

void hudCreate(tView *pView, const tFont *pFont) {
  s_pVpHud = vPortCreate(0,
    TAG_VPORT_VIEW, pView,
    TAG_VPORT_BPP, 5,
    TAG_VPORT_HEIGHT, 31,
  TAG_END);

  s_pHudBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpHud,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, 31 * 5,
  TAG_END);

	bitmapLoadFromFile(s_pHudBuffer->pBack, "data/hud.bm", 0, 0);
	s_pFaces = bitmapCreateFromFile("data/comm_faces.bm", 0);

	s_pFont = pFont;
	s_pLineBuffer = fontCreateTextBitMap(s_pHudBuffer->uBfrBounds.uwX, pFont->uwHeight);

	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, HUD_ORIGIN_X, ROW_1_Y,
		"Player 1", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, HUD_ORIGIN_X, ROW_2_Y,
		"Player 2", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE
	);

	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_DRILL_X - 1, ROW_1_Y,
		"Drill:", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE | FONT_RIGHT
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_CARGO_X - 1, ROW_1_Y,
		"Cargo:", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE | FONT_RIGHT
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_HULL_X - 1, ROW_1_Y,
		"Hull:", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE | FONT_RIGHT
	);
	fontDrawStr(
		s_pHudBuffer->pBack, s_pFont, GAUGE_CASH_X - 1, ROW_1_Y,
		"Cash:", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE | FONT_RIGHT
	);
	hudReset(0, 0);
}

static void hudShowPage(UBYTE ubPage) {
	cameraSetCoord(s_pHudBuffer->pCamera, 0, ubPage * 31);
}

void hudShowMessage(UBYTE ubFace, const char *szMsg) {
	logWrite("Showing HUD message: '%s'\n", szMsg);
	CopyMem(szMsg, s_szMsg, MIN(strlen(szMsg) + 1, HUD_MSG_LEN_MAX));
	s_eState = STATE_MSG_NOISE_IN;
	s_uwFrameDelay = 25;
	s_uwStateCounter = 0;
	s_ubFaceToDraw = ubFace;
}

void hudSet2pPlaying(UBYTE isPlaying) {
	s_is2pPlaying = isPlaying;
}

static void hudResetStateMachine(void) {
	s_eState = (
		s_isChallenge ? STATE_MAIN_PREPARE_CASH : STATE_MAIN_PREPARE_DEPTH
	);
	s_ePlayer = PLAYER_1;
	s_ubHudOffsY = ROW_1_Y;
}

void hudReset(UBYTE isChallenge, UBYTE is2pPlaying) {
	s_isChallenge = isChallenge;
	s_is2pPlaying = is2pPlaying;
	const UBYTE ubLabelWidth = fontMeasureText(s_pFont, "Depth:").uwX;
	if(isChallenge) {
		// Clear depth label and use it as cash
		blitRect(
			s_pHudBuffer->pBack, GAUGE_DEPTH_X - 1 - ubLabelWidth, ROW_2_Y,
			ubLabelWidth, s_pFont->uwHeight, COLOR_BG
		);
	}
	else {
		// Depth instead of 2p cash
		fontDrawStr(
			s_pHudBuffer->pBack, s_pFont, GAUGE_DEPTH_X - 1, ROW_2_Y,
			"Depth:", COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE | FONT_RIGHT
		);
	}

	// Empty bars
	UWORD pBarOffsX[3] = {GAUGE_DRILL_X, GAUGE_CARGO_X, GAUGE_HULL_X};
	for(UBYTE i = 0; i < 3; ++i) {
		for(UBYTE b = 0; b < 10; ++b) {
			blitRect(
				s_pHudBuffer->pBack, pBarOffsX[i] + 3 * b, ROW_1_Y,
				2, 5, COLOR_BAR_EMPTY
			);
			blitRect(
				s_pHudBuffer->pBack, pBarOffsX[i] + 3 * b, ROW_2_Y,
				2, 5, COLOR_BAR_EMPTY
			);
		}
	}

	// Values to display
	for(UBYTE ubPlayer = PLAYER_1; ubPlayer <= PLAYER_2; ++ubPlayer) {
		s_pPlayerData[ubPlayer].uwDepthDisp = 0xFFFF;
		s_pPlayerData[ubPlayer].uwDepth = 0;
		s_pPlayerData[ubPlayer].lCashDisp = 0xFFFFFFFF;
		s_pPlayerData[ubPlayer].lCash = 0;
		s_pPlayerData[ubPlayer].ubCargoDisp = 0;
		s_pPlayerData[ubPlayer].ubCargo = 0;
		s_pPlayerData[ubPlayer].ubCargoMax = 0;
		s_pPlayerData[ubPlayer].ubDrillDisp = 0;
		s_pPlayerData[ubPlayer].uwDrill = 0;
		s_pPlayerData[ubPlayer].uwDrillMax = 0;
		s_pPlayerData[ubPlayer].ubHullDisp = 0;
		s_pPlayerData[ubPlayer].uwHull = 0;
		s_pPlayerData[ubPlayer].uwHullMax = 0;
	}

	hudResetStateMachine();
}

void hudSetDepth(UBYTE ubPlayer, UWORD uwDepth) {
	s_pPlayerData[ubPlayer].uwDepth = uwDepth;
}

void hudSetCash(UBYTE ubPlayer, LONG lCash) {
	s_pPlayerData[ubPlayer].lCash = lCash;
}

void hudSetCargo(UBYTE ubPlayer, UBYTE ubCargo, UBYTE ubCargoMax) {
	s_pPlayerData[ubPlayer].ubCargo = ubCargo;
	s_pPlayerData[ubPlayer].ubCargoMax = ubCargoMax;
}

void hudSetDrill(UBYTE ubPlayer, UWORD uwDrill, UWORD uwDrillMax) {
	s_pPlayerData[ubPlayer].uwDrill = uwDrill;
	s_pPlayerData[ubPlayer].uwDrillMax = uwDrillMax;
}

void hudSetHull(UBYTE ubPlayer, UWORD uwHull, UWORD uwHullMax) {
	s_pPlayerData[ubPlayer].uwHull = uwHull;
	s_pPlayerData[ubPlayer].uwHullMax = uwHullMax;
}

static void hudDrawStateBarPercent(
	UWORD uwOffsX, UBYTE ubOffsY, UBYTE ubPercent, UBYTE ubColor
) {
	// This draws only 0..99! 0: only 1px of first bar, 99: full
	UBYTE ubBarIdx = ubPercent / 10;
	UBYTE ubBarY = 4 - (UBYTE)((ubPercent % 10) * 4) / 9;
	UWORD uwX = uwOffsX + ubBarIdx * 3;
	chunkyToPlanar(ubColor, uwX + 0, ubOffsY + ubBarY, s_pHudBuffer->pBack);
	chunkyToPlanar(ubColor, uwX + 1, ubOffsY + ubBarY, s_pHudBuffer->pBack);
}

void hudUpdate(void) {
	tHudPlayerData * const pData = &s_pPlayerData[s_ePlayer];
	char szBfr[20];
	UBYTE ubPercent;
	UWORD uwDepth;
	LONG lCash;
	static UBYTE isDrawPending = 0;
	static UBYTE isLineOverflow = 0;
	switch(s_eState) {
		case STATE_MAIN_PREPARE_DEPTH:
			if(s_is2pPlaying) {
				uwDepth = (s_pPlayerData[0].uwDepth + s_pPlayerData[1].uwDepth) / 2;
			}
			else {
				uwDepth = s_pPlayerData[0].uwDepth;
			}
			if(uwDepth != pData->uwDepthDisp) {
				sprintf(szBfr, "%u.%um", uwDepth / 10, uwDepth % 10);
				fontFillTextBitMap(s_pFont, s_pLineBuffer, szBfr);
				pData->uwDepthDisp = uwDepth;
				s_eState = STATE_MAIN_DRAW_DEPTH;
				isDrawPending = 1;
				break;
			}
		case STATE_MAIN_DRAW_DEPTH:
			if(isDrawPending) {
				// decreased clear height 'cuz digits are smaller than whole font
				blitRect(
					s_pHudBuffer->pBack, GAUGE_DEPTH_X, ROW_2_Y,
					320 - (GAUGE_DEPTH_X + HUD_ORIGIN_X), s_pFont->uwHeight - 2, COLOR_BG
				);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					GAUGE_DEPTH_X, ROW_2_Y, COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE
				);
				s_eState = STATE_MAIN_PREPARE_CASH;
				isDrawPending = 0;
				break;
			}
		case STATE_MAIN_PREPARE_CASH:
			if(s_isChallenge) {
				lCash = s_pPlayerData[s_ePlayer].lCash;
			}
			else {
				lCash = s_pPlayerData[0].lCash + s_pPlayerData[1].lCash;
			}
			if(lCash != pData->lCashDisp) {
				ULONG ulDisp;
				UBYTE ubOffs = 0;
				if(lCash < 0) {
					ulDisp = -lCash;
					szBfr[0] = '-';
					ubOffs = 1;
				}
				else {
					ulDisp = lCash;
				}
				UWORD m = (ulDisp / 1000U) / 1000U;
				UWORD k = (ulDisp / 1000U) % 1000U;
				UWORD u = ulDisp % 1000U;
				if(ulDisp >= 1000000U) {
					sprintf(&szBfr[ubOffs], "%lu.%03lu.%03lu\x1F", m, k, u);
				}
				else if(ulDisp >= 1000U) {
					sprintf(&szBfr[ubOffs], "%lu.%03lu\x1F", k, u);
				}
				else {
					sprintf(&szBfr[ubOffs], "%lu\x1F", u);
				}
				fontFillTextBitMap(s_pFont, s_pLineBuffer, szBfr);
				pData->lCashDisp = lCash;
				s_eState = STATE_MAIN_DRAW_CASH;
				isDrawPending = 1;
				break;
			}
		case STATE_MAIN_DRAW_CASH:
			if(isDrawPending) {
				// decreased clear height 'cuz digits are smaller than whole font
				UBYTE ubY = (s_isChallenge ? s_ubHudOffsY : ROW_1_Y);
				blitRect(
					s_pHudBuffer->pBack, GAUGE_CASH_X, ubY,
					320 - (GAUGE_CASH_X + HUD_ORIGIN_X), s_pFont->uwHeight - 1, COLOR_BG
				);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					GAUGE_CASH_X, ubY, COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE
				);
				s_eState = STATE_MAIN_DRAW_FUEL;
				isDrawPending = 0;
				break;
			}
		case STATE_MAIN_DRAW_FUEL:
			ubPercent = 0;
			if(pData->uwDrillMax) {
				ubPercent = (100 * pData->uwDrill) / pData->uwDrillMax;
			}
			if(pData->ubDrillDisp != ubPercent) {
				UBYTE ubColor, ubDraw;
				if(ubPercent > pData->ubDrillDisp) {
					ubDraw = pData->ubDrillDisp;
					++pData->ubDrillDisp;
					ubColor = COLOR_BAR_FULL;
				}
				else {
					--pData->ubDrillDisp;
					ubDraw = pData->ubDrillDisp;
					ubColor = COLOR_BAR_EMPTY;
				}
				hudDrawStateBarPercent(GAUGE_DRILL_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eState = STATE_MAIN_DRAW_CARGO;
				break;
			}
		case STATE_MAIN_DRAW_CARGO:
			ubPercent = 0;
			if(pData->ubCargoMax) {
				ubPercent = (100 * pData->ubCargo) / pData->ubCargoMax;
			}
			if(pData->ubCargoDisp != ubPercent) {
				UBYTE ubColor, ubDraw;
				if(ubPercent > pData->ubCargoDisp) {
					ubDraw = pData->ubCargoDisp;
					++pData->ubCargoDisp;
					ubColor = COLOR_BAR_FULL;
				}
				else {
					--pData->ubCargoDisp;
					ubDraw = pData->ubCargoDisp;
					ubColor = COLOR_BAR_EMPTY;
				}
				hudDrawStateBarPercent(GAUGE_CARGO_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eState = STATE_MAIN_DRAW_HULL;
				break;
			}
		case STATE_MAIN_DRAW_HULL:
			ubPercent = 0;
			if(pData->uwHullMax) {
				ubPercent = (100 * pData->uwHull) / pData->uwHullMax;
			}
			if(pData->ubHullDisp != ubPercent) {
				UBYTE ubColor, ubDraw;
				if(ubPercent > pData->ubHullDisp) {
					ubDraw = pData->ubHullDisp;
					++pData->ubHullDisp;
					ubColor = COLOR_BAR_FULL;
				}
				else {
					--pData->ubHullDisp;
					ubDraw = pData->ubHullDisp;
					ubColor = COLOR_BAR_EMPTY;
				}
				hudDrawStateBarPercent(GAUGE_HULL_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eState = STATE_MAIN_DRAW_END;
				break;
			}
		case STATE_MAIN_DRAW_END:
			// Cycle players and start again
			if(s_ePlayer == PLAYER_1) {
				s_ePlayer = PLAYER_2;
				s_ubHudOffsY = ROW_2_Y;
				s_eState = (s_isChallenge ? STATE_MAIN_PREPARE_CASH : STATE_MAIN_DRAW_FUEL);
			}
			else {
				s_ePlayer = PLAYER_1;
				s_ubHudOffsY = ROW_1_Y;
				s_eState = (s_isChallenge ? STATE_MAIN_PREPARE_CASH : STATE_MAIN_PREPARE_DEPTH);
			}
			break;
		case STATE_MSG_NOISE_IN:
			if(--s_uwFrameDelay == 0) {
				s_eState = STATE_MSG_DRAW_FACE;
			}
			else if(++s_uwStateCounter >= 4) {
				hudShowPage(1 + (s_uwFrameDelay % 3));
				s_uwStateCounter = 0;
			}
			break;
		case STATE_MSG_DRAW_FACE:
			isLineOverflow = 0;
			s_sMsgCharPos.uwX = HUD_ORIGIN_X;
			s_sMsgCharPos.uwY = 4 * HUD_HEIGHT + HUD_ORIGIN_Y;

			blitCopy(
				s_pFaces, 0, s_ubFaceToDraw * HUD_FACE_SIZE,
				s_pHudBuffer->pBack, s_sMsgCharPos.uwX, s_sMsgCharPos.uwY,
				HUD_FACE_SIZE, HUD_FACE_SIZE, MINTERM_COOKIE, 0xFF
			);
			s_sMsgCharPos.uwX += HUD_FACE_SIZE + 1;
			blitRect(
				s_pHudBuffer->pBack, s_sMsgCharPos.uwX, s_sMsgCharPos.uwY,
				283, 2 * s_pFont->uwHeight + 1, COLOR_BG
			);

			hudShowPage(4);
			s_ubMsgCharIdx = 0;
			s_uwMsgLen = strlen(s_szMsg);
			s_eState = STATE_MSG_PREPARE_LETTER;
			break;
		case STATE_MSG_PREPARE_LETTER:
			s_szLetter[0] = s_szMsg[s_ubMsgCharIdx];
			if(s_szMsg[s_ubMsgCharIdx] != '\n') {
				fontFillTextBitMap(s_pFont, s_pLineBuffer, s_szLetter);
			}
			s_eState = STATE_MSG_DRAW_LETTER;
			break;
		case STATE_MSG_DRAW_LETTER:
			if(s_szMsg[s_ubMsgCharIdx] == '\n') {
				s_sMsgCharPos.uwX = HUD_ORIGIN_X + HUD_FACE_SIZE + 1;
				s_sMsgCharPos.uwY += s_pFont->uwHeight + 1;
				isLineOverflow = 0;
			}
			else {
				UBYTE ubGlyphWidth = fontGlyphWidth(s_pFont, s_szMsg[s_ubMsgCharIdx]);
				if(
					!isLineOverflow &&
					s_sMsgCharPos.uwX + ubGlyphWidth <= HUD_ORIGIN_X + HUD_FACE_SIZE + 1 + 283
				) {
					fontDrawTextBitMap(
						s_pHudBuffer->pBack, s_pLineBuffer,
						s_sMsgCharPos.uwX, s_sMsgCharPos.uwY, COLOR_BAR_FULL, FONT_COOKIE
					);
					s_sMsgCharPos.uwX += ubGlyphWidth + 1;
				}
				else {
					isLineOverflow = 1;
				}
			}
			if(++s_ubMsgCharIdx >= s_uwMsgLen) {
				s_eState = STATE_MSG_WAIT_OUT;
				s_uwFrameDelay = 0;
			}
			else {
				s_eState = STATE_MSG_PREPARE_LETTER;
			}
			break;
		case STATE_MSG_WAIT_OUT:
			if (++s_uwFrameDelay == HUD_MSG_WAIT_CNT) {
				s_eState = STATE_MSG_NOISE_OUT;
				s_uwFrameDelay = 25;
			}
			break;
		case STATE_MSG_NOISE_OUT:
			if(--s_uwFrameDelay == 0) {
				s_eState = STATE_MSG_END;
			}
			else if(++s_uwStateCounter >= 4) {
				hudShowPage(1 + (s_uwFrameDelay % 3));
				s_uwStateCounter = 0;
			}
			break;
		case STATE_MSG_END:
			hudShowPage(0);
			hudResetStateMachine();
			break;
		case STATE_PAUSE_INITIAL_DRAW:
				blitRect(
					s_pHudBuffer->pBack, HUD_ORIGIN_X, 4 * HUD_HEIGHT + HUD_ORIGIN_Y,
					320 - 2 * HUD_ORIGIN_X, 2 * s_pFont->uwHeight + 1, COLOR_BG
				);

				fontFillTextBitMap(s_pFont, s_pLineBuffer, "Game paused");
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					HUD_ORIGIN_X + (320 - HUD_ORIGIN_X) / 2,
					4 * HUD_HEIGHT + HUD_ORIGIN_Y, COLOR_BAR_FULL, FONT_COOKIE | FONT_HCENTER
				);
				hudShowPage(4);
				s_eState = STATE_PAUSE_LOOP;
				s_ubSelection = 0;
				s_ubSelectionPrev = 1;
			break;
		case STATE_PAUSE_LOOP:
			if(s_ubSelection != s_ubSelectionPrev) {
				fontFillTextBitMap(s_pFont, s_pLineBuffer, "Resume");
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					HUD_ORIGIN_X + (320 - HUD_ORIGIN_X) / 3,
					4 * HUD_HEIGHT + HUD_ORIGIN_Y + s_pFont->uwHeight,
					(s_ubSelection == 0) ? COLOR_BAR_FULL : COLOR_BAR_EMPTY,
					FONT_COOKIE | FONT_HCENTER
				);

				fontFillTextBitMap(s_pFont, s_pLineBuffer, "Quit");
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					HUD_ORIGIN_X + 2 * (320 - HUD_ORIGIN_X) / 3,
					4 * HUD_HEIGHT + HUD_ORIGIN_Y + s_pFont->uwHeight,
					(s_ubSelection == 1) ? COLOR_BAR_FULL : COLOR_BAR_EMPTY,
					FONT_COOKIE | FONT_HCENTER
				);
				s_ubSelectionPrev = s_ubSelection;
			}
			break;
	}
}

void hudDestroy(void) {
	fontDestroyTextBitMap(s_pLineBuffer);
	bitmapDestroy(s_pFaces);
}

void hudShowMain(void) {
	hudShowPage(0);
}

void hudPause(UBYTE isPaused) {
	if(isPaused) {
		s_eState = STATE_PAUSE_INITIAL_DRAW;
	}
	else {
		hudShowPage(0);
		hudResetStateMachine();
		// TODO: perhaps reset to last "safe" state? E.g. begin of msg display
	}
}

UBYTE hudIsPaused(void) {
	return (
		s_eState == STATE_PAUSE_INITIAL_DRAW ||
		s_eState == STATE_PAUSE_LOOP
	);
}

void hudSelect(UBYTE ubSelection) {
	s_ubSelection = ubSelection;
}

UBYTE hudGetSelection(void) {
	return s_ubSelection;
}
