/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <bob_new.h>
#include <ace/managers/memory.h>
#include <ace/managers/system.h>
#include <ace/managers/blit.h>
#include <ace/utils/custom.h>

// Undraw stack must be accessible during adding new bobs, so the most safe
// approach is to have two lists - undraw list gets populated after draw
// and depopulated during undraw
typedef struct tBobQueue {
	UBYTE ubUndrawCount;
	tBobNew **pBobs;
	tBitMap *pBg;
	tBitMap *pDst;
} tBobQueue;

static UBYTE s_ubBufferCurr;
static UBYTE s_ubMaxBobCount;

static UBYTE s_isPushingDone;
static UBYTE s_ubBpp;

// This can't be a decreasing counter such as in toSave/toDraw since after
// decrease another bob may be pushed, which would trash bg saving
static UBYTE s_ubBobsPushed;
static UBYTE s_ubBobsDrawn;
static UBYTE s_ubBobsSaved;
static UWORD s_uwAvailHeight;
static UWORD s_uwBgBufferLength;

tBobQueue s_pQueues[2];

static void bobNewDeallocBuffers(void) {
	blitWait();
	systemUse();
	if(s_pQueues[0].pBobs && s_ubMaxBobCount) {
		memFree(s_pQueues[0].pBobs, sizeof(tBobNew*) * s_ubMaxBobCount);
		s_pQueues[0].pBobs = 0;
	}
	if(s_pQueues[1].pBobs && s_ubMaxBobCount) {
		memFree(s_pQueues[1].pBobs, sizeof(tBobNew*) * s_ubMaxBobCount);
		s_pQueues[1].pBobs = 0;
	}
	s_ubMaxBobCount = 0;
	if(s_pQueues[0].pBg) {
		bitmapDestroy(s_pQueues[0].pBg);
		s_pQueues[0].pBg = 0;
	}
	if(s_pQueues[1].pBg) {
		bitmapDestroy(s_pQueues[1].pBg);
		s_pQueues[1].pBg = 0;
	}
	systemUnuse();
}

void bobNewManagerReset(void) {
	bobNewDeallocBuffers();

	// Don't reset s_ubBufferCurr - we still may need to keep track which buffer
	// is in the front and which in the back
	// E.g. multiple states in single buffer manager:
	// fade-out, reset bobs, fade-in, start display

	s_uwBgBufferLength = 0;
	s_isPushingDone = 0;
	s_ubBobsPushed = 0;
	s_ubBobsSaved = 0;
	s_ubBobsDrawn = 0;
	bobNewDiscardUndraw();
}

void bobNewManagerCreate(
	tBitMap *pFront, tBitMap *pBack, UWORD uwAvailHeight
) {
	logBlockBegin(
		"bobNewManagerCreate(pFront: %p, pBack: %p, uwAvailHeight: %hu)",
		pFront, pBack, uwAvailHeight
	);
	s_ubBpp = pFront->Depth;
	s_pQueues[0].pDst = pBack;
	s_pQueues[1].pDst = pFront;

	s_pQueues[0].pBg = 0;
	s_pQueues[1].pBg = 0;
	s_ubMaxBobCount = 0;
	bobNewManagerReset();
	s_ubBufferCurr = 0;
	s_uwAvailHeight = uwAvailHeight;

	s_pQueues[0].pBg = 0;
	s_pQueues[1].pBg = 0;
	s_pQueues[0].pBobs = 0;
	s_pQueues[1].pBobs = 0;
	logBlockEnd("bobNewManagerCreate()");
}

void bobNewReallocateBgBuffers(void) {
	systemUse();
	logBlockBegin("bobNewReallocateBgBuffers()");
	s_pQueues[0].pBobs = memAllocFast(sizeof(tBobNew*) * s_ubMaxBobCount);
	s_pQueues[1].pBobs = memAllocFast(sizeof(tBobNew*) * s_ubMaxBobCount);
	s_pQueues[0].pBg = bitmapCreate(16, s_uwBgBufferLength, s_ubBpp, BMF_INTERLEAVED);
	s_pQueues[1].pBg = bitmapCreate(16, s_uwBgBufferLength, s_ubBpp, BMF_INTERLEAVED);
	logWrite(
		"New bg buffer length: %hu, max bobs: %hhu\n",
		s_uwBgBufferLength, s_ubMaxBobCount
	);
	logBlockEnd("bobNewReallocateBgBuffers()");
	systemUnuse();
}

