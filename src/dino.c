/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dino.h"
#include "bob_new.h"
#include "core.h"
#include "save.h"
#include "tile.h"

#define DINO_BOB_COUNT 9

static tBobNew s_pDinoBobs[DINO_BOB_COUNT];
static UBYTE s_pDinoWereDrawn[DINO_BOB_COUNT];
static UBYTE s_ubDinoBonesFound = 0;
static tBitMap *s_pBones, *s_pBonesMask;


void dinoCreate(void) {
	static const tUwCoordYX s_pDinoPos[DINO_BOB_COUNT] = {
		{.uwX = 32 + 92, .uwY = 100 * 32 + 170},
		{.uwX = 32 + 116, .uwY = 100 * 32 + 179},
		{.uwX = 32 + 147, .uwY = 100 * 32 + 172},
		{.uwX = 32 + 159, .uwY = 100 * 32 + 189},
		{.uwX = 32 + 178, .uwY = 100 * 32 + 170},
		{.uwX = 32 + 215, .uwY = 100 * 32 + 192},
		{.uwX = 32 + 209, .uwY = 100 * 32 + 201},
		{.uwX = 32 + 220, .uwY = 100 * 32 + 205},
		{.uwX = 32 + 250, .uwY = 100 * 32 + 218},
	};

	static const UBYTE pDinoHeights[DINO_BOB_COUNT] = {
		22, 10, 15, 24, 44, 29, 45, 45, 22
	};

	static const UBYTE pDinoFrameOffs[DINO_BOB_COUNT] = {
		0, 24, 35, 51, 76, 121, 151, 197, 243
	};
	s_pBones = bitmapCreateFromFile("data/bones.bm", 0);
	s_pBonesMask = bitmapCreateFromFile("data/bones_mask.bm", 0);

	for(UBYTE i = 0; i < DINO_BOB_COUNT; ++i) {
		bobNewInit(
			&s_pDinoBobs[i], 80, pDinoHeights[i], 0,
			bobNewCalcFrameAddress(s_pBones, pDinoFrameOffs[i]),
			bobNewCalcFrameAddress(s_pBonesMask, pDinoFrameOffs[i]),
			s_pDinoPos[i].uwX, s_pDinoPos[i].uwY
		);
	}
	dinoReset();
}

void dinoDestroy(void) {
	bitmapDestroy(s_pBones);
	bitmapDestroy(s_pBonesMask);
}

void dinoReset(void) {
	s_ubDinoBonesFound = 0;
	for(UBYTE i = 0; i < DINO_BOB_COUNT; ++i) {
		s_pDinoWereDrawn[i] = 0;
	}
}

void dinoSave(tFile *pFile) {
	saveWriteHeader(pFile, "DINO");
	fileWrite(pFile, &s_ubDinoBonesFound, sizeof(s_ubDinoBonesFound));
	// fileWrite(pFile, s_pDinoWereDrawn, sizeof(DINO_BOB_COUNT * s_pDinoWereDrawn[0]));
}

UBYTE dinoLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "DINO")) {
		return 0;
	}

	fileRead(pFile, &s_ubDinoBonesFound, sizeof(s_ubDinoBonesFound));
	// fileRead(pFile, s_pDinoWereDrawn, sizeof(DINO_BOB_COUNT * s_pDinoWereDrawn[0]));
	return 1;
}

void dinoProcess(void) {
	static UBYTE ubLastDino = 0;

	if(s_ubDinoBonesFound && tileBufferIsTileOnBuffer(
		g_pMainBuffer,
		s_pDinoBobs[ubLastDino].sPos.uwX / 32,
		s_pDinoBobs[ubLastDino].sPos.uwY / 32
	) && s_pDinoWereDrawn[ubLastDino] < 2) {
		if(s_pDinoWereDrawn[ubLastDino] >= 2) {
			++ubLastDino;
			if(ubLastDino >= s_ubDinoBonesFound) {
				ubLastDino = 0;
			}
		}
		else {
			bobNewPush(&s_pDinoBobs[ubLastDino]);
			++s_pDinoWereDrawn[ubLastDino];
		}
	}
	else {
		s_pDinoWereDrawn[ubLastDino] = 0;
		++ubLastDino;
		if(ubLastDino >= s_ubDinoBonesFound) {
			ubLastDino = 0;
		}
	}

}

UBYTE dinoGetBoneCount(void) {
	return s_ubDinoBonesFound;
}

void dinoFoundBone(void) {
	++s_ubDinoBonesFound;

	if(s_ubDinoBonesFound == 1) {
		tileReplaceBaseWithVariant(BASE_ID_DINO_UNPOPULATED, BASE_ID_DINO_POPULATED);
	}
}
