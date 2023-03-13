/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _EXPLOSION_H_
#define _EXPLOSION_H_

#include <ace/types.h>

typedef void (*tCbOnPeak)(ULONG ulData);

typedef enum tExplosionKind {
	EXPLOSION_KIND_BOOM,
	EXPLOSION_KIND_TELEPORT,
} tExplosionKind;

void explosionManagerCreate(void);

void explosionManagerDestroy(void);

void explosionAdd(
	UWORD uwX, UWORD uwY, tCbOnPeak cbOnPeak, ULONG ulCbData,
	UBYTE isQuick, tExplosionKind eKind
);

void explosionManagerProcess(void);

#endif // _EXPLOSION_H_
