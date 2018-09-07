/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h> // Keyboard processing
#include <ace/managers/game.h> // For using gameClose
#include <ace/managers/system.h> // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/utils/palette.h>
#include <ace/utils/custom.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include "bob_new.h"

// All variables outside fns are global - can be accessed in any fn
// Static means here that given var is only for this file, hence 's_' prefix
// You can have many variables with same name in different files and they'll be
// independent as long as they're static
// * means pointer, hence 'p' prefix
static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpScore; // Viewport for score
static tSimpleBufferManager *s_pScoreBuffer;
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;

static tBitMap *s_pTiles;

static tBobNew s_sBobDrill;
static tBitMap *s_pDrillBitmap, *s_pDrillMask;

#define TILE_ROCK 0
#define TILE_GOLD 1
#define TILE_COPPER 2
#define TILE_COAL 3
#define TILE_DIAMOND 4
#define TILE_RUBY 5
#define TILE_DIRT 6

#define DRILL_WIDTH 32
#define DRILL_HEIGHT 23
// TODO sapphire, emerald, topaz

// 32px: 1 << 5
#define TILE_SIZE 5

void gameGsCreate(void) {
  // Create a view - first arg is always zero, then it's option-value
  s_pView = viewCreate(0,
    TAG_VIEW_GLOBAL_CLUT, 1, // Same Color LookUp Table for all viewports
  TAG_END); // Must always end with TAG_END or synonym: TAG_DONE

  // Viewport for score bar - on top of screen
  s_pVpScore = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView, // Required: specify parent view
    TAG_VPORT_BPP, 4, // Optional: 2 bits per pixel, 4 colors
    TAG_VPORT_HEIGHT, 32, // Optional: let's make it 32px high
  TAG_END); // same syntax as view creation

  // Create simple buffer manager with bitmap exactly as large as viewport
  s_pScoreBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpScore, // Required: parent viewport
    // Optional: buffer bitmap creation flags
    // we'll use them to initially clear the bitmap
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
  TAG_END);

  // Now let's do the same for main playfield
  s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 4, // 2 bits per pixel, 4 colors
    // We won't specify height here - viewport will take remaining space.
  TAG_END);
  s_pMainBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpMain, // Required: parent viewport
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
  TAG_END);

  // Since we've set up global CLUT, palette will be loaded from first viewport
  // Colors are 0x0RGB, each channel accepts values from 0 to 15 (0 to F).
  s_pVpScore->pPalette[0] = 0x0000; // First color is also border color
  s_pVpScore->pPalette[1] = 0x0888; // Gray
  s_pVpScore->pPalette[2] = 0x0800; // Red - not max, a bit dark
  s_pVpScore->pPalette[3] = 0x0008; // Blue - same brightness as red

	s_pTiles = bitmapCreateFromFile("data/tiles.bm");
	paletteLoad("data/aminer.plt", s_pVpScore->pPalette, 16);
	paletteLoad("data/aminer.plt", s_pVpMain->pPalette, 16);

  // We don't need anything from OS anymore
  systemUnuse();
	randInit(2184);

	for(UBYTE x = 0; x < 10; ++x) {
		blitCopyAligned(
			s_pTiles, 0, TILE_DIRT << TILE_SIZE, s_pMainBuffer->pBack,
			x << TILE_SIZE, 2 << TILE_SIZE, 32, 32
		);
		blitCopyAligned(
			s_pTiles, 0, TILE_DIRT << TILE_SIZE, s_pMainBuffer->pFront,
			x << TILE_SIZE, 2 << TILE_SIZE, 32, 32
		);
		for(UBYTE y = 3; y < 7; ++y) {
			blitCopyAligned(
				s_pTiles, 0, TILE_ROCK << TILE_SIZE, s_pMainBuffer->pBack,
				x << TILE_SIZE, y << TILE_SIZE, 32, 32
			);
			blitCopyAligned(
				s_pTiles, 0, TILE_ROCK << TILE_SIZE, s_pMainBuffer->pFront,
				x << TILE_SIZE, y << TILE_SIZE, 32, 32
			);
		}
	}

	bobNewManagerCreate(
		1, DRILL_HEIGHT*(DRILL_WIDTH/16 + 1),
		s_pMainBuffer->pFront, s_pMainBuffer->pBack
	);

	s_pDrillBitmap = bitmapCreateFromFile("data/drill/drill.bm");
	s_pDrillMask = bitmapCreateFromFile("data/drill/drill_mask.bm");
	bobNewInit(&s_sBobDrill, DRILL_WIDTH, DRILL_HEIGHT, 1, s_pDrillBitmap, s_pDrillMask, 0, 0);

  // Load the view
  viewLoad(s_pView);
}

void gameGsLoop(void) {
	g_pCustom->color[0] = 0x800;
  if(keyCheck(KEY_ESCAPE)) {
    gameClose();
		return;
  }

	bobNewBegin();
	if(keyCheck(KEY_D)) {
		s_sBobDrill.sPos.sUwCoord.uwX += 2;
		bobNewSetBitMapOffset(&s_sBobDrill, 0);
	}
	else if(keyCheck(KEY_A)) {
		s_sBobDrill.sPos.sUwCoord.uwX -= 2;
		bobNewSetBitMapOffset(&s_sBobDrill, DRILL_HEIGHT);
	}
	bobNewPush(&s_sBobDrill);
	bobNewPushingDone();
	bobNewEnd();

	viewProcessManagers(s_pView);
	copProcessBlocks();
	g_pCustom->color[0] = 0x000;
	vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  // Cleanup when leaving this gamestate
  systemUse();

	bitmapDestroy(s_pTiles);
	bitmapDestroy(s_pDrillBitmap);
	bitmapDestroy(s_pDrillMask);

	bobNewManagerDestroy();

  // This will also destroy all associated viewports and viewport managers
  viewDestroy(s_pView);
}
