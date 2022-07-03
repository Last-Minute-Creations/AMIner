/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BOB_NEW_H
#define BOB_NEW_H

#include <ace/types.h>
#include <ace/managers/blit.h>

/**
 * The mighty bob manager.
 * Its workflow is as follows:
 *
 * in gamestate create:
 * bobNewManagerCreate(...)
 * bobNewInit(&sBob1, ...)
 * bobNewInit(&sBob2, ...)
 * bobNewInit(&sBobN, ...)
 * bobNewReallocateBgBuffers()
 *
 * in gamestate loop:
 * bobNewBegin()
 * someCalcHereOrOtherBlitterOperationsHere()
 * bobNewPush(&sBobX) <-- no other blitting past this point
 * someCalcHere()
 * bobNewPush(&sBobY)
 * bobNewPush(&sBobZ)
 * someCalcHere()
 * bobNewProcessNext()
 * someCalcHere()
 * bobNewPush(&sBobT)
 * someCalcHere()
 * bobNewProcessNext()
 * someCalcHere()
 * bobNewPushingDone()
 * someCalcHere()
 * bobNewProcessNext()
 * someCalcHere()
 * bobNewProcessNext()
 * someCalcHere()
 * bobNewEnd()
 * someCalcHereOrOtherBlitterOperationsHere()
 *
 * in gamestate destroy:
 * bobNewManagerDestroy()
 *
 */

/**
 * @brief The bob structure.
 * You can safely change sPos to set new position. Rest is read-only and should
 * only be changed by provided fns.
 */
typedef struct tBobNew {
	UBYTE *pFrameData;
	UBYTE *pMaskData;
	tUwCoordYX pOldPositions[2];
	tUwCoordYX sPos;
	UWORD uwWidth;
	UWORD uwHeight;
	UBYTE isUndrawRequired;
	// Platform-dependent private fields. Don't rely on them externally.
	UWORD _uwBlitSize;
	WORD _wModuloUndrawSave;
} tBobNew;

/**
 * @brief Creates bob manager with optional double buffering support.
 * If you use single buffering, pass same pointer in pFront and pBack.
 *
 * After calling this fn you should call series of bobNewInit() followed by
 * single bobNewReallocateBgBuffers().
 *
 * @param pFront Double buffering's front buffer bitmap.
 * @param pBack Double buffering's back buffer bitmap.
 * @param uwAvailHeight True available height for Y-scroll in passed bitmap.
 *
 * @see bobNewInit()
 * @see bobNewReallocateBgBuffers()
 * @see bobNewManagerDestroy()
 */
void bobNewManagerCreate(tBitMap *pFront, tBitMap *pBack, UWORD uwAvailHeight);

/**
 * @brief Destroys bob manager, releasing all its resources.
 *
 * @see bobNewManagerCreate()
 */
void bobNewManagerDestroy(void);

void bobNewManagerReset(void);

/**
 * @brief Initializes new bob for use with manager.
 *
 * @param pBob Pointer to bob structure.
 * @param uwWidth Bob's width.
 * @param uwHeight Bob's height.
 * @param isUndrawRequired If set to 1, its background will be undrawn.
 * @param pFrameData Pointer to frame to be displayed.
 * @param pMaskData Pointer to transparency mask of pFrameData.
 * @param uwX Initial X position.
 * @param uwY Initial Y position.
 */
void bobNewInit(
	tBobNew *pBob, UWORD uwWidth, UWORD uwHeight, UBYTE isUndrawRequired,
	UBYTE *pFrameData, UBYTE *pMaskData, UWORD uwX, UWORD uwY
);

/**
 * @brief Allocates buffers for storing background for later undrawing of bobs.
 * Background of all bobs are stored in single buffer. This way there is no need
 * to reconfigure blitter's destination register when storing BGs.
 *
 * After call to this function, you can't call bobNewInit() anymore!
 */
void bobNewReallocateBgBuffers(void);

/**
 * @brief Changes bob's animation frame.
 *
 * Storing animation frames one under another implies simplest calculations,
 * hence exclusively supported by this manager.
 *
 * @param pBob Bob which should have its frame changed.
 * @param pFrameData Pointer to frame to be displayed.
 * @param pMaskData Pointer to transparency mask of pFrameData.
 */
void bobNewSetFrame(tBobNew *pBob, UBYTE *pFrameData, UBYTE *pMaskData);

/**
 * @brief Calculates byte address of a frame located at given Y offset.
 *
 * This function assumes that bitmap is exactly 1 frame-wide and next frames
 * are located one after another.
 *
 * @param pBitmap Bitmap which stores animation frames/masks.
 * @param uwOffsetY Y Offset of frame which address is to be calculated.
 * @return Byte address of frame/mask data of given frame.
 */
UBYTE *bobNewCalcFrameAddress(tBitMap *pBitmap, UWORD uwOffsetY);

/**
 * @brief Undraws all bobs, restoring BG to its former state.
 * Also bob current drawing queue is reset, making room for pushing new bobs.
 * After calling this function, you may push new bobs to screen.
 *
 * @see bobNewPush()
 */
void bobNewBegin(tBitMap *pBuffer);

/**
 * @brief Adds next bob to draw queue.
 * Bobs which were pushed in previous frame but not in current will still be
 * undrawn if needed.
 * There is no z-order, thus bobs are drawn in order of pushing.
 * When this function operates, it calls bobNewProcessNext().
 * Don't modify bob's struct past calling this fn - there is no guarantee when
 * bob system will access its data!
 *
 * @param pBob Pointer to bob to be drawn.
 *
 * @see bobNewProcessNext()
 * @see bobNewPushingDone()
 */
void bobNewPush(tBobNew *pBob);

/**
 * @brief Tries to store BG of or draw next bob.
 * Call this function periodically to check if blitter is idle and if it is,
 * give it more work to do.
 * Before calling bobNewPushingDone() bobs have their BG stored so that BG of
 * later pushed bobs won't get corrupted with gfx of earlier processed ones.
 *
 * Don't use blitter for any other thing until you do bobNewEnd()! It will
 * heavily corrupt memory!
 *
 * @return 1 if there's still some work to do by the blitter, otherwise 0.
 */
UBYTE bobNewProcessNext(void);

/**
 * @brief Closes drawing queue.
 * After calling this function bobs will get actually drawn, instead of just
 * storing BGs of bobs pushed to this point.
 * It also indicates that there will be no call to bobNewPush() until next
 * bobNewBegin().
 *
 * @see bobNewEnd()
 */
void bobNewPushingDone(void);

/**
 * @brief Ends bob processing, enforcing all remaining bobs to be drawn.
 * After making this call all other blitter operations are safe again.
 */
void bobNewEnd(void);

void bobNewDiscardUndraw(void);

#endif // BOB_NEW_H
