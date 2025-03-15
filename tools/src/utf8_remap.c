/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 #include "utf8_remap.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include "utf8.h"

 char remapChar(const tCodeRemap *pRemap, uint32_t ulCodepoint) {
	 if(ulCodepoint < 128) {
		 return (char)ulCodepoint;
	 }
	 for(uint16_t j = 0; pRemap[j].ulCodepoint != 0; ++j) {
		 if(pRemap[j].ulCodepoint == ulCodepoint) {
			 return pRemap[j].ubFontCode;
		 }
	 }
	 return (char)ulCodepoint;
 }

 char *remapFile(
	 const char *szFilePath, const tCodeRemap *pRemap,
	 uint16_t *pOutAllocSize, uint16_t *pOutStringSize
 ) {
	 // Read whole file to plain buffer
	 FILE *pFile = fopen(szFilePath, "rb");

	 if(!pFile) {
		 printf("ERR: Couldn't read contents of '%s'\n", szFilePath);
		 return 0;
	 }

	 fseek(pFile, 0, SEEK_END);
	 uint16_t uwRemainingChars = (uint16_t)ftell(pFile);
	 fseek(pFile, 0, SEEK_SET);
	 uint16_t uwFileContentsBufferSize = uwRemainingChars + 1;
	 char *szFileContents = malloc(uwFileContentsBufferSize);
	 fread(szFileContents, uwFileContentsBufferSize, 1, pFile);
	 fclose(pFile);

	 // Unicode takes more or same space than ascii - can convert in-place
	 uint16_t uwTextLength = 0;
	 uint32_t ulCodepoint;
	 uint32_t ulState = 0;
	 uint16_t uwReadPos = 0;
	 uint8_t ubCharCode;
	 while(uwRemainingChars--) {
		 ubCharCode = szFileContents[uwReadPos++];
		 if(decode(&ulState, &ulCodepoint, ubCharCode) != UTF8_ACCEPT) {
			 continue;
		 }

		 if(pRemap) {
			 ubCharCode = remapChar(pRemap, ulCodepoint);
		 }
		 else {
			 ubCharCode = ulCodepoint;
		 }

		 szFileContents[uwTextLength++] = ubCharCode;
	 }
	 szFileContents[uwTextLength] = '\0';

	 *pOutAllocSize = uwFileContentsBufferSize;
	 if(pOutStringSize) {
		 *pOutStringSize = uwTextLength;
	 }
	 return szFileContents;
 }
