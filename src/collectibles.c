/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "collectibles.h"
#include <ace/managers/bob.h>
#include "core.h"

#define COLLECTIBLES_BOB_MAX 16

typedef struct tCollectiblesZoneDef {
	UWORD uwDepthTop;
	UWORD uwDepthBottom;
	UBYTE ubCount;
	tUwCoordYX pBobCoords[COLLECTIBLES_BOB_MAX];
	tUwCoordYX pBobSizes[COLLECTIBLES_BOB_MAX];
	const char *szFramesFile;
	const char *szMasksFile;
} tCollectiblesZoneDef;

typedef struct tCollectiblesZoneData {
	UBYTE ubFoundCount;
	tBitMap *pBitmapFrames;
	tBitMap *pBitmapMasks;
	UBYTE *pFrames[COLLECTIBLES_BOB_MAX];
	UBYTE *pMasks[COLLECTIBLES_BOB_MAX];
} tCollectiblesZoneData;

//----------------------------------------------------------------- PRIVATE VARS

static const tCollectiblesZoneDef s_pZones[COLLECTIBLE_KIND_COUNT] = {
	[COLLECTIBLE_KIND_DINO] = {
		.uwDepthTop = 3000,
		.uwDepthBottom = 4000,
		.ubCount = 9,
		.pBobCoords = {
			{.uwX = 32 + 92, .uwY = 100 * 32 + 170},
			{.uwX = 32 + 116, .uwY = 100 * 32 + 179},
			{.uwX = 32 + 147, .uwY = 100 * 32 + 172},
			{.uwX = 32 + 159, .uwY = 100 * 32 + 189},
			{.uwX = 32 + 178, .uwY = 100 * 32 + 170},
			{.uwX = 32 + 215, .uwY = 100 * 32 + 192},
			{.uwX = 32 + 209, .uwY = 100 * 32 + 201},
			{.uwX = 32 + 220, .uwY = 100 * 32 + 205},
			{.uwX = 32 + 250, .uwY = 100 * 32 + 218},
		},
		.pBobSizes = {
			{.uwX = 80, .uwY = 22},
			{.uwX = 80, .uwY = 10},
			{.uwX = 80, .uwY = 15},
			{.uwX = 80, .uwY = 24},
			{.uwX = 80, .uwY = 44},
			{.uwX = 80, .uwY = 29},
			{.uwX = 80, .uwY = 45},
			{.uwX = 80, .uwY = 45},
			{.uwX = 80, .uwY = 22},
		},
		.szFramesFile = "data/bones.bm",
		.szMasksFile = "data/bones_mask.bm",
	},
	[COLLECTIBLE_KIND_GATE] = {
		.uwDepthTop = 8000,
		.uwDepthBottom = 10000,
		// TODO: Proper values
		.ubCount = 9,
		.pBobCoords = {
			{.uwX = 32 + 92, .uwY = 100 * 32 + 170},
			{.uwX = 32 + 116, .uwY = 100 * 32 + 179},
			{.uwX = 32 + 147, .uwY = 100 * 32 + 172},
			{.uwX = 32 + 159, .uwY = 100 * 32 + 189},
			{.uwX = 32 + 178, .uwY = 100 * 32 + 170},
			{.uwX = 32 + 215, .uwY = 100 * 32 + 192},
			{.uwX = 32 + 209, .uwY = 100 * 32 + 201},
			{.uwX = 32 + 220, .uwY = 100 * 32 + 205},
			{.uwX = 32 + 250, .uwY = 100 * 32 + 218},
		},
		.pBobSizes = {
			{.uwX = 80, .uwY = 22},
			{.uwX = 80, .uwY = 10},
			{.uwX = 80, .uwY = 15},
			{.uwX = 80, .uwY = 24},
			{.uwX = 80, .uwY = 44},
			{.uwX = 80, .uwY = 29},
			{.uwX = 80, .uwY = 45},
			{.uwX = 80, .uwY = 45},
			{.uwX = 80, .uwY = 22},
		},
		.szFramesFile = "data/bones.bm",
		.szMasksFile = "data/bones_mask.bm",
	},
};

static tBob s_pBobs[COLLECTIBLES_BOB_MAX];
static UBYTE s_ubCurrentZone;
static UBYTE s_ubCurrentBob;
static UBYTE s_isBobsConfigured;
static UBYTE s_pBobsDrawCounts[COLLECTIBLES_BOB_MAX];

static tCollectiblesZoneData s_pZoneDatas[COLLECTIBLE_KIND_COUNT];

//------------------------------------------------------------------ PRIVATE FNS

