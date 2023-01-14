/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hud.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/chunky.h>
#include <ace/utils/string.h>
#include "defs.h"

#define HUD_COLOR_BG 11
#define HUD_COLOR_BAR_FULL 14
#define HUD_COLOR_BAR_EMPTY 12

#define HUD_HEIGHT 31
#define HUD_ORIGIN_X 11
#define HUD_ORIGIN_Y 9
#define HUD_FACE_SIZE 16

#define GAUGE_DRILL_X (HUD_ORIGIN_X + 65)
#define GAUGE_CARGO_X (HUD_ORIGIN_X + 125)
#define GAUGE_HULL_X (HUD_ORIGIN_X + 182)
#define GAUGE_DEPTH_X (HUD_ORIGIN_X + 248)
#define GAUGE_CASH_X (HUD_ORIGIN_X + 248)

#define ROW_1_Y (HUD_ORIGIN_Y + 0)
#define ROW_2_Y (HUD_ORIGIN_Y + 9)
#define HUD_MSG_BFR_SIZE 250
#define HUD_MSG_WAIT_CNT 150

#define MODE_ICON_HEIGHT 12
#define MODE_DISABLED_OFFS_Y 3 * MODE_ICON_HEIGHT

typedef enum _tHudPage {
	HUD_PAGE_MAIN,
	HUD_PAGE_NOISE_1,
	HUD_PAGE_NOISE_2,
	HUD_PAGE_NOISE_3,
	HUD_PAGE_MSG,
	HUD_PAGE_PAUSE,
	HUD_PAGE_MODE,
	HUD_PAGE_COUNT,
} tHudPage;

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
	STATE_MODE_SHOW,
	STATE_MODE_LOOP,
	STATE_MODE_HIDE,
} tHudState;

typedef struct _tHudPlayerData {
	UWORD uwDepth, uwDepthDisp;
	LONG lCash, lCashDisp;
	UWORD uwDrill, uwDrillMax;
	UBYTE ubCargo, ubCargoMax;
	UWORD uwHull, uwHullMax;
	UBYTE ubHullDisp, ubCargoDisp, ubDrillDisp;
	tMode eMode, eModePrev;
} tHudPlayerData;

static tVPort *s_pVpHud;
static tSimpleBufferManager *s_pHudBuffer;
static const tFont *s_pFont;
static UBYTE s_ubLineHeight;
static tTextBitMap *s_pLineBuffer;
static UBYTE s_isBitmapFilled = 0;
static tHudPlayerData s_pPlayerData[2];
static tHudState s_eState;
static tHudPlayer s_ePlayer;
static UBYTE s_ubHudOffsY;
static UBYTE s_isChallenge, s_is2pPlaying;
static UWORD s_uwFrameDelay, s_uwStateCounter;
static tHudPage s_ePageCurrent = HUD_PAGE_MAIN;

// Message vars
static UWORD s_uwMsgLen;
static tUwCoordYX s_sMsgCharPos;
static UBYTE s_ubMsgCharIdx;
static char s_szMsg[HUD_MSG_BFR_SIZE];
static char s_szLetter[2] = {'\0'};
static tBitMap *s_pFaces;
static tFaceId s_eFaceToDraw;

// Pause vars
static UBYTE s_ubSelection, s_ubSelectionPrev;

// Mode vars
static tBitMap *s_pModeIcons, *s_pModeCursor, *s_pModeCursorMask;
static WORD s_pModeCounters[MODE_COUNT] = {-1, 0, 0, 0};
static WORD s_pModeCountersPrev[MODE_COUNT] = {-1, 0, 0, 0};
static tMode s_eModeToDraw;
static UBYTE s_isPendingModeShow;
static tHudPage s_eModeReturnPage;
static tHudState s_eModeReturnState;
static UBYTE s_ubHudShowStack;

//----------------------------------------------------------------------- STATIC

static void hudResetStateMachine(void) {
	s_eState = (
		s_isChallenge ? STATE_MAIN_PREPARE_CASH : STATE_MAIN_PREPARE_DEPTH
	);
	s_ePlayer = PLAYER_1;
	s_ubHudOffsY = ROW_1_Y;
	s_isBitmapFilled = 0;
}

