/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _BOB_NEW_H_
#define _BOB_NEW_H_

#include <ace/types.h>
#include <ace/managers/blit.h>

typedef struct _tBobNew {
	tUwCoordYX pOldPositions[2];
	tUwCoordYX sPos;
	UWORD uwWidth;
	UWORD uwHeight;
	UBYTE isUndrawRequired;
	tBitMap *pBitmap;
	tBitMap *pMask;
	UWORD uwOffsetY;
	// Platform-dependent private fields. Don't rely on them externally.
	UWORD _uwBlitSize;
	WORD _wModuloUndrawSave;
} tBobNew;

void bobNewManagerCreate(tBitMap *pFront, tBitMap *pBack, UWORD uwAvailHeight);

void bobNewAllocateBgBuffers(void);

void bobNewManagerDestroy(void);

void bobNewPush(tBobNew *pBob);

void bobNewInit(
	tBobNew *pBob, UWORD uwWidth, UWORD uwHeight, UBYTE isUndrawRequired,
	tBitMap *pBitMap, tBitMap *pMask, UWORD uwX, UWORD uwY
);

void bobNewSetBitMapOffset(tBobNew *pBob, UWORD uwOffsetY);

UBYTE bobNewProcessNext(void);

void bobNewBegin(void);

void bobNewPushingDone(void);

void bobNewEnd(void);

void bobNewDiscardUndraw(void);

#endif // _BOB_NEW_H_
