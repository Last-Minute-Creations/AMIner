#ifndef _UTF8_REMAP_H_
#define _UTF8_REMAP_H_

#include <ace/types.h>

typedef struct _tCodeRemap {
	ULONG ulCodepoint;
	UBYTE ubFontCode;
} tCodeRemap;

char remapChar(const tCodeRemap *pRemapCodes, ULONG ulCodepoint);

#endif // _UTF8_REMAP_H_
