/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CORE_H_
#define _CORE_H_

#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/font.h>

void coreProcessBeforeBobs(void);

void coreProcessAfterBobs(void);

void coreGsCreate(void);

void coreGsLoop(void);

void coreGsDestroy(void);

void coreSetLangPrefix(const char * const szPrefix);

const char * coreGetLangPrefix(void);

extern tTileBufferManager *g_pMainBuffer;
extern tFont *g_pFont;

#endif // _CORE_H_
