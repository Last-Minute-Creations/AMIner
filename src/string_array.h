/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _STRING_ARRAY_H_
#define _STRING_ARRAY_H_

#include <ace/utils/file.h>

/**
 * Array of string pointers terimnated with nullptr
 */

#define STRING_ARRAY_EMPTY_POS ((char*)-1)
#define STRING_ARRAY_TERMINATOR ((char*)0)

char **stringArrayCreateFromFd(tFile *pFile);

void stringArrayDestroy(char **pArray);

UWORD stringArrayGetCount(const char * const *pArray);

#endif // _STRING_ARRAY_H_
