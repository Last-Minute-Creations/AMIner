/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _HI_SCORE_H_
#define _HI_SCORE_H_

#include <ace/types.h>

void hiScoreEnteringProcess(void);

UBYTE hiScoreIsEntering(void);

void hiScoreBobsDisplay(void);

void hiScoreBobsCreate(void);

void hiScoreBobsDestroy(void);

void hiScoreSetup(LONG lScore);

void hiScoreLoad(void);

#endif // _HI_SCORE_H_
