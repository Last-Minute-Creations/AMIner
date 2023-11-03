/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "twister.h"
#include <ace/generic/screen.h>
#include <ace/managers/blit.h>
#include <ace/utils/chunky.h>
#include "core.h"

#define CLIP_MARGIN_X 32
#define CLIP_MARGIN_Y 16
// #define TWISTER_CENTER_X (32 + 86)
// #define TWISTER_CENTER_Y ((6816 + 66) & (512))
#define TWISTER_BITMAP_OFFSET_X 32
#define TWISTER_BITMAP_OFFSET_Y (6816 & (512 - 1))
#define TWISTER_CAMERA_OFFSET_X 86
#define TWISTER_CAMERA_OFFSET_Y 60
#define TWISTER_CENTER_X (TWISTER_BITMAP_OFFSET_X + TWISTER_CAMERA_OFFSET_X)
#define TWISTER_CENTER_Y ((TWISTER_BITMAP_OFFSET_Y + TWISTER_CAMERA_OFFSET_Y) & (512 - 1))
#define TWISTER_CENTER_RADIUS 5
#define TWISTER_BLOCK_SIZE 32
#define TWISTER_MIN_BLOCK_X (-(((TWISTER_CAMERA_OFFSET_X + TWISTER_BLOCK_SIZE - 1) / TWISTER_BLOCK_SIZE) + 2))
#define TWISTER_MAX_BLOCK_X (+((((SCREEN_PAL_WIDTH - TWISTER_CAMERA_OFFSET_X) + TWISTER_BLOCK_SIZE - 1) / TWISTER_BLOCK_SIZE) + 0))
#define TWISTER_MIN_BLOCK_Y (-(((TWISTER_CAMERA_OFFSET_Y + TWISTER_BLOCK_SIZE - 1) / TWISTER_BLOCK_SIZE) + 2))
#define TWISTER_MAX_BLOCK_Y (+(((SCREEN_PAL_HEIGHT - TWISTER_CAMERA_OFFSET_Y + TWISTER_BLOCK_SIZE - 1) / TWISTER_BLOCK_SIZE) + 0))
// #define TWISTER_MIN_BLOCK_X  (-2)
// #define TWISTER_MAX_BLOCK_X (+2)
// #define TWISTER_MIN_BLOCK_Y (-2)
// #define TWISTER_MAX_BLOCK_Y (2)
#define TWISTER_BLOCKS_X (TWISTER_MAX_BLOCK_X - TWISTER_MIN_BLOCK_X)
#define TWISTER_BLOCKS_Y (TWISTER_MAX_BLOCK_Y - TWISTER_MIN_BLOCK_Y)
#define TWISTER_LEFT_BORDER (TWISTER_BITMAP_OFFSET_X - CLIP_MARGIN_X)
#define TWISTER_RIGHT_BORDER (TWISTER_BITMAP_OFFSET_X + SCREEN_PAL_WIDTH + CLIP_MARGIN_X)
#define TWISTER_TOP_BORDER (TWISTER_BITMAP_OFFSET_Y - CLIP_MARGIN_Y)
#define TWISTER_BOTTOM_BORDER (TWISTER_BITMAP_OFFSET_Y + SCREEN_PAL_HEIGHT + CLIP_MARGIN_Y)

static ULONG s_ps;
static UBYTE s_isTwisterEnabled;

void twisterEnable(void) {
	s_isTwisterEnabled = 1;
	s_ps = 0;
}

void twisterDisable(void) {
	s_isTwisterEnabled = 0;
}

void twisterProcess(void) {
	if(!s_isTwisterEnabled) {
		return;
	}

	++s_ps;

	UWORD uwShift = 0;
	// for(UBYTE i = 0; i <= 4; ++i) {
		uwShift = (uwShift << 1) | ((s_ps >> 0) & 1);
		uwShift = (uwShift << 1) | ((s_ps >> 1) & 1);
		uwShift = (uwShift << 1) | ((s_ps >> 2) & 1);
		uwShift = (uwShift << 1) | ((s_ps >> 3) & 1);
		uwShift = (uwShift << 1) | ((s_ps >> 4) & 1);
	// }

	for(BYTE y = TWISTER_MIN_BLOCK_Y; y < TWISTER_MAX_BLOCK_Y; ++y) {
		WORD yy = TWISTER_CENTER_Y + y * TWISTER_BLOCK_SIZE + uwShift;
		for(BYTE x = TWISTER_MIN_BLOCK_X; x < TWISTER_MAX_BLOCK_X; ++x) {
			WORD xx = TWISTER_CENTER_X + x * TWISTER_BLOCK_SIZE + uwShift;

			WORD wSrcX = xx - (y + 1) - (x + 1);
			WORD wSrcY = yy - (y + 1) + (x + 1);
			WORD wDstX = xx;
			WORD wDstY = yy;
			WORD wWidth = TWISTER_BLOCK_SIZE;
			WORD wHeight = TWISTER_BLOCK_SIZE;

			if(wDstX < TWISTER_LEFT_BORDER) {
				WORD wDelta = TWISTER_LEFT_BORDER - wDstX;
				wSrcX += wDelta;
				wWidth -= wDelta;
				wDstX = TWISTER_LEFT_BORDER;
			}
			if(wSrcX < TWISTER_LEFT_BORDER) {
				WORD wDelta = TWISTER_LEFT_BORDER - wSrcX;
				wDstX += wDelta;
				wWidth -= wDelta;
				wSrcX = TWISTER_LEFT_BORDER;
			}
			if(wSrcX + wWidth > TWISTER_RIGHT_BORDER) {
				wWidth = SCREEN_PAL_WIDTH + CLIP_MARGIN_X - wSrcX;
			}
			if(wDstX + wWidth > TWISTER_RIGHT_BORDER) {
				wWidth = SCREEN_PAL_WIDTH + CLIP_MARGIN_X - wDstX;
			}

			if(wDstY < TWISTER_TOP_BORDER) {
				WORD wDelta = TWISTER_TOP_BORDER - wDstY;
				wSrcY += wDelta;
				wHeight -= wDelta;
				wDstY = TWISTER_TOP_BORDER;
			}
			if(wSrcY < TWISTER_TOP_BORDER) {
				WORD wDelta = TWISTER_TOP_BORDER - wSrcY;
				wDstY += wDelta;
				wWidth -= wDelta;
				wSrcY = TWISTER_TOP_BORDER;
			}
			if(wSrcY + wHeight > TWISTER_BOTTOM_BORDER) {
				wHeight = TWISTER_BOTTOM_BORDER - wSrcY;
			}
			if(wDstY + wHeight > TWISTER_BOTTOM_BORDER) {
				wHeight = TWISTER_BOTTOM_BORDER - wDstY;
			}

			if(wWidth <= 0 || wHeight <= 0) {
				continue;
			}

			blitCopy(
				g_pMainBuffer->pScroll->pFront, wSrcX, wSrcY,
				g_pMainBuffer->pScroll->pBack, wDstX, wDstY, wWidth, wHeight, MINTERM_COOKIE
			);
		}
	}

	UBYTE ubColor = 17 + (randUw(&g_sRand) & 3);
	blitRect(
		g_pMainBuffer->pScroll->pBack,
		TWISTER_CENTER_X - 1,
		TWISTER_CENTER_Y - 35,
		2, 70, ubColor
	);
	blitRect(
		g_pMainBuffer->pScroll->pBack,
		TWISTER_CENTER_X - 35,
		TWISTER_CENTER_Y - 1,
		70, 1, ubColor
	);
}
