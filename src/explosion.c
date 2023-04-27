/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "explosion.h"
#include "bob_new.h"
#include "game.h"
#include "mixer/ace_mixer.h"

#include <ace/managers/ptplayer.h>

#define EXPLOSION_MAX 6
#define EXPLOSION_COUNTER_MAX 4
#define EXPLOSION_FRAME_HEIGHT 32
#define EXPLOSION_FRAME_PEAK 5
#define EXPLOSION_FRAME_COUNT 10

#define SFX_CHANNEL_BOOM 1
#define SFX_PRIORITY_BOOM 5

typedef struct tExplosion {
	tBobNew sBob;
	tCbOnPeak cbOnPeak;
	tExplosionKind eKind;
	ULONG ulCbData;
	UBYTE ubFrame;
	UBYTE ubCnt;
	UBYTE isQuick;
} tExplosion;

static tExplosion s_pExplosions[EXPLOSION_MAX];
static tExplosion *s_pExplosionNext = 0;

static tBitMap *s_pBoomFrames, *s_pBoomFramesMask;
static tBitMap *s_pTpFrames, *s_pTpFramesMask;
static tPtplayerSfx *s_pSfxBoom, *s_pSfxTeleport;

static UWORD s_uwTeleportFrameCntMax;

void explosionManagerCreate(void) {
	s_pBoomFrames = bitmapCreateFromFile("data/explosion.bm", 0);
	s_pBoomFramesMask = bitmapCreateFromFile("data/explosion_mask.bm", 0);
	s_pTpFrames = bitmapCreateFromFile("data/teleport.bm", 0);
	s_pTpFramesMask = bitmapCreateFromFile("data/teleport_mask.bm", 0);

	s_pSfxBoom = ptplayerSfxCreateFromFile("data/sfx/explosion.sfx", 1);
	s_pSfxTeleport = ptplayerSfxCreateFromFile("data/sfx/teleport.sfx", 1);

	s_pExplosionNext = &s_pExplosions[0];
	for(UBYTE i = 0; i < EXPLOSION_MAX; ++i) {
		s_pExplosions[i].ubFrame = EXPLOSION_FRAME_COUNT;
		s_pExplosions[i].ubCnt = 0;

		bobNewInit(
			&s_pExplosions[i].sBob, EXPLOSION_FRAME_HEIGHT, EXPLOSION_FRAME_HEIGHT, 1,
			bobNewCalcFrameAddress(s_pBoomFrames, 0),
			bobNewCalcFrameAddress(s_pBoomFramesMask, 0),
			0, 0
		);
	}
	s_uwTeleportFrameCntMax = (
		ptplayerSfxLengthInFrames(s_pSfxTeleport) + EXPLOSION_FRAME_COUNT - 1
	) / (EXPLOSION_FRAME_COUNT * 2);

}

void explosionManagerDestroy(void) {
	bitmapDestroy(s_pBoomFrames);
	bitmapDestroy(s_pBoomFramesMask);
	bitmapDestroy(s_pTpFrames);
	bitmapDestroy(s_pTpFramesMask);

	ptplayerSfxDestroy(s_pSfxBoom);
	ptplayerSfxDestroy(s_pSfxTeleport);
}

void explosionAdd(
	UWORD uwX, UWORD uwY, tCbOnPeak cbOnPeak, ULONG ulCbData,
	UBYTE isQuick, tExplosionKind eKind
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
	s_pExplosionNext->eKind = eKind;
	if(eKind == EXPLOSION_KIND_TELEPORT) {
		bobNewSetFrame(
			&s_pExplosionNext->sBob,
			bobNewCalcFrameAddress(s_pTpFrames, 0),
			bobNewCalcFrameAddress(s_pTpFramesMask, 0)
		);
	}
	else {
		bobNewSetFrame(
			&s_pExplosionNext->sBob,
			bobNewCalcFrameAddress(s_pBoomFrames, 0),
			bobNewCalcFrameAddress(s_pBoomFramesMask, 0)
		);
	}
	audioMixerPlaySfx(
		eKind == EXPLOSION_KIND_TELEPORT ?
			s_pSfxTeleport : s_pSfxBoom,
		SFX_CHANNEL_BOOM, SFX_PRIORITY_BOOM, 0
	);
}

void explosionManagerProcess(void) {
	tExplosion *pExplosion = &s_pExplosions[0];
	for(UBYTE i = 0; i < EXPLOSION_MAX; ++i, ++pExplosion) {
		UBYTE uwCntMax;
		if(pExplosion->eKind == EXPLOSION_KIND_TELEPORT) {
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
				UWORD uwFrameOffsetY = pExplosion->ubFrame * EXPLOSION_FRAME_HEIGHT;
				if(pExplosion->eKind == EXPLOSION_KIND_TELEPORT) {
					bobNewSetFrame(
						&s_pExplosionNext->sBob,
						bobNewCalcFrameAddress(s_pTpFrames, uwFrameOffsetY),
						bobNewCalcFrameAddress(s_pTpFramesMask, uwFrameOffsetY)
					);
				}
				else {
					bobNewSetFrame(
						&s_pExplosionNext->sBob,
						bobNewCalcFrameAddress(s_pBoomFrames, uwFrameOffsetY),
						bobNewCalcFrameAddress(s_pBoomFramesMask, uwFrameOffsetY)
					);
				}
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
