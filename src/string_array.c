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
	char *szDestination;
	UWORD uwIdx = jsonGetDom(pJson, szDom);
	if(uwIdx == 0) {
		logWrite("ERR: %s not found\n", szDom);
		szDestination = memAllocFast(strlen(szDom) + 1);
		strcpy(szDestination, szDom);
	}
	else {
		szDestination = stringCreateFromTok(pJson, pRemap, uwIdx);
	}
	return szDestination;
}

static void stringDestroy(char *szString) {
	memFree(szString, strlen(szString) + 1);
}

//----------------------------------------------------------------- STRING ARRAY

static char **stringArrayCreate(UBYTE ubCount) {
	logBlockBegin("stringArrayCreate(ubCount: %hu)", ubCount);
	char **pArray = 0;
	if(ubCount) {
		pArray = memAllocFast((ubCount + 1) * sizeof(char*));
		for(UBYTE i = 0; i < ubCount; ++i) {
			pArray[i] = STRING_ARRAY_EMPTY_POS;
		}
	}
	pArray[ubCount] = STRING_ARRAY_TERMINATOR;
	logBlockEnd("stringArrayCreate()");
	return pArray;
}

char **stringArrayCreateFromDom(
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
	UBYTE ubCount = pJson->pTokens[uwTokArray].size;
	char **pArray = stringArrayCreate(ubCount);

	for(UWORD i = 0; i < ubCount; ++i) {
		UWORD uwTokElement = jsonGetElementInArray(pJson, uwTokArray, i);
		if(!uwTokElement) {
			logWrite("ERR: json array element not found: '%s'[%hhu]", szDom, i);
		}
		else {
			pArray[i] = stringCreateFromTok(pJson, pRemap, uwTokElement);
		}
	}
	logBlockEnd("stringArrayCreateFromDom()");
	return pArray;
}

char **stringArrayCreateFromDomElements(
	tJson *pJson, const tCodeRemap *pRemap, const char * const *pNames
) {
	logBlockBegin(
		"stringArrayCreateFromDom(pJson: %p, pRemap: %p, pNames: %p)",
		pJson, pRemap, pNames
	);
	UBYTE ubCount = stringArrayGetCount(pNames);
	char **pArray = stringArrayCreate(ubCount);
	for(UBYTE i = 0; i < ubCount; ++i) {
		const char *szDom = pNames[i];
		pArray[i] = stringCreateFromDom(pJson, pRemap, szDom);
	}
	logBlockEnd("stringArrayCreateFromDomElements()");
	return pArray;
}

void stringArrayDestroy(char **pArray) {
	logBlockBegin("stringArrayDestroy(pArray: %p)", pArray);
	UBYTE ubCount;
	for(ubCount = 0; pArray[ubCount] != STRING_ARRAY_TERMINATOR; ++ubCount) {
		if(pArray[ubCount] != STRING_ARRAY_EMPTY_POS) {
			logWrite(
				"Freeing string %hhu (0x%p): '%s'\n",
				ubCount, pArray[ubCount], pArray[ubCount]
			);
			stringDestroy(pArray[ubCount]);
		}
	}
	// Free string pointers + terminator
	memFree(pArray, sizeof(char*) * (ubCount + 1));
	logBlockEnd("stringArrayDestroy()");
}

UBYTE stringArrayGetCount(const char * const *pArray) {
	UBYTE ubCount;
	for(ubCount = 0; pArray[ubCount] != STRING_ARRAY_TERMINATOR; ++ubCount) { }
	return ubCount;
}
