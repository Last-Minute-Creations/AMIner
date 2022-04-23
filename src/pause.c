#include "pause.h"
#include "hud.h"
#include "game.h"
#include "core.h"
#include "menu.h"
#include "steer.h"

void pauseGsCreate(void) {
	hudPause(1);
}

void pauseGsLoop(void) {
	steerUpdateFromInput(g_is1pKbd, g_is2pKbd);

	if(steerGet(STEER_P1_LEFT) || steerGet(STEER_P2_LEFT)) {
		hudSelect(0);
	}
	else if(steerGet(STEER_P1_RIGHT) || steerGet(STEER_P2_RIGHT)) {
		hudSelect(1);
	}
	else if(steerGet(STEER_P1_FIRE) || steerGet(STEER_P2_FIRE)) {
		if(hudGetSelection() == 0) {
			stateChange(g_pGameStateManager, &g_sStateGame);
		}
		else {
			menuGsEnter(0);
		}
	}
	hudUpdate();

	tView *pView = g_pMainBuffer->sCommon.pVPort->pView;
	tVPort *pVpHud = pView->pFirstVPort;
	vPortProcessManagers(pVpHud);
	copProcessBlocks();
	vPortWaitForEnd(pVpHud->pNext);
}

void pauseGsDestroy(void) {
	hudPause(0);
}

tState g_sStatePause = {
	.cbCreate = pauseGsCreate, .cbLoop = pauseGsLoop, .cbDestroy = pauseGsDestroy
};
