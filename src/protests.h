/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROTESTS_H
#define PROTESTS_H

#include <ace/utils/file.h>

void protestsCreate(void);

void protestsDestroy(void);

void protestsReset(void);

void protestsSave(tFile *pFile);

UBYTE protestsLoad(tFile *pFile);

void protestsProcess(void);

void protestsDrawBobs(void);

#endif // PROTESTS_H