static void configureBobsForCurrentZone(void) {
	const tCollectiblesZoneDef *pZoneDef = &s_pZones[s_ubCurrentZone];
	tCollectiblesZoneData *pZoneData = &s_pZoneDatas[s_ubCurrentZone];
	for(UBYTE i = 0; i < pZoneDef->ubCount; ++i) {
		bobSetWidth(&s_pBobs[i], pZoneDef->pBobSizes[i].uwX);
		bobSetHeight(&s_pBobs[i], pZoneDef->pBobSizes[i].uwY);
		s_pBobs[i].sPos.ulYX = pZoneDef->pBobCoords[i].ulYX;
		bobSetFrame(&s_pBobs[i], pZoneData->pFrames[i], pZoneData->pMasks[i]);
		s_pBobsDrawCounts[i] = 0;
	}
}

//------------------------------------------------------------------- PUBLIC FNS

void collectiblesCreate(void) {
	for(tCollectibleKind eKind = 0; eKind < COLLECTIBLE_KIND_COUNT; ++eKind) {
		s_pZoneDatas[eKind].pBitmapFrames = bitmapCreateFromFile(s_pZones[eKind].szFramesFile, 0);
		s_pZoneDatas[eKind].pBitmapMasks = bitmapCreateFromFile(s_pZones[eKind].szMasksFile, 0);

		s_pZoneDatas[eKind].ubFoundCount = 0;
		UWORD uwOffsY = 0;
		for(UBYTE ubBobIndex = 0; ubBobIndex < s_pZones[eKind].ubCount; ++ubBobIndex) {
			s_pZoneDatas[eKind].pFrames[ubBobIndex] =  bobCalcFrameAddress(s_pZoneDatas[eKind].pBitmapFrames, uwOffsY),
			s_pZoneDatas[eKind].pMasks[ubBobIndex] =  bobCalcFrameAddress(s_pZoneDatas[eKind].pBitmapMasks, uwOffsY),
			uwOffsY += s_pZones[eKind].pBobSizes[ubBobIndex].uwY;
		}
	}
	for(UBYTE ubBobIndex = 0; ubBobIndex < COLLECTIBLES_BOB_MAX; ++ubBobIndex) {
		bobInit(
			&s_pBobs[ubBobIndex], 0, 0, 0, 0, 0, 0, 0
		);
	}

	collectiblesReset();
}

void collectiblesProcess(void) {
	tCameraManager *pCamera = g_pMainBuffer->pCamera;
	const tCollectiblesZoneDef *pZone = &s_pZones[s_ubCurrentZone];

	if(pCamera->uPos.uwY + g_pMainBuffer->uwMarginedHeight < pZone->uwDepthTop) {
		if(s_ubCurrentZone) {
			--s_ubCurrentZone;
			s_ubCurrentBob = 0;
		}
		return;
	}
	if(pCamera->uPos.uwY > pZone->uwDepthBottom) {
		if(s_ubCurrentZone < COLLECTIBLE_KIND_COUNT - 1) {
			++s_ubCurrentZone;
			s_ubCurrentBob = 0;
		}
		return;
	}

	if(!s_isBobsConfigured) {
		configureBobsForCurrentZone();
		s_isBobsConfigured = 1;
		return;
	}

	if(s_pZoneDatas[s_ubCurrentZone].ubFoundCount && tileBufferIsTileOnBuffer(
		g_pMainBuffer,
		s_pBobs[s_ubCurrentBob].sPos.uwX / 32, s_pBobs[s_ubCurrentBob].sPos.uwY / 32
	) && s_pBobsDrawCounts[s_ubCurrentBob] < 2) {
		if(s_pBobsDrawCounts[s_ubCurrentBob] >= 2) {
			++s_ubCurrentBob;
			if(s_ubCurrentBob >= s_pZoneDatas[s_ubCurrentZone].ubFoundCount) {
				s_ubCurrentBob = 0;
			}
		}
		else {
			bobPush(&s_pBobs[s_ubCurrentBob]);
			++s_pBobsDrawCounts[s_ubCurrentBob];
		}
	}
	else {
		s_pBobsDrawCounts[s_ubCurrentBob] = 0;
		++s_ubCurrentBob;
		if(s_ubCurrentBob >= s_pZoneDatas[s_ubCurrentZone].ubFoundCount) {
			s_ubCurrentBob = 0;
		}
	}
}

void collectiblesDestroy(void) {
	for(tCollectibleKind eKind = 0; eKind < COLLECTIBLE_KIND_COUNT; ++eKind) {
		bitmapDestroy(s_pZoneDatas[eKind].pBitmapFrames);
		bitmapDestroy(s_pZoneDatas[eKind].pBitmapMasks);
	}
}

void collectiblesReset(void) {
	s_isBobsConfigured = 0;
	s_ubCurrentBob = 0;
	s_ubCurrentZone = 0;
}

void collectibleSetFoundCount(tCollectibleKind eKind, UBYTE ubCount) {
	s_pZoneDatas[eKind].ubFoundCount = ubCount;
}
