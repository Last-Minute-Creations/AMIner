#include "utf8_remap.h"

char remapChar(const tCodeRemap *pRemap, ULONG ulCodepoint) {
	UWORD j;
	for(j = 0; pRemap[j].ulCodepoint != 0; ++j) {
		if(pRemap[j].ulCodepoint == ulCodepoint) {
			return pRemap[j].ubFontCode;
		}
	}
	return (char)ulCodepoint;
}
