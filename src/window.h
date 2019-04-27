/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <ace/types.h>

#define WINDOW_WIDTH (320-64)
#define WINDOW_HEIGHT (192)

#define WINDOW_DISPLAY_X 23
#define WINDOW_DISPLAY_Y 32
#define WINDOW_DISPLAY_WIDTH 168
#define WINDOW_DISPLAY_HEIGHT 120
#define WINDOW_DISPLAY_COLOR_BG 11
#define WINDOW_DISPLAY_COLOR_TEXT 14

void windowInit(void);

void windowDeinit(void);

UBYTE windowShow(void);

void windowHide(void);

tUwCoordYX windowGetOrigin(void);

#endif // _WINDOW_H_