static void hudShowPage(tHudPage eHudPage) {
	s_ePageCurrent = eHudPage;
	cameraSetCoord(s_pHudBuffer->pCamera, 0, eHudPage * 31);
}

//------------------------------------------------------------------------ PAUSE

void hudPause(UBYTE isPaused) {
	if(isPaused) {
		s_eState = STATE_PAUSE_INITIAL_DRAW;
	}
	else {
		hudShowMain();
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

//------------------------------------------------------------------------- MODE

static void hudDrawModeIcon(tMode eMode) {
	static const UWORD uwModeWidth = 40 - 1;
	UWORD uwOffsX = HUD_ORIGIN_X + (eMode + 1) * uwModeWidth;
	UWORD uwOffsY = HUD_ORIGIN_Y + HUD_PAGE_MODE * 31 + 1;

	// Erase selection
	blitRect(
		s_pHudBuffer->pBack, uwOffsX, uwOffsY - 1, uwModeWidth, 15, HUD_COLOR_BG
	);

	// Draw icon
	UWORD uwIconOffs = eMode * MODE_ICON_HEIGHT;
	if(eMode != MODE_DRILL && !s_pModeCounters[eMode]) {
		uwIconOffs += MODE_DISABLED_OFFS_Y;
	}
	blitCopy(
		s_pModeIcons, 0, uwIconOffs,
		s_pHudBuffer->pBack, uwOffsX + 1, uwOffsY,
		16, MODE_ICON_HEIGHT, MINTERM_COOKIE
	);

	// Draw selection
	if(s_pPlayerData[PLAYER_1].eMode == eMode) {
		blitCopyMask(
			s_pModeCursor, 0, 0,
			s_pHudBuffer->pBack, uwOffsX, uwOffsY - 1,
			16, 15, (UWORD*)s_pModeCursorMask->Planes[0]
		);
	}
	if(s_is2pPlaying && s_pPlayerData[PLAYER_2].eMode == eMode) {
		blitCopyMask(
			s_pModeCursor, 0, 15,
			s_pHudBuffer->pBack, uwOffsX, uwOffsY - 1,
			16, 15, (UWORD*)s_pModeCursorMask->Planes[0]
		);
	}

	// Draw count
	if(eMode != MODE_DRILL) {
		char szCount[6];
		stringDecimalFromULong(s_pModeCounters[eMode], szCount);
		fontFillTextBitMap(s_pFont, s_pLineBuffer, szCount);
		fontDrawTextBitMap(
			s_pHudBuffer->pBack, s_pLineBuffer,
			uwOffsX + 1 + 16, uwOffsY + MODE_ICON_HEIGHT / 2,
			HUD_COLOR_BAR_FULL, FONT_COOKIE | FONT_VCENTER
		);
	}
}

void hudSetModeCounter(tMode eMode, WORD wCount) {
	s_pModeCounters[eMode] = wCount;
}

void hudSetMode(tHudPlayer ePlayer, tMode eMode) {
	s_pPlayerData[ePlayer].eMode = eMode;
}

void hudShowMode(void) {
	if(++s_ubHudShowStack == 1) {
		s_isPendingModeShow = 1;
		s_eModeToDraw = MODE_DRILL;
	}
}

void hudHideMode(void) {
	if(--s_ubHudShowStack == 0) {
		s_eState = STATE_MODE_HIDE;
	}
}

//------------------------------------------------------------------------- MAIN

void hudCreate(tView *pView, const tFont *pFont) {
  s_pVpHud = vPortCreate(0,
    TAG_VPORT_VIEW, pView,
    TAG_VPORT_BPP, 5,
    TAG_VPORT_HEIGHT, 31,
  TAG_END);

  s_pHudBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpHud,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, 31 * HUD_PAGE_COUNT,
  TAG_END);

	bitmapLoadFromFile(s_pHudBuffer->pBack, "data/hud.bm", 0, 0);
	s_pFaces = bitmapCreateFromFile("data/comm_faces.bm", 0);
	s_pModeIcons = bitmapCreateFromFile("data/mode_icons.bm", 0);
	s_pModeCursor = bitmapCreateFromFile("data/mode_cursor.bm", 0);
	s_pModeCursorMask = bitmapCreateFromFile("data/mode_cursor_mask.bm", 0);

	s_pFont = pFont;
	s_ubLineHeight = 7;
	s_pLineBuffer = fontCreateTextBitMap(s_pHudBuffer->uBfrBounds.uwX, s_pFont->uwHeight);

	fontDrawStr(
		s_pFont, s_pHudBuffer->pBack, HUD_ORIGIN_X, ROW_1_Y - 3,
		g_pMsgs[MSG_HUD_P1], HUD_COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE, s_pLineBuffer
	);
	fontDrawStr(
		s_pFont, s_pHudBuffer->pBack, HUD_ORIGIN_X, ROW_2_Y - 3,
		g_pMsgs[MSG_HUD_P2], HUD_COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE, s_pLineBuffer
	);

	fontDrawStr(
		s_pFont, s_pHudBuffer->pBack, GAUGE_DRILL_X - 1, ROW_1_Y - 3,
		g_pMsgs[MSG_HUD_DRILL], HUD_COLOR_BAR_FULL,
		FONT_LAZY | FONT_COOKIE | FONT_RIGHT, s_pLineBuffer
	);
	fontDrawStr(
		s_pFont, s_pHudBuffer->pBack, GAUGE_CARGO_X - 1, ROW_1_Y - 3,
		g_pMsgs[MSG_HUD_CARGO], HUD_COLOR_BAR_FULL,
		FONT_LAZY | FONT_COOKIE | FONT_RIGHT, s_pLineBuffer
	);
	fontDrawStr(
		s_pFont, s_pHudBuffer->pBack, GAUGE_HULL_X - 1, ROW_1_Y - 3,
		g_pMsgs[MSG_HUD_HULL], HUD_COLOR_BAR_FULL,
		FONT_LAZY | FONT_COOKIE | FONT_RIGHT, s_pLineBuffer
	);
	fontDrawStr(
		s_pFont, s_pHudBuffer->pBack, GAUGE_CASH_X - 1, ROW_1_Y - 3,
		g_pMsgs[MSG_HUD_CASH], HUD_COLOR_BAR_FULL,
		FONT_LAZY | FONT_COOKIE | FONT_RIGHT, s_pLineBuffer
	);
	hudReset(0, 0);
}

UBYTE hudIsShowingMessage(void) {
	return STATE_MSG_NOISE_IN <= s_eState && s_eState <= STATE_MSG_END;
}

void hudShowMessage(tFaceId eFace, const char *szMsg) {
	logWrite("Showing HUD message: '%s'\n", szMsg);
	stringCopyLimited(szMsg, s_szMsg, HUD_MSG_BFR_SIZE);
	s_eState = STATE_MSG_NOISE_IN;
	s_uwFrameDelay = 25;
	s_uwStateCounter = 0;
	s_eFaceToDraw = eFace;
}

void hudSet2pPlaying(UBYTE isPlaying) {
	s_is2pPlaying = isPlaying;
}

void hudReset(UBYTE isChallenge, UBYTE is2pPlaying) {
	s_isChallenge = isChallenge;
	s_is2pPlaying = is2pPlaying;
	s_ubHudShowStack = 0;
	const UBYTE ubLabelWidth = fontMeasureText(
		s_pFont, g_pMsgs[MSG_HUD_DEPTH]
	).uwX;
	if(isChallenge) {
		// Clear depth label and use it as cash
		blitRect(
			s_pHudBuffer->pBack, GAUGE_DEPTH_X - 1 - ubLabelWidth, ROW_2_Y,
			ubLabelWidth, s_ubLineHeight, HUD_COLOR_BG
		);
	}
	else {
		// Depth instead of 2p cash
		fontDrawStr(
			s_pFont, s_pHudBuffer->pBack, GAUGE_DEPTH_X - 1, ROW_2_Y - 3,
			g_pMsgs[MSG_HUD_DEPTH], HUD_COLOR_BAR_FULL,
			FONT_LAZY | FONT_COOKIE | FONT_RIGHT, s_pLineBuffer
		);
	}

	// Empty bars
	UWORD pBarOffsX[3] = {GAUGE_DRILL_X, GAUGE_CARGO_X, GAUGE_HULL_X};
	for(UBYTE i = 0; i < 3; ++i) {
		for(UBYTE b = 0; b < 10; ++b) {
			blitRect(
				s_pHudBuffer->pBack, pBarOffsX[i] + 3 * b, ROW_1_Y,
				2, 5, HUD_COLOR_BAR_EMPTY
			);
			blitRect(
				s_pHudBuffer->pBack, pBarOffsX[i] + 3 * b, ROW_2_Y,
				2, 5, HUD_COLOR_BAR_EMPTY
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
				char *pEnd = szBfr;
				pEnd = stringDecimalFromULong(uwDepth / 10, pEnd);
				*(pEnd++) = '.';
				pEnd = stringDecimalFromULong(uwDepth % 10, pEnd);
				*(pEnd++) = 'm';
				*(pEnd++) = '\0';
				fontFillTextBitMap(s_pFont, s_pLineBuffer, szBfr);
				s_isBitmapFilled = 1;
				pData->uwDepthDisp = uwDepth;
				s_eState = STATE_MAIN_DRAW_DEPTH;
				isDrawPending = 1;
				break;
			}
			// fallthrough if value is the same
		case STATE_MAIN_DRAW_DEPTH:
			if(isDrawPending) {
				// decreased clear height 'cuz digits are smaller than whole font
				blitRect(
					s_pHudBuffer->pBack, GAUGE_DEPTH_X, ROW_2_Y,
					320 - (GAUGE_DEPTH_X + HUD_ORIGIN_X), s_ubLineHeight - 2, HUD_COLOR_BG
				);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					GAUGE_DEPTH_X, ROW_2_Y - 3, HUD_COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE
				);
				s_isBitmapFilled = 0;
				s_eState = STATE_MAIN_PREPARE_CASH;
				isDrawPending = 0;
				break;
			}
			// fallthrough if doesn't need to draw new value
		case STATE_MAIN_PREPARE_CASH:
			if(s_isChallenge) {
				lCash = s_pPlayerData[s_ePlayer].lCash;
			}
			else {
				lCash = s_pPlayerData[0].lCash + s_pPlayerData[1].lCash;
			}
			if(lCash != pData->lCashDisp) {
				ULONG ulDisp;
				char *pEnd = szBfr;
				if(lCash < 0) {
					ulDisp = -lCash;
					*(pEnd++) = '-';
				}
				else {
					ulDisp = lCash;
				}
				char *pStart = pEnd;
				pEnd = stringDecimalFromULong(ulDisp, pEnd);
				*(pEnd++) = '\x1F';
				*pEnd = '\0';
				char *pSeparator = pEnd - 4;
				while(pSeparator > pStart) {
					char *pSrc = pEnd;
					char *pDst = ++pEnd;
					while(pDst != pSeparator) {
						*(pDst--) = *(pSrc--);
					}
					*pSeparator = '.';
					pSeparator -= 3;
				}
				fontFillTextBitMap(s_pFont, s_pLineBuffer, szBfr);
				s_isBitmapFilled = 1;
				pData->lCashDisp = lCash;
				s_eState = STATE_MAIN_DRAW_CASH;
				isDrawPending = 1;
				break;
			}
			// fallthrough if value is the same
		case STATE_MAIN_DRAW_CASH:
			if(isDrawPending) {
				// decreased clear height 'cuz digits are smaller than whole font
				UBYTE ubY = (s_isChallenge ? s_ubHudOffsY : ROW_1_Y);
				blitRect(
					s_pHudBuffer->pBack, GAUGE_CASH_X, ubY,
					320 - (GAUGE_CASH_X + HUD_ORIGIN_X), s_ubLineHeight - 1, HUD_COLOR_BG
				);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					GAUGE_CASH_X, ubY - 3, HUD_COLOR_BAR_FULL, FONT_LAZY | FONT_COOKIE
				);
				s_isBitmapFilled = 0;
				s_eState = STATE_MAIN_DRAW_FUEL;
				isDrawPending = 0;
				break;
			}
			// fallthrough if doesn't need to draw new value
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
					ubColor = HUD_COLOR_BAR_FULL;
				}
				else {
					--pData->ubDrillDisp;
					ubDraw = pData->ubDrillDisp;
					ubColor = HUD_COLOR_BAR_EMPTY;
				}
				hudDrawStateBarPercent(GAUGE_DRILL_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eState = STATE_MAIN_DRAW_CARGO;
				break;
			}
			// fallthrough if doesn't need to draw new value
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
					ubColor = HUD_COLOR_BAR_FULL;
				}
				else {
					--pData->ubCargoDisp;
					ubDraw = pData->ubCargoDisp;
					ubColor = HUD_COLOR_BAR_EMPTY;
				}
				hudDrawStateBarPercent(GAUGE_CARGO_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eState = STATE_MAIN_DRAW_HULL;
				break;
			}
			// fallthrough if doesn't need to draw new value
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
					ubColor = HUD_COLOR_BAR_FULL;
				}
				else {
					--pData->ubHullDisp;
					ubDraw = pData->ubHullDisp;
					ubColor = HUD_COLOR_BAR_EMPTY;
				}
				hudDrawStateBarPercent(GAUGE_HULL_X, s_ubHudOffsY, ubDraw, ubColor);
				s_eState = STATE_MAIN_DRAW_END;
				break;
			}
			// fallthrough if doesn't need to draw new value
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
				hudShowPage(HUD_PAGE_NOISE_1 + (s_uwFrameDelay % 3));
				s_uwStateCounter = 0;
			}
			break;
		case STATE_MSG_DRAW_FACE:
			isLineOverflow = 0;
			s_sMsgCharPos.uwX = HUD_ORIGIN_X;
			s_sMsgCharPos.uwY = 4 * HUD_HEIGHT + HUD_ORIGIN_Y;

			blitCopy(
				s_pFaces, 0, s_eFaceToDraw * HUD_FACE_SIZE,
				s_pHudBuffer->pBack, s_sMsgCharPos.uwX, s_sMsgCharPos.uwY,
				HUD_FACE_SIZE, HUD_FACE_SIZE, MINTERM_COOKIE
			);
			s_sMsgCharPos.uwX += HUD_FACE_SIZE + 1;
			blitRect(
				s_pHudBuffer->pBack, s_sMsgCharPos.uwX, s_sMsgCharPos.uwY,
				283, 2 * s_ubLineHeight + 1, HUD_COLOR_BG
			);
			s_sMsgCharPos.uwY -= 2;

			hudShowPage(HUD_PAGE_MSG);
			s_ubMsgCharIdx = 0;
			s_uwMsgLen = strlen(s_szMsg);
			s_eState = STATE_MSG_PREPARE_LETTER;
			break;
		case STATE_MSG_PREPARE_LETTER:
			s_szLetter[0] = s_szMsg[s_ubMsgCharIdx];
			if(s_szMsg[s_ubMsgCharIdx] != '\n') {
				fontFillTextBitMap(s_pFont, s_pLineBuffer, s_szLetter);
			}
			s_isBitmapFilled = 1;
			s_eState = STATE_MSG_DRAW_LETTER;
			break;
		case STATE_MSG_DRAW_LETTER:
			if(s_szMsg[s_ubMsgCharIdx] == '\n') {
				s_sMsgCharPos.uwX = HUD_ORIGIN_X + HUD_FACE_SIZE + 1;
				s_sMsgCharPos.uwY += s_ubLineHeight + 1;
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
						s_sMsgCharPos.uwX, s_sMsgCharPos.uwY, HUD_COLOR_BAR_FULL, FONT_COOKIE
					);
					s_sMsgCharPos.uwX += ubGlyphWidth + 1;
				}
				else {
					isLineOverflow = 1;
				}
			}
			s_isBitmapFilled = 0;
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
				hudShowPage(HUD_PAGE_NOISE_1 + (s_uwFrameDelay % 3));
				s_uwStateCounter = 0;
			}
			break;
		case STATE_MSG_END:
			hudShowPage(HUD_PAGE_MAIN);
			hudResetStateMachine();
			break;
		case STATE_PAUSE_INITIAL_DRAW: {
				const UWORD uwPageOriginY = HUD_PAGE_PAUSE * HUD_HEIGHT + HUD_ORIGIN_Y;
				blitRect(
					s_pHudBuffer->pBack, HUD_ORIGIN_X, uwPageOriginY,
					320 - 2 * HUD_ORIGIN_X, 2 * s_ubLineHeight + 1, HUD_COLOR_BG
				);

				fontFillTextBitMap(s_pFont, s_pLineBuffer, g_pMsgs[MSG_HUD_PAUSED]);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					HUD_ORIGIN_X + (320 - HUD_ORIGIN_X) / 2,
					uwPageOriginY - 3, HUD_COLOR_BAR_FULL, FONT_COOKIE | FONT_HCENTER
				);
				hudShowPage(HUD_PAGE_PAUSE);
				s_eState = STATE_PAUSE_LOOP;
				s_ubSelection = 0;
				s_ubSelectionPrev = 1;
		} break;
		case STATE_PAUSE_LOOP:
			if(s_ubSelection != s_ubSelectionPrev) {
				const UWORD uwPageOriginY = HUD_PAGE_PAUSE * HUD_HEIGHT + HUD_ORIGIN_Y;
				fontFillTextBitMap(s_pFont, s_pLineBuffer, g_pMsgs[MSG_HUD_RESUME]);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					HUD_ORIGIN_X + (320 - HUD_ORIGIN_X) / 3,
					uwPageOriginY - 3 + s_ubLineHeight,
					(s_ubSelection == 0) ? HUD_COLOR_BAR_FULL : HUD_COLOR_BAR_EMPTY,
					FONT_COOKIE | FONT_HCENTER
				);

				fontFillTextBitMap(s_pFont, s_pLineBuffer, g_pMsgs[MSG_HUD_QUIT]);
				fontDrawTextBitMap(
					s_pHudBuffer->pBack, s_pLineBuffer,
					HUD_ORIGIN_X + 2 * (320 - HUD_ORIGIN_X) / 3,
					uwPageOriginY - 3 + s_ubLineHeight,
					(s_ubSelection == 1) ? HUD_COLOR_BAR_FULL : HUD_COLOR_BAR_EMPTY,
					FONT_COOKIE | FONT_HCENTER
				);
				s_ubSelectionPrev = s_ubSelection;
			}
			break;
		case STATE_MODE_SHOW:
			s_eModeReturnPage = s_ePageCurrent;
			if(s_eModeToDraw < MODE_COUNT) {
				hudDrawModeIcon(s_eModeToDraw);
				++s_eModeToDraw;
			}
			else {
				hudShowPage(HUD_PAGE_MODE);
				s_eState = STATE_MODE_LOOP;
			}
			break;
		case STATE_MODE_LOOP:
			if(s_ePlayer == PLAYER_1 || s_is2pPlaying) {
				tHudPlayerData *pData = &s_pPlayerData[s_ePlayer];
				if(pData->eModePrev != pData->eMode) {
					hudDrawModeIcon(pData->eModePrev);
					hudDrawModeIcon(pData->eMode);
					pData->eModePrev = pData->eMode;
				}
				else if(s_pModeCountersPrev[pData->eMode] != s_pModeCounters[pData->eMode]) {
					hudDrawModeIcon(pData->eMode);
					s_pModeCountersPrev[pData->eMode] = s_pModeCounters[pData->eMode];
				}
			}
			// Cycle players and start again
			s_ePlayer = (s_ePlayer == PLAYER_1) ? PLAYER_2 : PLAYER_1;
		break;
		case STATE_MODE_HIDE:
			hudShowPage(s_eModeReturnPage);
			s_eState = s_eModeReturnState;
			break;
	}
	if(s_isPendingModeShow && !s_isBitmapFilled) {
		s_eModeReturnState = s_eState;
		s_eState = STATE_MODE_SHOW;
		s_isPendingModeShow = 0;
	}
}

void hudDestroy(void) {
	fontDestroyTextBitMap(s_pLineBuffer);
	bitmapDestroy(s_pFaces);
	bitmapDestroy(s_pModeIcons);
	bitmapDestroy(s_pModeCursor);
	bitmapDestroy(s_pModeCursorMask);
}

void hudShowMain(void) {
	hudShowPage(HUD_PAGE_MAIN);
}
