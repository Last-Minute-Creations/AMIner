/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "explosion.h"
#include "bob_new.h"

#define EXPLOSION_MAX 4
#define EXPLOSION_FRAME_MAX 10
#define EXPLOSION_COUNTER_MAX 4
#define EXPLOSION_FRAME_HEIGHT 32

typedef struct tExplosion {
	UBYTE ubFrame;
	UBYTE ubCnt;
	tBobNew sBob;
} tExplosion;

static tExplosion s_pExplosions[EXPLOSION_MAX];
static tExplosion *s_pExplosionNext = 0;

static tBitMap *s_pFrames;
static tBitMap *s_pFramesMask;

void explosionManagerCreate(void) {
	s_pFrames = bitmapCreateFromFile("data/explosion.bm", 0);
	s_pFramesMask = bitmapCreateFromFile("data/explosion_mask.bm", 0);

	s_pExplosionNext = &s_pExplosions[0];
	for(UBYTE i = 0; i < EXPLOSION_MAX; ++i) {
		s_pExplosions[i].ubFrame = EXPLOSION_FRAME_MAX;
		s_pExplosions[i].ubCnt = 0;

		bobNewInit(
			&s_pExplosions[i].sBob, EXPLOSION_FRAME_HEIGHT, EXPLOSION_FRAME_HEIGHT, 1,
			s_pFrames, s_pFramesMask, 0, 0
		);
	}

}

void explosionManagerDestroy(void) {
	bitmapDestroy(s_pFrames);
	bitmapDestroy(s_pFramesMask);
}

void explosionAdd(UWORD uwX, UWORD uwY, tCbOnPeak cbOnPeak) {
	tExplosion *pStart = s_pExplosionNext;
	do {
		if(s_pExplosionNext->ubFrame >= EXPLOSION_FRAME_MAX) {
			s_pExplosionNext->sBob.sPos.uwX = uwX;
			s_pExplosionNext->sBob.sPos.uwY = uwY;
			s_pExplosionNext->ubFrame = 0;
			bobNewSetBitMapOffset(&s_pExplosionNext->sBob, 0);
			break;
		}
		if(++s_pExplosionNext >= &s_pExplosions[EXPLOSION_MAX]) {
			s_pExplosionNext = &s_pExplosions[0];
		}
	} while(s_pExplosionNext != pStart);
}

void explosionManagerProcess(void) {
	tExplosion *pExplosion = &s_pExplosions[0];
	for(UBYTE i = 0; i < EXPLOSION_MAX; ++i, ++pExplosion) {
		if(pExplosion->ubFrame < EXPLOSION_FRAME_MAX) {
			if(pExplosion->ubCnt >= EXPLOSION_COUNTER_MAX) {
				pExplosion->ubCnt = 0;
				++pExplosion->ubFrame;
				bobNewSetBitMapOffset(
					&pExplosion->sBob, pExplosion->ubFrame * EXPLOSION_FRAME_HEIGHT
				);
			}
			else {
				++pExplosion->ubCnt;
			}
			if(pExplosion->ubFrame < EXPLOSION_FRAME_MAX) {
				bobNewPush(&pExplosion->sBob);
			}
		}
	}
}
