/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _STEER_H_
#define _STEER_H_

#include <ace/types.h>
#include <ace/macros.h>

typedef enum _tSteer {
	STEER_P1_UP    = 1 << 0,
	STEER_P1_DOWN  = 1 << 1,
	STEER_P1_LEFT  = 1 << 2,
	STEER_P1_RIGHT = 1 << 3,
	STEER_P2_UP    = 1 << 4,
	STEER_P2_DOWN  = 1 << 5,
	STEER_P2_LEFT  = 1 << 6,
	STEER_P2_RIGHT = 1 << 7,
	STEER_P1_FIRE  = 1 << 8,
	STEER_P2_FIRE  = 1 << 9,
} tSteer;

extern UWORD g_uwSteer;

void steerUpdateFromInput(UBYTE is1pKbd, UBYTE is2pKbd);

static inline UWORD steerGet(UWORD uwInput) {
	return g_uwSteer & uwInput;
}

UBYTE steerUse(UWORD uwInput);

#endif // _STEER_H_
