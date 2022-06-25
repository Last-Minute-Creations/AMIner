#ifndef _JSON_JSON_H_
#define _JSON_JSON_H_

#define JSMN_STRICT
#define JSMN_HEADER
#include "jsmn.h"
#include "utf8_remap.h"
#include <ace/types.h>

typedef struct _tJson {
	char *szData;
	jsmntok_t *pTokens;
	WORD wTokenCount;
} tJson;

tJson *jsonCreate(const char *szFilePath);

void jsonDestroy(tJson *pJson);

UWORD jsonGetElementInArray(const tJson *pJson,UWORD uwParentIdx,UWORD uwIdx);

UWORD jsonGetElementInStruct(
	const tJson *pJson,UWORD uwParentIdx,const char *szElement
);

UWORD jsonGetDom(const tJson *pJson,const char *szPattern);

ULONG jsonTokToUlong(const tJson *pJson,UWORD uwTok);

UWORD jsonStrLen(const tJson *pJson, UWORD uwTok);

UWORD jsonTokStrCpy(
	const tJson *pJson, const tCodeRemap *pRemap, UWORD uwTok, char *pDst,
	UWORD uwMaxBytes
);

#endif // _JSON_JSON_H_