void bobNewManagerDestroy(void) {
	bobNewDeallocBuffers();
}

void bobNewPush(tBobNew *pBob) {
	tBobQueue *pQueue = &s_pQueues[s_ubBufferCurr];
	pQueue->pBobs[s_ubBobsPushed] = pBob;
	++s_ubBobsPushed;
	if(blitIsIdle()) {
		bobNewProcessNext();
	}
}

void bobNewInit(
	tBobNew *pBob, UWORD uwWidth, UWORD uwHeight, UBYTE isUndrawRequired,
	UBYTE *pFrameData, UBYTE *pMaskData, UWORD uwX, UWORD uwY
) {
	pBob->uwWidth = uwWidth;
	pBob->uwHeight = uwHeight;
	pBob->isUndrawRequired = isUndrawRequired;
	UWORD uwBlitWords = (uwWidth+15) / 16 + 1; // One word more for aligned copy
	pBob->_uwBlitSize = ((uwHeight*s_ubBpp) << 6) | uwBlitWords;
	pBob->_wModuloUndrawSave = bitmapGetByteWidth(s_pQueues[0].pDst) - uwBlitWords*2;
	bobNewSetFrame(pBob, pFrameData, pMaskData);

	pBob->sPos.uwX = uwX;
	pBob->sPos.uwY = uwY;
	pBob->pOldPositions[0].uwX = uwX;
	pBob->pOldPositions[0].uwY = uwY;
	pBob->pOldPositions[1].uwX = uwX;
	pBob->pOldPositions[1].uwY = uwY;

	s_uwBgBufferLength += uwBlitWords * uwHeight * s_ubBpp;
	++s_ubMaxBobCount;
	// logWrite("Added bob, now max: %hhu\n", s_ubMaxBobCount);
}

void bobNewSetFrame(tBobNew *pBob, UBYTE *pFrameData, UBYTE *pMaskData) {
	pBob->pFrameData = pFrameData;
	pBob->pMaskData = pMaskData;
}

UBYTE *bobNewCalcFrameAddress(tBitMap *pBitmap, UWORD uwOffsetY) {
	return &pBitmap->Planes[0][pBitmap->BytesPerRow * uwOffsetY];
}

