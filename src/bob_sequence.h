#ifndef _AMINER_BOB_SEQUENCE_H_
#define _AMINER_BOB_SEQUENCE_H_

#include "bob_new.h"

#define BOB_SEQUENCE_NOP 0xFF
#define BOB_SEQUENCE_MAX_LENGTH 200

typedef struct _tBobAnim {
	tBobNew *pBob;
	UBYTE ubFrameIdx;
	UBYTE ubFrameCount;
} tBobAnim;

typedef struct _tBobSequence {
	// Bobs
	tBobAnim *pAnims;
	UBYTE ubMaxBobs; ///< Maximum bobs in sequence
	UBYTE ubBobCount;
	// The sequence itself
	UBYTE pSequence[BOB_SEQUENCE_MAX_LENGTH];
	UBYTE ubSequenceLength; ///< Total sequence length.
	UBYTE ubSeqFrame; ///< Current sequence frame.
	UBYTE ubLastFrame; ///< Frame to draw when current frame is NOP
} tBobSequence;

tBobSequence *bobSequenceCreate(tBobSequence *pSeq, UBYTE ubMaxBobs);

void bobSequenceDestroy(tBobSequence *pSeq);

void bobSequenceAppend(tBobSequence *pSeq, UBYTE ubLength, ...);

void bobSequenceSetSequence(tBobSequence *pSeq, UBYTE ubLength, ...);

UBYTE bobSequenceAddBob(
	tBobSequence *pSeq, tBobNew *pBob, UBYTE ubFrameCount
);

void bobSequenceDraw(tBobSequence *pSeq);


#endif // _AMINER_BOB_SEQUENCE_H_
