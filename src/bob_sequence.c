#include "bob_sequence.h"
#include <stdarg.h>

tBobSequence *bobSequenceCreate(
	tBobSequence *pSeq, UBYTE ubMaxBobs
) {
	pSeq->ubMaxBobs = ubMaxBobs;
	pSeq->pAnims = memAllocFast(ubMaxBobs * sizeof(tBobAnim));
	pSeq->ubBobCount = 0;
	pSeq->ubSequenceLength = 0;
	pSeq->ubSeqFrame= 0;
	pSeq->ubLastFrame = 0;
}

UBYTE bobSequenceAddBob(
	tBobSequence *pSeq, tBobNew *pBob, UBYTE ubFrameCount
) {
	UBYTE ubBobIdx = pSeq->ubBobCount++;
	pSeq->pAnims[ubBobIdx].pBob = pBob;
	pSeq->pAnims[ubBobIdx].ubFrameCount = ubFrameCount;
	return ubBobIdx;
}

static void bobSequenceAppendVa(tBobSequence *pSeq, UBYTE ubLength, va_list vaArgs) {
	for(UBYTE i = 0; i < ubLength; ++i) {
		pSeq->pSequence[i] = va_arg(vaArgs, int);
	}
	pSeq->ubSequenceLength += ubLength;
}

void bobSequenceAppend(tBobSequence *pSeq, UBYTE ubLength, ...) {
	va_list vaArgs;
	va_start(vaArgs, ubLength);
	bobSequenceAppendVa(pSeq, ubLength, vaArgs);
	va_end(vaArgs);
}

void bobSequenceSetSequence(tBobSequence *pSeq, UBYTE ubLength, ...) {
	pSeq->ubSequenceLength = 0;
	va_list vaArgs;
	va_start(vaArgs, ubLength);
	bobSequenceAppendVa(pSeq, ubLength, vaArgs);
	va_end(vaArgs);
}

void bobSequenceDraw(tBobSequence *pSeq) {
	UBYTE ubFrame = pSeq->pSequence[pSeq->ubSeqFrame];
	if(ubFrame == BOB_SEQUENCE_NOP) {
		ubFrame = pSeq->ubLastFrame;
	}
	else {
		// Increment bob's frame
		tBobAnim *pAnim = &pSeq->pAnims[ubFrame];
		bobNewSetBitMapOffset(
			pAnim->pBob, pAnim->pBob->uwHeight * pAnim->ubFrameIdx
		);
		++pAnim->ubFrameIdx;
		if(pAnim->ubFrameIdx >= pAnim->ubFrameCount) {
			pAnim->ubFrameIdx = 0;
		}
		pSeq->ubLastFrame = ubFrame;
	}

	// Push the bob
	bobNewPush(pSeq->pAnims[ubFrame].pBob);

	// Go to next bob in sequence
	++pSeq->ubSeqFrame;
	if(pSeq->ubSeqFrame >= pSeq->ubSequenceLength) {
		pSeq->ubSeqFrame = 0;
	}
}

void bobSequenceDestroy(tBobSequence *pSeq) {
	memFree(pSeq->pAnims, pSeq->ubMaxBobs * sizeof(tBobAnim));
}

