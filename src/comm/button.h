#ifndef _AMINER_BUTTON_H_
#define _AMINER_BUTTON_H_

#include <ace/types.h>
#include <ace/utils/font.h>

#define BUTTON_NAME_MAX 10
#define BUTTON_INVALID 0xFF

typedef enum _tButtonPreset {
	BUTTON_PRESET_CUSTOM,
	BUTTON_PRESET_ACCEPT_DECLINE,
	BUTTON_PRESET_OK,
} tButtonPreset;

typedef struct _tButton {
	char szName[BUTTON_NAME_MAX];
	tUwCoordYX sPos;
} tButton;

void buttonRmAll(void);

UBYTE buttonAdd(const char *szName, UWORD uwX, UWORD uwY);

void buttonDraw(UBYTE ubIdx, tBitMap *pBfr);

void buttonDrawAll(tBitMap *pBfr);

UBYTE buttonSelect(UBYTE ubIdx);

UBYTE buttonGetCount(void);

UBYTE buttonGetSelected(void);

UBYTE buttonGetHeight(void);

tButtonPreset buttonGetPreset(void);

void buttonInitAcceptDecline(
	const char *szTextAccept, const char *szTextDecline
);

void buttonInitOk(const char *szText);

#endif // _AMINER_BUTTON_H_
