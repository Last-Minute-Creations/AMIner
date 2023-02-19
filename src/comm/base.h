/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _AMINER_COMM_BASE_H_
#define _AMINER_COMM_BASE_H_

#include <ace/utils/font.h>
#include "face_id.h"

#define COMM_WIDTH (320-64)
#define COMM_HEIGHT (192)

#define COMM_DISPLAY_X 23
#define COMM_DISPLAY_Y 32
#define COMM_DISPLAY_WIDTH 168
#define COMM_DISPLAY_HEIGHT 120
#define COMM_DISPLAY_COLOR_BG 11
#define COMM_DISPLAY_COLOR_TEXT 14
#define COMM_DISPLAY_COLOR_TEXT_DARK 13

typedef void (*tPageProcess)(void);

typedef void (*tPageCleanup)(void);

typedef enum _tCommNav {
	COMM_NAV_UP,
	COMM_NAV_DOWN,
	COMM_NAV_LEFT,
	COMM_NAV_RIGHT,
	COMM_NAV_BTN,
	COMM_NAV_COUNT
} tCommNav;

typedef enum _tCommNavEx {
	COMM_NAV_EX_BTN_CLICK,
	COMM_NAV_EX_SHIFT_LEFT,
	COMM_NAV_EX_SHIFT_RIGHT,
	COMM_NAV_EX_COUNT
} tCommNavEx;

/**
 * @brief Tabs in Commrade interface.
 * Must be same order as MSG_TUTORIAL_DESCRIPTION_TAB_* enum!
 */
typedef enum _tCommTab {
	COMM_TAB_OFFICE,
	COMM_TAB_WORKSHOP,
	COMM_TAB_WAREHOUSE,
	COMM_TAB_COUNT
} tCommTab;

typedef enum _tBtnState {
	BTN_STATE_NACTIVE = 0,
	BTN_STATE_ACTIVE = 1,
	BTN_STATE_USED
} tBtnState;

void commCreate(void);

void commDestroy(void);

void commProcess(void);

UBYTE commTryShow(void);

void commHide(void);

UBYTE commIsShown(void);

tBtnState commNavCheck(tCommNav eNav);

UBYTE commNavUse(tCommNav eNav);

UBYTE commNavExUse(tCommNavEx eNavEx);

tBitMap *commGetDisplayBuffer(void);

tUwCoordYX commGetOrigin(void);

tUwCoordYX commGetOriginDisplay(void);

void commSetActiveLed(tCommTab eLed);

void commDrawText(
	UWORD uwX, UWORD uwY, const char *szText, UBYTE ubFontFlags, UBYTE ubColor
);

/**
 * @brief Draws multiline text, dividing it where whitespace occurs.
 *
 * @param szText Text to display.
 * @param uwStartX Start X position of each line, relative to Commrade screen origin.
 * @param uwStartY Stary Y position of first line, relative to Commrade screen origin.
 * @return Number of lines written.
 */
UBYTE commDrawMultilineText(const char *szText, UWORD uwStartX, UWORD uwStartY);

void commDrawTitle(UWORD uwX, UWORD uwY, const char *szTitle);

void commDrawFaceAt(tFaceId eFace, UWORD uwX, UWORD uwY);

void commErase(UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight);

void commEraseAll(void);

void commProgress(UBYTE ubPercent, const char *szDescription);

UBYTE commBreakTextToWidth(const char *szInput, UWORD uwMaxLineWidth);

UBYTE commGetLineHeight(void);

void commRegisterPage(tPageProcess cbProcess, tPageCleanup cbCleanup);

UBYTE commProcessPage(void);

extern tBitMap *g_pCommBmFaces;
extern tBitMap *g_pCommBmSelection;

#endif // _AMINER_COMM_BASE_H_
