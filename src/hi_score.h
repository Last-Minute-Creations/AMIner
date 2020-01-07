/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _HI_SCORE_H_
#define _HI_SCORE_H_

#include <ace/types.h>
#include "string_array.h"

typedef enum _tMsgHiScore {
	MSG_HI_SCORE_NEW,
	MSG_HI_SCORE_PRESS,
	MSG_HI_SCORE_COUNT,
} tMsgHiScore;

void hiScoreEnteringProcess(void);

UBYTE hiScoreIsEntering(void);

void hiScoreSetup(LONG lScore, const char *szResult);

void hiScoreLoad(void);

void hiScoreDrawAll(void);

extern tStringArray g_sHiScoreMessages;

#endif // _HI_SCORE_H_