UBYTE bobNewProcessNext(void) {
	if(s_ubBobsSaved < s_ubBobsPushed) {
		tBobQueue *pQueue = &s_pQueues[s_ubBufferCurr];
		if(!s_ubBobsSaved) {
			// Prepare for saving.
			// Bltcon0/1, bltaxwm could be reset between Begin and ProcessNext.
			// I tried to change A->D to C->D bug afwm/alwm need to be set
			// for mask-copying bobs, so there's no perf to be gained.
			UWORD uwBltCon0 = USEA|USED | MINTERM_A;
			g_pCustom->bltcon0 = uwBltCon0;
			g_pCustom->bltcon1 = 0;
			g_pCustom->bltafwm = 0xFFFF;
			g_pCustom->bltalwm = 0xFFFF;

			g_pCustom->bltdmod = 0;
			g_pCustom->bltdpt = pQueue->pBg->Planes[0];
		}
		const tBobNew *pBob = pQueue->pBobs[s_ubBobsSaved];
		++s_ubBobsSaved;
		if(pBob->isUndrawRequired) {
			ULONG ulSrcOffs = (
				pQueue->pDst->BytesPerRow * (pBob->sPos.uwY & (s_uwAvailHeight-1)) +
				pBob->sPos.uwX / 8
			);
			UBYTE *pA = &pQueue->pDst->Planes[0][ulSrcOffs];
			g_pCustom->bltamod = pBob->_wModuloUndrawSave;
			g_pCustom->bltapt = (APTR)pA;
			UWORD uwPartHeight = s_uwAvailHeight - (pBob->sPos.uwY & (s_uwAvailHeight-1));
			if(uwPartHeight >= pBob->uwHeight) {
				g_pCustom->bltsize = pBob->_uwBlitSize;
			}
			else {
				UWORD uwBlitWords = (pBob->uwWidth+15) / 16 + 1;
				g_pCustom->bltsize =((uwPartHeight * s_ubBpp) << 6) | uwBlitWords;
				pA = &pQueue->pDst->Planes[0][pBob->sPos.uwX / 8];
				blitWait();
				g_pCustom->bltapt = pA;
				g_pCustom->bltsize =(((pBob->uwHeight - uwPartHeight) * s_ubBpp) << 6) | uwBlitWords;
			}
		}
		return 1;
	}
	else {
		if(!s_isPushingDone) {
			return 1;
		}

		tBobQueue *pQueue = &s_pQueues[s_ubBufferCurr];
		if(s_ubBobsDrawn < s_ubBobsPushed) {
			// Draw next
			tBobNew *pBob = pQueue->pBobs[s_ubBobsDrawn];
			const tUwCoordYX * pPos = &pBob->sPos;
			++s_ubBobsDrawn;
			UBYTE ubDstOffs = pPos->uwX & 0xF;
			UWORD uwBlitWidth = (pBob->uwWidth + ubDstOffs + 15) & 0xFFF0;
			UWORD uwBlitWords = uwBlitWidth >> 4;
			UWORD uwBlitSize = ((pBob->uwHeight * s_ubBpp) << 6) | uwBlitWords;
			WORD wSrcModulo = (pBob->uwWidth >> 3) - (uwBlitWords<<1);
			UWORD uwBltCon1 = ubDstOffs << BSHIFTSHIFT;
			UWORD uwBltCon0;
			if(pBob->pMaskData) {
				uwBltCon0 = uwBltCon1 | USEA|USEB|USEC|USED | MINTERM_COOKIE;
			}
			else {
				// TODO change to A - performance boost
				// TODO setting B & C regs isn't necessary - few write cycles less
				uwBltCon0 = uwBltCon1 | USEB|USED | MINTERM_B;
			}
			ULONG ulDstOffs = (
				pQueue->pDst->BytesPerRow * (pPos->uwY & (s_uwAvailHeight-1)) + pPos->uwX / 8
			);

			WORD wDstModulo = bitmapGetByteWidth(pQueue->pDst) - (uwBlitWords<<1);
			UBYTE *pB = pBob->pFrameData;
			UBYTE *pCD = &pQueue->pDst->Planes[0][ulDstOffs];

			g_pCustom->bltcon0 = uwBltCon0;
			g_pCustom->bltcon1 = uwBltCon1;

			if(pBob->pMaskData) {
				UWORD uwLastMask = 0xFFFF << (uwBlitWidth-pBob->uwWidth);
				UBYTE *pA = pBob->pMaskData;
				g_pCustom->bltalwm = uwLastMask;
				g_pCustom->bltamod = wSrcModulo;
				g_pCustom->bltapt = (APTR)pA;
			}

			g_pCustom->bltbmod = wSrcModulo;
			g_pCustom->bltcmod = wDstModulo;
			g_pCustom->bltdmod = wDstModulo;

			g_pCustom->bltbpt = (APTR)pB;
			g_pCustom->bltcpt = (APTR)pCD;
			g_pCustom->bltdpt = (APTR)pCD;
			UWORD uwPartHeight = s_uwAvailHeight - (pBob->sPos.uwY & (s_uwAvailHeight-1));
			if(uwPartHeight >= pBob->uwHeight) {
				g_pCustom->bltsize = uwBlitSize;
			}
			else {
				g_pCustom->bltsize =((uwPartHeight * s_ubBpp) << 6) | uwBlitWords;
				pCD = &pQueue->pDst->Planes[0][pBob->sPos.uwX / 8];
				blitWait();
				g_pCustom->bltcpt = (APTR)pCD;
				g_pCustom->bltdpt = (APTR)pCD;
				g_pCustom->bltsize =(((pBob->uwHeight - uwPartHeight) * s_ubBpp) << 6) | uwBlitWords;
			}

			pBob->pOldPositions[s_ubBufferCurr].ulYX = pPos->ulYX;

			return 1;
		}
	}
	return 0;
}

