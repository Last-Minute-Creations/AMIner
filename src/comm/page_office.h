/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_PAGE_OFFICE_H_
#define _COMM_PAGE_OFFICE_H_

#include <comm/base.h>

typedef enum _tOfficePage {
	OFFICE_PAGE_MAIN,
	OFFICE_PAGE_LIST_MIETEK,
	OFFICE_PAGE_LIST_KRYSTYNA,
	OFFICE_PAGE_LIST_KOMISARZ,
	OFFICE_PAGE_LIST_URZEDAS,
	OFFICE_PAGE_KRYSTYNA_DOSSIER,
	OFFICE_PAGE_KRYSTYNA_ACCOUNTING,
	OFFICE_PAGE_URZEDAS_DOSSIER,
	OFFICE_PAGE_URZEDAS_BRIBE,
	OFFICE_PAGE_URZEDAS_FAVOR,
	OFFICE_PAGE_KOMISARZ_DOSSIER,
	OFFICE_PAGE_KOMISARZ_WELCOME,
	OFFICE_PAGE_KOMISARZ_REBUKE_1,
	OFFICE_PAGE_KOMISARZ_REBUKE_2,
	OFFICE_PAGE_KOMISARZ_REBUKE_3,
	OFFICE_PAGE_COUNT
} tOfficePage;

void pageOfficeReset(void);

void pageOfficeCreate(void);

void pageOfficeUnlockPerson(tCommFace ePerson);

void pageOfficeUnlockPersonSubpage(tCommFace ePerson, tOfficePage eSubpage);

void pageOfficeOpenSubpage(tOfficePage eCameFrom, tOfficePage eTarget);

void pageOfficeGoBack(void);

#endif // _COMM_PAGE_OFFICE_H_
