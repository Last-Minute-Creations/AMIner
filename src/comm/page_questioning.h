/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _AMINER_COMM_PAGE_QUESTIONING_H_
#define _AMINER_COMM_PAGE_QUESTIONING_H_

#include <ace/utils/file.h>
#include <ace/macros.h>

typedef enum tQuestioningBit {
	QUESTIONING_BIT_NONE = 0,
	QUESTIONING_BIT_GATE = BV(0),
	QUESTIONING_BIT_TELEPORT_PARTS = BV(1),
	QUESTIONING_BIT_END ///< Not really a bit, used for iterating
} tQuestioningBit;

typedef void (*tQuestioningHandler)(tQuestioningBit eQuestioningBit, UBYTE isReported);

void pageQuestioningCreate(void);

void pageQuestioningReset(void);

void pageQuestioningSave(tFile *pFile);

UBYTE pageQuestioningLoad(tFile *pFile);

void pageQuestioningTrySetPendingQuestioning(tQuestioningBit eQuestioningBit);

void pageQuestioningTryCancelPendingQuestioning(tQuestioningBit eQuestioningBit);

UBYTE pageQuestioningIsReported(tQuestioningBit eQuestioning);

void pageQuestioningSetHandler(tQuestioningBit eQuestioningBit, tQuestioningHandler cbOnQuestioningEnded);

#endif // _AMINER_COMM_PAGE_QUESTIONING_H_
