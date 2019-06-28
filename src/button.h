#ifndef _AMINER_BUTTON_H_
#define _AMINER_BUTTON_H_

#include <ace/types.h>
#include <ace/utils/font.h>

#define BUTTON_NAME_MAX 10
#define BUTTON_INVALID 0xFF

typedef struct _tButton {
	char szName[BUTTON_NAME_MAX];
	tUwCoordYX sPos;
} tButton;

void buttonRmAll(void);

UBYTE buttonAdd(const char *szName, UWORD uwX, UWORD uwY);

void buttonDraw(UBYTE ubIdx, tBitMap *pBfr, tTextBitMap *pTextBitmap);

void buttonDrawAll(tBitMap *pBfr, tTextBitMap *pTextBitmap);

void buttonSelect(UBYTE ubIdx);

UBYTE buttonGetSelected(void);

#endif // _AMINER_BUTTON_H_
