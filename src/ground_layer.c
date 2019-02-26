#include "ground_layer.h"
#include <ace/utils/extview.h>

#define RGB(r,g,b) ((((r) >> 4) << 8) | (((g) >> 4) << 4) | (((b) >> 4) << 0))

#define LAYER_COLOR_START 27
#define LAYER_COLOR_COUNT (32 - LAYER_COLOR_START)
#define LAYER_COUNT 5

static tCopBlock *s_pColorsAbove;
static tCopBlock *s_pColorsBelow;
static UBYTE s_isSet = 0;
static UWORD s_uwVpHeight, s_uwVpStartY;
static tCopList *s_pCopList;
tCopBlock *s_pDisableNext = 0;
static UBYTE s_ubLowerLayer = 1;

typedef struct _tGroundLayer {
	UWORD pColors[LAYER_COLOR_COUNT];
	UWORD uwTop;
	UBYTE ubDifficulty;
} tGroundLayer;

static const tGroundLayer s_pLayers[LAYER_COUNT] = {
	{
		// Stock
		{
			RGB(51, 0, 0), RGB(102, 34, 0), RGB(153, 68, 17),
			RGB(204, 102, 34), RGB(255, 136, 51)
		},
		0, 0
	},
	{
		// A
		{
			RGB(51, 34, 0), RGB(102, 68, 0), RGB(153, 102, 17),
			RGB(204, 136, 34), RGB(255, 170, 51)
		},
		512 + 32, 0
	},
	{
		// B
		{
			RGB(51, 34, 17), RGB(102, 68, 34), RGB(153, 102, 51),
			RGB(204, 136, 68), RGB(255, 170, 85)
		},
		1024 + 32, 0
	},
	{
		// C
		{
			RGB(17, 34, 17), RGB(68, 68, 34), RGB(119, 102, 51),
			RGB(170, 136, 68), RGB(221, 170, 85)
		},
		1536 + 32, 0
	},
	{
		// D
		{
			RGB(17, 34, 34), RGB(68, 68, 51), RGB(119, 102, 68),
			RGB(170, 136, 85), RGB(221, 170, 102)
		},
		2048 + 32, 0
	}
};

void groundLayerCreate(tVPort *pVp) {
	logBlockBegin("groundLayerCreate(pVp: %p)", pVp);
	tView *pView = pVp->pView;
	s_pCopList = pView->pCopList;
	s_uwVpStartY = pVp->uwOffsY + 0x2C;
	s_uwVpHeight = pVp->uwHeight;
	s_isSet = 0;
	s_pColorsBelow = copBlockCreate(pView->pCopList, LAYER_COUNT, 0, 0);
	s_pColorsBelow->ubDisabled = 1;
	s_pColorsAbove = copBlockCreate(pView->pCopList, LAYER_COUNT, 0, 0);
	// s_pColorsAbove->ubDisabled = 1;
	logBlockEnd("groundLayerCreate()");
}

static void layerCopyColorsToBlock(
	const tGroundLayer *pLayer, tCopBlock *pBlock
) {
	volatile UWORD *pColorRegs = &g_pCustom->color[LAYER_COLOR_START];
	pBlock->uwCurrCount = 0;
	for(UBYTE i = 0; i < LAYER_COLOR_COUNT; ++i) {
		copMove(s_pCopList, pBlock, &pColorRegs[i], pLayer->pColors[i]);
	}
}

void groundLayerProcess(UWORD uwCameraY) {
	if(uwCameraY < s_pLayers[s_ubLowerLayer-1].uwTop) {
		--s_ubLowerLayer;
	}
	else if(uwCameraY + s_uwVpHeight >= s_pLayers[s_ubLowerLayer+1].uwTop) {
		++s_ubLowerLayer;
	}

	// Transition between layers on screen
	const UWORD uwSeamPos = s_pLayers[s_ubLowerLayer].uwTop;
	if(uwCameraY < uwSeamPos && uwSeamPos < uwCameraY + s_uwVpHeight) {
		if(!s_isSet) {
			copBlockEnable(s_pCopList, s_pColorsBelow);
			layerCopyColorsToBlock(&s_pLayers[s_ubLowerLayer], s_pColorsBelow);

			copBlockEnable(s_pCopList, s_pColorsAbove);
			layerCopyColorsToBlock(&s_pLayers[s_ubLowerLayer - 1], s_pColorsAbove);
			copBlockWait(
				s_pCopList, s_pColorsAbove, 0xE2, s_uwVpStartY + s_uwVpHeight - 1
			);
			s_isSet = 1;
		}
		copBlockWait(
			s_pCopList, s_pColorsBelow, 0, s_uwVpStartY + uwSeamPos - uwCameraY
		);
	}
	else {
		if(s_isSet) {

			if(s_pDisableNext) {
				copBlockDisable(s_pCopList, s_pDisableNext);
				s_pDisableNext = 0;
				s_isSet = 0;
			}
			else {
				if(uwCameraY >= uwSeamPos) {
					copBlockDisable(s_pCopList, s_pColorsAbove);
					s_pDisableNext = s_pColorsBelow;
				}
				else {
					copBlockDisable(s_pCopList, s_pColorsBelow);
					s_pDisableNext = s_pColorsAbove;
				}
			}
		}
	}
}
