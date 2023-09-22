/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ASSETS_H
#define ASSETS_H

#include <ace/managers/ptplayer.h>
#include <ace/utils/bitmap.h>

#define ASSETS_GAME_MOD_COUNT 10

extern tPtplayerSfx *g_pSfxDrill;
extern tPtplayerSfx *g_pSfxOre;
extern tPtplayerSfx *g_pSfxPenalty;
extern tPtplayerSfx *g_pSfxFlyLoop;
extern tPtplayerMod *g_pGameMods[ASSETS_GAME_MOD_COUNT];
extern tPtplayerMod *g_pMenuMod;
extern tPtplayerSamplePack *g_pModSampleData;

extern tBitMap *g_pBombMarker;
extern tBitMap *g_pBombMarkerMask;
extern tFont *g_pFont;

void assetsAudioCreate(void);

void assetsAudioDestroy(void);

void assetsBombMarkersCreate(void);

void assetsBombMarkersDestroy(void);

#endif // ASSETS_H
