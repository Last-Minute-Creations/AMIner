/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "string_array.h"
#include <stdarg.h>
#include <ace/managers/memory.h>
#include <ace/managers/log.h>

//----------------------------------------------------------------- STRING ALLOC

static char *stringCreateFromTok(
	const tJson *pJson, const tCodeRemap *pRemap, UWORD uwTokIdx
) {
	char *szDestination = 0;
	if(pJson->pTokens[uwTokIdx].type == JSMN_ARRAY) {
		UWORD uwAllocSize = 0;
		for(UBYTE i = 0; i < pJson->pTokens[uwTokIdx].size; ++i) {
			UWORD uwArrIdx = jsonGetElementInArray(pJson, uwTokIdx, i);
			uwAllocSize += jsonStrLen(pJson, uwArrIdx) + 1;
		}
		szDestination = memAllocFast(uwAllocSize);
		UWORD uwOffs = 0;
		for(UBYTE i = 0; i < pJson->pTokens[uwTokIdx].size; ++i) {
			UWORD uwArrIdx = jsonGetElementInArray(pJson, uwTokIdx, i);
			uwOffs += jsonTokStrCpy(
				pJson, pRemap, uwArrIdx, &szDestination[uwOffs], uwAllocSize - uwOffs
			);
			szDestination[uwOffs++] = '\n';
		}
		szDestination[uwAllocSize - 1] = '\0';
	}
	else if(pJson->pTokens[uwTokIdx].type == JSMN_STRING) {
		UWORD uwAllocSize = jsonStrLen(pJson, uwTokIdx) + 1;
		szDestination = memAllocFast(uwAllocSize);
		jsonTokStrCpy(pJson, pRemap, uwTokIdx, szDestination, uwAllocSize);
	}
	else {
		logWrite("ERR: unknown json node type: %d\n", pJson->pTokens[uwTokIdx].type);
	}
	return szDestination;
}

static char *stringCreateFromDom(
	const tJson *pJson, const tCodeRemap *pRemap, const char *szDom
) {
	UWORD uwIdx = jsonGetDom(pJson, szDom);
	if(uwIdx == 0) {
		logWrite("ERR: %s not found\n", szDom);
		return 0;
	}
	return stringCreateFromTok(pJson, pRemap, uwIdx);
}

static void stringDestroy(char *szString) {
	memFree(szString, strlen(szString) + 1);
}

//----------------------------------------------------------------- STRING ARRAY

static tStringArray stringArrayCreate(UBYTE ubCount) {
	tStringArray sArray;
	sArray.ubCount = ubCount;
	if(ubCount) {
		sArray.pStrings = memAllocFastClear(ubCount * sizeof(char*));
	}
	return sArray;
}

tStringArray stringArrayCreateFromDom(
	tJson *pJson, const tCodeRemap *pRemap, const char *szDom
) {
	logBlockBegin(
		"stringArrayCreateFromDom(pJson: %p, pRemap: %p, szDom: '%s')",
		pJson, pRemap, szDom
	);
	UWORD uwTokArray = jsonGetDom(pJson, szDom);
	if(!uwTokArray) {
		logWrite("ERR: json not found: '%s'\n", szDom);
		logBlockEnd("stringArrayCreateFromDom()");
		return stringArrayCreate(0);
	}
	tStringArray sArray = stringArrayCreate(pJson->pTokens[uwTokArray].size);

	for(UWORD i = 0; i < sArray.ubCount; ++i) {
		UWORD uwTokElement = jsonGetElementInArray(pJson, uwTokArray, i);
		if(!uwTokElement) {
			logWrite("ERR: json array element not found: '%s'[%hhu]", szDom, i);
		}
		else {
			sArray.pStrings[i] = stringCreateFromTok(pJson, pRemap, uwTokElement);
		}
	}
	logBlockEnd("stringArrayCreateFromDom()");
	return sArray;
}

tStringArray stringArrayCreateFromDomElements(
	tJson *pJson, const tCodeRemap *pRemap, UBYTE ubCount, ...
) {
	logBlockBegin("stringArrayCreateFromDom(pJson: %p, ubCount: '%hhu')", pJson, ubCount);
	tStringArray sArray = stringArrayCreate(ubCount);
	va_list vaArgs;
	va_start(vaArgs, ubCount);
	for(UBYTE i = 0; i < ubCount; ++i) {
		const char *szDom = va_arg(vaArgs, const char*);
		sArray.pStrings[i] = stringCreateFromDom(pJson, pRemap, szDom);
	}
	va_end(vaArgs);
	logBlockEnd("stringArrayCreateFromDomElements()");
	return sArray;
}


void stringArrayDestroy(tStringArray *pArray) {
	logBlockBegin("stringArrayDestroy(pArray: %p)", pArray);
	for(UWORD i = 0; i < pArray->ubCount; ++i) {
		logWrite("Freeing string %hhu (0x%p): '%s'\n", i, pArray->pStrings[i], pArray->pStrings[i]);
		stringDestroy(pArray->pStrings[i]);
	}
	memFree(pArray->pStrings, sizeof(char*) * pArray->ubCount);
	logBlockEnd("stringArrayDestroy()");
}
