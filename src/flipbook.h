/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _FLIPBOOK_H_
#define _FLIPBOOK_H_

#include <ace/types.h>

typedef void (*tCbOnPeak)(ULONG ulData);

typedef enum tFlipbookKind {
	FLIPBOOK_KIND_BOOM,
	FLIPBOOK_KIND_TELEPORT,
	FLIPBOOK_KIND_TELEPORTER_OUT,
	FLIPBOOK_KIND_TELEPORTER_IN,
} tFlipbookKind;

void flipbookManagerCreate(void);

void flipbookManagerDestroy(void);

void flipbookAdd(
	UWORD uwX, UWORD uwY, tCbOnPeak cbOnPeak, ULONG ulCbData,
	UBYTE isQuick, tFlipbookKind eKind
);

void flipbookManagerProcess(void);

#endif // _FLIPBOOK_H_
