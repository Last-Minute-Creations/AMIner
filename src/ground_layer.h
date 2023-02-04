#ifndef _AMINER_GROUND_LAYER_H_
#define _AMINER_GROUND_LAYER_H_

#include <ace/utils/extview.h>

void groundLayerCreate(const tVPort *pVp);

void groundLayerProcess(UWORD uwCameraY, UBYTE ubColorLevel);

void groundLayerReset(UBYTE ubLowerLayer);

void groundLayerSave(tFile *pFile);

UBYTE groundLayerLoad(tFile *pFile);

UWORD groundLayerGetLowerAtDepth(UWORD uwY);

UBYTE groundLayerGetDifficultyAtDepth(UWORD uwDepth);

#endif // _AMINER_GROUND_LAYER_H_
