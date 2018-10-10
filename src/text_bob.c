/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "text_bob.h"

void textBobCreate(
	tTextBob *pTextBob, const tFont *pFont, const char *szMaxText
) {
	tUwCoordYX sBounds = fontMeasureText(pFont, szMaxText);
	pTextBob->pFont = pFont;
	pTextBob->uwWidth = ((sBounds.sUwCoord.uwX + 3 + 15) / 16) * 16;
	pTextBob->pTextBitmap = fontCreateTextBitMap(pTextBob->uwWidth, pFont->uwHeight);
	UWORD uwHeight = sBounds.sUwCoord.uwY + 3;
	tBitMap *pTextBm = bitmapCreate(
		pTextBob->uwWidth, uwHeight, 4, BMF_INTERLEAVED | BMF_CLEAR
	);
	tBitMap *pTextMask = bitmapCreate(
		pTextBob->uwWidth, uwHeight, 4, BMF_INTERLEAVED | BMF_CLEAR
	);
	bobNewInit(
		&pTextBob->sBob, pTextBob->uwWidth, uwHeight, 1, pTextBm, pTextMask, 0, 0
	);
	pTextBob->isUpdateRequired = 0;
}

void textBobSet(
	tTextBob *pTextBob, const char *szText, UBYTE ubColor,
	UWORD uwPosX, UWORD uwPosY, UWORD uwDestPosY, UBYTE isCenterH
) {
	textBobSetText(pTextBob, szText);
	textBobSetColor(pTextBob, ubColor);
	textBobSetPosition(pTextBob, uwPosX, uwPosY, uwDestPosY, isCenterH);
}

void textBobSetColor(tTextBob *pTextBob, UBYTE ubColor) {
	pTextBob->ubColor = ubColor;
	pTextBob->isUpdateRequired = 1;
}

void textBobSetText(tTextBob *pTextBob, const char *szText, ...) {
	va_list vaArgs;
	va_start(vaArgs, szText);
	vsprintf(pTextBob->szText, szText, vaArgs);
	va_end(vaArgs);
	tUwCoordYX sSize = fontMeasureText(pTextBob->pFont, szText);
	pTextBob->uwWidth = sSize.sUwCoord.uwX;
	pTextBob->isUpdateRequired = 1;
}

void textBobSetPosition(
	tTextBob *pTextBob, UWORD uwX, UWORD uwY, UWORD uwDestY, UBYTE isCenterH
) {
	pTextBob->sBob.sPos.sUwCoord.uwX = uwX;
	pTextBob->sBob.sPos.sUwCoord.uwY = uwY;
	if(isCenterH) {
		pTextBob->sBob.sPos.sUwCoord.uwX -= pTextBob->uwWidth / 2;
	}
	pTextBob->uwDestPosY = uwDestY;
	pTextBob->isUpdateRequired = 1;
}

void textBobUpdate(tTextBob *pTextBob) {
	if(!pTextBob->isUpdateRequired) {
		return;
	}
	fontFillTextBitMap(pTextBob->pFont, pTextBob->pTextBitmap, pTextBob->szText);
	fontDrawTextBitMap(
		pTextBob->sBob.pBitmap, pTextBob->pTextBitmap, 1, 1, pTextBob->ubColor, 0
	);
	// Mask outline
	blitRect(
		pTextBob->sBob.pMask, 0, 0,
		pTextBob->sBob.uwWidth, pTextBob->sBob.pMask->Rows, 0
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 1, 1, 15, 0
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 1, 0, 15, FONT_COOKIE
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 1, 2, 15, FONT_COOKIE
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 0, 1, 15, FONT_COOKIE
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 2, 1, 15, FONT_COOKIE
	);
	pTextBob->isUpdateRequired = 0;
}

void textBobAnimate(tTextBob *pTextBob) {
	if(
		!pTextBob->isUpdateRequired &&
		pTextBob->sBob.sPos.sUwCoord.uwY != pTextBob->uwDestPosY
	) {
		if(pTextBob->sBob.sPos.sUwCoord.uwY < pTextBob->uwDestPosY) {
			++pTextBob->sBob.sPos.sUwCoord.uwY;
		}
		else {
			--pTextBob->sBob.sPos.sUwCoord.uwY;
		}
		bobNewPush(&pTextBob->sBob);
	}
}

void textBobDestroy(tTextBob *pTextBob) {
	bitmapDestroy(pTextBob->sBob.pBitmap);
	bitmapDestroy(pTextBob->sBob.pMask);
	fontDestroyTextBitMap(pTextBob->pTextBitmap);
}
