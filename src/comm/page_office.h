/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_PAGE_OFFICE_H_
#define _COMM_PAGE_OFFICE_H_

typedef enum _tOfficePage {
	OFFICE_PAGE_MAIN,
	OFFICE_PAGE_LIST_MIETEK,
	OFFICE_PAGE_LIST_KRYSTYNA,
	OFFICE_PAGE_LIST_PUTIN,
	OFFICE_PAGE_LIST_URZEDAS,
	OFFICE_PAGE_DOSSIER_KRYSTYNA,
	OFFICE_PAGE_DOSSIER_URZEDAS,
	OFFICE_PAGE_BRIBE,
	OFFICE_PAGE_FAVOR,
	OFFICE_PAGE_ACCOUNTING,
	OFFICE_PAGE_COUNT
} tOfficePage;

void pageOfficeReset(void);

void pageOfficeCreate(void);

#endif // _COMM_PAGE_OFFICE_H_
