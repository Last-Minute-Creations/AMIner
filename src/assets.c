/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "assets.h"

tPtplayerSfx *g_pSfxDrill;
tPtplayerSfx *g_pSfxOre;
tPtplayerSfx *g_pSfxPenalty;
tPtplayerSfx *g_pSfxFlyLoop;
tPtplayerMod *g_pGameMods[ASSETS_GAME_MOD_COUNT];
tPtplayerMod *g_pMenuMod;
tPtplayerSamplePack *g_pModSampleData;

tBitMap *g_pBombMarker;
tBitMap *g_pBombMarkerMask;
tBitMap *g_pTileOverlays;
tBitMap *g_pTileOverlayMasks;
tFont *g_pFont;
tPakFile *g_pPakFile;

void assetsAudioCreate(void) {
	g_pSfxFlyLoop = ptplayerSfxCreateFromFd(pakFileGetFile(g_pPakFile, "sfx/fly_loop.sfx"), 1);
	g_pSfxDrill = ptplayerSfxCreateFromFd(pakFileGetFile(g_pPakFile, "sfx/drill1.sfx"), 1);
	g_pSfxOre = ptplayerSfxCreateFromFd(pakFileGetFile(g_pPakFile, "sfx/ore2.sfx"), 1);
	g_pSfxPenalty = ptplayerSfxCreateFromFd(pakFileGetFile(g_pPakFile, "sfx/penalty.sfx"), 1);

	for(UBYTE i = 0; i < ASSETS_GAME_MOD_COUNT; ++i) {
		char szModPath[30];
		sprintf(szModPath, "music/game%hhu.mod", i);
		g_pGameMods[i] = ptplayerModCreateFromFd(pakFileGetFile(g_pPakFile, szModPath));
	}
	g_pMenuMod = ptplayerModCreateFromFd(pakFileGetFile(g_pPakFile, "music/menu.mod"));
	g_pModSampleData = ptplayerSampleDataCreateFromFd(pakFileGetFile(g_pPakFile, "music/samples.samplepack"));
}

void assetsAudioDestroy(void) {
	ptplayerSfxDestroy(g_pSfxDrill);
	ptplayerSfxDestroy(g_pSfxFlyLoop);
	ptplayerSfxDestroy(g_pSfxOre);
	ptplayerSfxDestroy(g_pSfxPenalty);

	for(UBYTE i = 0; i < ASSETS_GAME_MOD_COUNT; ++i) {
		ptplayerModDestroy(g_pGameMods[i]);
	}
	ptplayerModDestroy(g_pMenuMod);
	ptplayerSamplePackDestroy(g_pModSampleData);
}

void assetsMarkersCreate(void) {
	g_pBombMarker = bitmapCreateFromFd(pakFileGetFile(g_pPakFile, "bomb_marker.bm"), 0);
	g_pBombMarkerMask = bitmapCreateFromFd(pakFileGetFile(g_pPakFile, "bomb_marker_mask.bm"), 0);
}

void assetsMarkersDestroy(void) {
	bitmapDestroy(g_pBombMarker);
	bitmapDestroy(g_pBombMarkerMask);
}

void assetsTileOverlayCreate(void) {
	g_pTileOverlays = bitmapCreateFromFd(pakFileGetFile(g_pPakFile, "tiles_overlay.bm"), 0);
	g_pTileOverlayMasks = bitmapCreateFromFd(pakFileGetFile(g_pPakFile, "tiles_overlay_masks.bm"), 0);
}

void assetsTileOverlayDestroy(void) {
	bitmapDestroy(g_pTileOverlays);
	bitmapDestroy(g_pTileOverlayMasks);
}