static void bobNewCheckGood(const tBitMap *pBack) {
#if defined(ACE_DEBUG)
	if(s_pQueues[s_ubBufferCurr].pDst != pBack) {
		logWrite(
			"ERR: bobNew manager operates on wrong buffer! Proper current: %p (%hhu), Other: %p, Arg: %p\n",
			s_pQueues[s_ubBufferCurr].pDst, s_ubBufferCurr, s_pQueues[!s_ubBufferCurr].pDst, pBack
		);
		if(s_pQueues[!s_ubBufferCurr].pDst == pBack) {
			logWrite("ERR: Wrong bob buffer as curr!\n");
			s_ubBufferCurr = !s_ubBufferCurr;
		}
	}
#endif
}

void bobNewBegin(tBitMap *pBuffer) {
	bobNewCheckGood(pBuffer);
	tBobQueue *pQueue = &s_pQueues[s_ubBufferCurr];

	// Prepare for undraw
	blitWait();
	UWORD uwBltCon0 = USEA|USED | MINTERM_A;
	g_pCustom->bltcon0 = uwBltCon0;
	g_pCustom->bltcon1 = 0;
	g_pCustom->bltafwm = 0xFFFF;
	g_pCustom->bltalwm = 0xFFFF;
	g_pCustom->bltamod = 0;
	UBYTE *pA = pQueue->pBg->Planes[0];
	g_pCustom->bltapt = pA;
#ifdef GAME_DEBUG
	UWORD uwDrawnHeight = 0;
#endif

	for(UBYTE i = 0; i < pQueue->ubUndrawCount; ++i) {
		const tBobNew *pBob = pQueue->pBobs[i];
		if(pBob->isUndrawRequired) {
			// Undraw next
			ULONG ulDstOffs = (
				pQueue->pDst->BytesPerRow * (pBob->pOldPositions[s_ubBufferCurr].uwY & (s_uwAvailHeight-1)) +
				pBob->pOldPositions[s_ubBufferCurr].uwX / 8
			);
			UBYTE *pD = &pQueue->pDst->Planes[0][ulDstOffs];
			blitWait();
			g_pCustom->bltdmod = pBob->_wModuloUndrawSave;
			g_pCustom->bltdpt = (APTR)pD;
			UWORD uwPartHeight = s_uwAvailHeight - (pBob->pOldPositions[s_ubBufferCurr].uwY & (s_uwAvailHeight-1));
			if(uwPartHeight >= pBob->uwHeight) {
				g_pCustom->bltsize = pBob->_uwBlitSize;
			}
			else {
				UWORD uwBlitWords = (pBob->uwWidth+15) / 16 + 1;
				g_pCustom->bltsize =((uwPartHeight * s_ubBpp) << 6) | uwBlitWords;
				pD = &pQueue->pDst->Planes[0][pBob->pOldPositions[s_ubBufferCurr].uwX / 8];
				blitWait();
				g_pCustom->bltdpt = pD;
				g_pCustom->bltsize =(((pBob->uwHeight - uwPartHeight) * s_ubBpp) << 6) | uwBlitWords;
			}

#ifdef GAME_DEBUG
			UWORD uwBlitWords = (pBob->uwWidth+15) / 16 + 1;
			uwDrawnHeight += uwBlitWords * pBob->uwHeight;
#endif
		}
	}
#ifdef GAME_DEBUG
	UWORD uwDrawLimit = s_pQueues[0].pBg->Rows * s_pQueues[0].pBg->Depth;
	if(uwDrawnHeight > uwDrawLimit) {
		logWrite(
			"ERR: BG restore out of bounds: used %hu, limit: %hu",
			uwDrawnHeight, uwDrawLimit
		);
	}
#endif
	s_ubBobsSaved = 0;
	s_ubBobsDrawn = 0;
	s_ubBobsPushed = 0;
	s_isPushingDone = 0;
}

void bobNewPushingDone(void) {
	s_isPushingDone = 1;
}

void bobNewEnd(void) {
	bobNewPushingDone();
	do {
		blitWait();
	} while(bobNewProcessNext());
	s_pQueues[s_ubBufferCurr].ubUndrawCount = s_ubBobsPushed;
	s_ubBufferCurr = !s_ubBufferCurr;
}

void bobNewDiscardUndraw(void) {
	s_pQueues[0].ubUndrawCount = 0;
	s_pQueues[1].ubUndrawCount = 0;
}
