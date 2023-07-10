/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SAVE_H
#define SAVE_H

#include <ace/utils/file.h>

UBYTE saveReadHeader(tFile *pFile, const char *szHeader);

void saveWriteHeader(tFile *pFile, const char *szHeader);

#endif // SAVE_H
