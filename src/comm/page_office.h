/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_PAGE_OFFICE_H_
#define _COMM_PAGE_OFFICE_H_

#include <comm/base.h>
#include <comm/gs_shop.h>

void pageOfficeReset(void);

void pageOfficeSave(tFile *pFile);

UBYTE pageOfficeLoad(tFile *pFile);

void pageOfficeShow(void);

void pageOfficeUnlockPerson(tFaceId ePerson);

UBYTE pageOfficeTryUnlockPersonSubpage(tFaceId ePerson, tCommShopPage eSubpage);

const tCommShopPage *officeGetPagesForFace(tFaceId eFace);

#endif // _COMM_PAGE_OFFICE_H_
