/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "flipbook.h"
#include <ace/managers/bob.h>
#include "game.h"
#include <ace/managers/ptplayer.h>
#include <ace/contrib/managers/audio_mixer.h>

#define FLIPBBOK_MAX 6
#define FLIPBBOK_COUNTER_MAX 4
#define FLIPBBOK_FRAME_HEIGHT 32
#define FLIPBBOK_FRAME_PEAK 5
#define FLIPBBOK_FRAME_COUNT 10

#define SFX_CHANNEL_BOOM 1
#define SFX_PRIORITY_BOOM 5

typedef struct tFlipbook {
	tBob sBob;
	tCbOnPeak cbOnPeak;
	tFlipbookKind eKind;
	ULONG ulCbData;
	UBYTE ubFrame;
	UBYTE ubCnt;
	UBYTE isQuick;
} tFlipbook;

static tFlipbook s_pFlipbooks[FLIPBBOK_MAX];
static tFlipbook *s_pFlipbookNext = 0;

static tBitMap *s_pBoomFrames, *s_pBoomFramesMask;
static tBitMap *s_pTpFrames, *s_pTpFramesMask;
static tPtplayerSfx *s_pSfxBoom, *s_pSfxTeleport;

static UWORD s_uwTeleportFrameCntMax;

void flipbookManagerCreate(void) {
	s_pBoomFrames = bitmapCreateFromFile("data/explosion.bm", 0);
	s_pBoomFramesMask = bitmapCreateFromFile("data/FLIPBBOK_mask.bm", 0);
	s_pTpFrames = bitmapCreateFromFile("data/teleport.bm", 0);
	s_pTpFramesMask = bitmapCreateFromFile("data/teleport_mask.bm", 0);

	s_pSfxBoom = ptplayerSfxCreateFromFile("data/sfx/explosion.sfx", 1);
	s_pSfxTeleport = ptplayerSfxCreateFromFile("data/sfx/teleport.sfx", 1);

	s_pFlipbookNext = &s_pFlipbooks[0];
	for(UBYTE i = 0; i < FLIPBBOK_MAX; ++i) {
		s_pFlipbooks[i].ubFrame = FLIPBBOK_FRAME_COUNT;
		s_pFlipbooks[i].ubCnt = 0;

		bobInit(
			&s_pFlipbooks[i].sBob, FLIPBBOK_FRAME_HEIGHT, FLIPBBOK_FRAME_HEIGHT, 1,
			bobCalcFrameAddress(s_pBoomFrames, 0),
			bobCalcFrameAddress(s_pBoomFramesMask, 0),
			0, 0
		);
	}
	s_uwTeleportFrameCntMax = (
		ptplayerSfxLengthInFrames(s_pSfxTeleport) + FLIPBBOK_FRAME_COUNT - 1
	) / (FLIPBBOK_FRAME_COUNT * 2);

}

void flipbookManagerDestroy(void) {
	bitmapDestroy(s_pBoomFrames);
	bitmapDestroy(s_pBoomFramesMask);
	bitmapDestroy(s_pTpFrames);
	bitmapDestroy(s_pTpFramesMask);

	ptplayerSfxDestroy(s_pSfxBoom);
	ptplayerSfxDestroy(s_pSfxTeleport);
}

void flipbookAdd(
	UWORD uwX, UWORD uwY, tCbOnPeak cbOnPeak, ULONG ulCbData,
	UBYTE isQuick, tFlipbookKind eKind
) {
	tFlipbook *pStart = s_pFlipbookNext;
	do {
		if(s_pFlipbookNext->ubFrame >= FLIPBBOK_FRAME_COUNT) {
			break;
		}
		if(++s_pFlipbookNext >= &s_pFlipbooks[FLIPBBOK_MAX]) {
			s_pFlipbookNext = &s_pFlipbooks[0];
		}
	} while(s_pFlipbookNext != pStart);

	// If there's free slot, s_pFlipbookNext will point to it
	// Otherwise, it will use oldest - the one from pStart

	if(
		s_pFlipbookNext == pStart &&
		s_pFlipbookNext->ubFrame <= FLIPBBOK_FRAME_PEAK &&
		s_pFlipbookNext->cbOnPeak
	) {
		// Call pStart's callback if it's before peak
		s_pFlipbookNext->cbOnPeak(s_pFlipbookNext->ulCbData);
	}

	s_pFlipbookNext->sBob.sPos.uwX = uwX;
	s_pFlipbookNext->sBob.sPos.uwY = uwY;
	s_pFlipbookNext->ubFrame = 0;
	s_pFlipbookNext->cbOnPeak = cbOnPeak;
	s_pFlipbookNext->ulCbData = ulCbData;
	s_pFlipbookNext->isQuick = isQuick;
	s_pFlipbookNext->eKind = eKind;
	if(eKind == FLIPBOOK_KIND_TELEPORT) {
		bobSetFrame(
			&s_pFlipbookNext->sBob,
			bobCalcFrameAddress(s_pTpFrames, 0),
			bobCalcFrameAddress(s_pTpFramesMask, 0)
		);
	}
	else {
		bobSetFrame(
			&s_pFlipbookNext->sBob,
			bobCalcFrameAddress(s_pBoomFrames, 0),
			bobCalcFrameAddress(s_pBoomFramesMask, 0)
		);
	}
	audioMixerPlaySfx(
		eKind == FLIPBOOK_KIND_TELEPORT ?
			s_pSfxTeleport : s_pSfxBoom,
		SFX_CHANNEL_BOOM, SFX_PRIORITY_BOOM, 0
	);
}

void flipbookManagerProcess(void) {
	tFlipbook *pFlipbook = &s_pFlipbooks[0];
	for(UBYTE i = 0; i < FLIPBBOK_MAX; ++i, ++pFlipbook) {
		UBYTE uwCntMax;
		if(pFlipbook->eKind == FLIPBOOK_KIND_TELEPORT) {
			uwCntMax = s_uwTeleportFrameCntMax;
		}
		else {
			uwCntMax = FLIPBBOK_COUNTER_MAX >> pFlipbook->isQuick;
		}
		if(pFlipbook->ubFrame < FLIPBBOK_FRAME_COUNT) {
			if(pFlipbook->ubCnt >= uwCntMax) {
				pFlipbook->ubCnt = 0;
				++pFlipbook->ubFrame;
				if(pFlipbook->ubFrame == FLIPBBOK_FRAME_PEAK && pFlipbook->cbOnPeak) {
					pFlipbook->cbOnPeak(pFlipbook->ulCbData);
				}
				UWORD uwFrameOffsetY = pFlipbook->ubFrame * FLIPBBOK_FRAME_HEIGHT;
				if(pFlipbook->eKind == FLIPBOOK_KIND_TELEPORT) {
					bobSetFrame(
						&pFlipbook->sBob,
						bobCalcFrameAddress(s_pTpFrames, uwFrameOffsetY),
						bobCalcFrameAddress(s_pTpFramesMask, uwFrameOffsetY)
					);
				}
				else {
					bobSetFrame(
						&pFlipbook->sBob,
						bobCalcFrameAddress(s_pBoomFrames, uwFrameOffsetY),
						bobCalcFrameAddress(s_pBoomFramesMask, uwFrameOffsetY)
					);
				}
			}
			else {
				++pFlipbook->ubCnt;
			}
			if(pFlipbook->ubFrame < FLIPBBOK_FRAME_COUNT) {
				gameTryPushBob(&pFlipbook->sBob);
			}
		}
	}
}
