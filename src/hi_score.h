/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _HI_SCORE_H_
#define _HI_SCORE_H_

#include <ace/types.h>
#include <ace/utils/bitmap.h>

void hiScoreEnteringProcess(tBitMap *pDisplayBuffer);

UBYTE hiScoreIsEntering(void);

void hiScoreSetup(LONG lScore, const char *szResult);

void hiScoreLoad(void);

void hiScoreDrawAll(tBitMap *pDisplayBuffer);

#endif // _HI_SCORE_H_
