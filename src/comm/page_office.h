/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_PAGE_OFFICE_H_
#define _COMM_PAGE_OFFICE_H_

#include <comm/base.h>
#include <comm/gs_shop.h>

void pageOfficeReset(void);

void pageOfficeCreate(void);

void pageOfficeUnlockPerson(tCommFace ePerson);

void pageOfficeUnlockPersonSubpage(tCommFace ePerson, tCommShopPage eSubpage);

const tCommShopPage *officeGetPagesForFace(tCommFace eFace);

#endif // _COMM_PAGE_OFFICE_H_
