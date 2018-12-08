/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _VENDOR_H_
#define _VENDOR_H_

#include <ace/types.h>

void vendorGsCreate(void);

void vendorGsLoop(void);

void vendorGsDestroy(void);

void vendorAlloc(void);

void vendorDealloc(void);

#endif // _VENDOR_H_
