/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "text_bob.h"

void textBobCreate(
	tTextBob *pTextBob, const tFont *pFont
) {
	pTextBob->pFont = pFont;
	pTextBob->uwWidth = 64;
	pTextBob->pTextBitmap = fontCreateTextBitMap(pTextBob->uwWidth, pFont->uwHeight);
	UWORD uwHeight = pFont->uwHeight + 3;
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
	UWORD uwPosX, UWORD uwPosY, UWORD uwDestPosY
) {
	strcpy(pTextBob->szText, szText);
	pTextBob->ubColor = ubColor;
	pTextBob->sBob.sPos.sUwCoord.uwX = uwPosX;
	pTextBob->sBob.sPos.sUwCoord.uwY = uwPosY;
	pTextBob->uwDestPosY = uwDestPosY;
	pTextBob->isUpdateRequired = 1;
}

void textBobUpdate(tTextBob *pTextBob) {
	if(!pTextBob->isUpdateRequired) {
		return;
	}
	fontFillTextBitMap(pTextBob->pFont, pTextBob->pTextBitmap, pTextBob->szText);
	fontDrawTextBitMap(
		pTextBob->sBob.pBitmap, pTextBob->pTextBitmap, 64/2 + 1, 1,
		pTextBob->ubColor, FONT_HCENTER
	);
	// Mask outline
	blitRect(pTextBob->sBob.pMask, 0, 0, 64, pTextBob->sBob.pMask->Rows, 0);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 64/2+1, 1, 15,
		FONT_HCENTER
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 64/2+1, 0, 15,
		FONT_COOKIE | FONT_HCENTER
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 64/2+1, 2, 15,
		FONT_COOKIE | FONT_HCENTER
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 64/2+0, 1, 15,
		FONT_COOKIE | FONT_HCENTER
	);
	fontDrawTextBitMap(
		pTextBob->sBob.pMask, pTextBob->pTextBitmap, 64/2+2, 1, 15,
		FONT_COOKIE | FONT_HCENTER
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
