/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "explosion.h"
#include "bob_new.h"
#include "game.h"

#include <ace/managers/audio.h>

#define EXPLOSION_MAX 6
#define EXPLOSION_COUNTER_MAX 4
#define EXPLOSION_FRAME_HEIGHT 32
#define EXPLOSION_FRAME_PEAK 5
#define EXPLOSION_FRAME_COUNT 10

#define EXPLOSION_AUDIO_CHANNEL AUDIO_CHANNEL_1

typedef struct tExplosion {
	tBobNew sBob;
	tCbOnPeak cbOnPeak;
	ULONG ulCbData;
	UBYTE ubFrame;
	UBYTE ubCnt;
	UBYTE isQuick;
	UBYTE isTeleport;
} tExplosion;

static tExplosion s_pExplosions[EXPLOSION_MAX];
static tExplosion *s_pExplosionNext = 0;

static tBitMap *s_pBoomFrames, *s_pBoomFramesMask;
static tBitMap *s_pTpFrames, *s_pTpFramesMask;
static tSample *s_pSampleBoom, *s_pSampleTeleport;

static UWORD s_uwTeleportFrameCntMax;

UBYTE audioGetSampleLengthInFrames(tSample *pSample) {
	UWORD uwSamplingRateHz = (3546895 + (pSample->uwPeriod / 2)) / pSample->uwPeriod;
	UWORD uwFrameCount = (pSample->uwLength * 50 + uwSamplingRateHz - 1) / uwSamplingRateHz;
	return uwFrameCount;
}

void explosionManagerCreate(void) {
	s_pBoomFrames = bitmapCreateFromFile("data/explosion.bm", 0);
	s_pBoomFramesMask = bitmapCreateFromFile("data/explosion_mask.bm", 0);
	s_pTpFrames = bitmapCreateFromFile("data/teleport.bm", 0);
	s_pTpFramesMask = bitmapCreateFromFile("data/teleport_mask.bm", 0);

	s_pSampleBoom = sampleCreateFromFile("data/sfx/explosion.raw8", 22050);
	s_pSampleTeleport = sampleCreateFromFile("data/sfx/teleport.raw8", 22050);

	s_pExplosionNext = &s_pExplosions[0];
	for(UBYTE i = 0; i < EXPLOSION_MAX; ++i) {
		s_pExplosions[i].ubFrame = EXPLOSION_FRAME_COUNT;
		s_pExplosions[i].ubCnt = 0;

		bobNewInit(
			&s_pExplosions[i].sBob, EXPLOSION_FRAME_HEIGHT, EXPLOSION_FRAME_HEIGHT, 1,
			s_pBoomFrames, s_pBoomFramesMask, 0, 0
		);
	}
	s_uwTeleportFrameCntMax = (
		audioGetSampleLengthInFrames(s_pSampleTeleport) + EXPLOSION_FRAME_COUNT - 1
	) / (EXPLOSION_FRAME_COUNT * 2);

}

void explosionManagerDestroy(void) {
	bitmapDestroy(s_pBoomFrames);
	bitmapDestroy(s_pBoomFramesMask);
	bitmapDestroy(s_pTpFrames);
	bitmapDestroy(s_pTpFramesMask);

	sampleDestroy(s_pSampleBoom);
	sampleDestroy(s_pSampleTeleport);
}

void explosionAdd(
	UWORD uwX, UWORD uwY, tCbOnPeak cbOnPeak, ULONG ulCbData,
	UBYTE isQuick, UBYTE isTeleport
) {
	tExplosion *pStart = s_pExplosionNext;
	do {
		if(s_pExplosionNext->ubFrame >= EXPLOSION_FRAME_COUNT) {
			break;
		}
		if(++s_pExplosionNext >= &s_pExplosions[EXPLOSION_MAX]) {
			s_pExplosionNext = &s_pExplosions[0];
		}
	} while(s_pExplosionNext != pStart);

	// If there's free slot, s_pExplosionNext will point to it
	// Otherwise, it will use oldest - the one from pStart

	if(
		s_pExplosionNext == pStart &&
		s_pExplosionNext->ubFrame <= EXPLOSION_FRAME_PEAK &&
		s_pExplosionNext->cbOnPeak
	) {
		// Call pStart's callback if it's before peak
		s_pExplosionNext->cbOnPeak(s_pExplosionNext->ulCbData);
	}

	s_pExplosionNext->sBob.sPos.uwX = uwX;
	s_pExplosionNext->sBob.sPos.uwY = uwY;
	s_pExplosionNext->ubFrame = 0;
	s_pExplosionNext->cbOnPeak = cbOnPeak;
	s_pExplosionNext->ulCbData = ulCbData;
	s_pExplosionNext->isQuick = isQuick;
	s_pExplosionNext->isTeleport = isTeleport;
	if(isTeleport) {
		s_pExplosionNext->sBob.pBitmap = s_pTpFrames;
		s_pExplosionNext->sBob.pMask = s_pTpFramesMask;
	}
	else {
		s_pExplosionNext->sBob.pBitmap = s_pBoomFrames;
		s_pExplosionNext->sBob.pMask = s_pBoomFramesMask;
	}
	bobNewSetBitMapOffset(&s_pExplosionNext->sBob, 0);
	audioPlay(
		EXPLOSION_AUDIO_CHANNEL, isTeleport ? s_pSampleTeleport : s_pSampleBoom,
		AUDIO_VOLUME_MAX, 1
	);
}

void explosionManagerProcess(void) {
	tExplosion *pExplosion = &s_pExplosions[0];
	for(UBYTE i = 0; i < EXPLOSION_MAX; ++i, ++pExplosion) {
		UBYTE uwCntMax;
		if(pExplosion->isTeleport) {
			uwCntMax = s_uwTeleportFrameCntMax;
		}
		else {
			uwCntMax = EXPLOSION_COUNTER_MAX >> pExplosion->isQuick;
		}
		if(pExplosion->ubFrame < EXPLOSION_FRAME_COUNT) {
			if(pExplosion->ubCnt >= uwCntMax) {
				pExplosion->ubCnt = 0;
				++pExplosion->ubFrame;
				if(pExplosion->ubFrame == EXPLOSION_FRAME_PEAK && pExplosion->cbOnPeak) {
					pExplosion->cbOnPeak(pExplosion->ulCbData);
				}
				bobNewSetBitMapOffset(
					&pExplosion->sBob, pExplosion->ubFrame * EXPLOSION_FRAME_HEIGHT
				);
			}
			else {
				++pExplosion->ubCnt;
			}
			if(pExplosion->ubFrame < EXPLOSION_FRAME_COUNT) {
				gameTryPushBob(&pExplosion->sBob);
			}
		}
	}
}
