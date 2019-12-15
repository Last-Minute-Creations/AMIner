/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _DINO_H_
#define _DINO_H_

#include <ace/types.h>

void dinoCreate(void);

void dinoDestroy(void);

void dinoReset(void);

void dinoProcess(void);

UBYTE dinoGetBoneCount(void);

void dinoFoundBone(void);

#endif // _DINO_H_
