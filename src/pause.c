#include "pause.h"
#include <ace/managers/key.h>
#include "hud.h"
#include "game.h"
#include "core.h"
#include "menu.h"
#include "steer.h"
#include "settings.h"

void pauseGsCreate(void) {
	hudPause(1);
}

void pauseGsLoop(void) {
	tSteer *pSteers = gameGetSteers();
	steerProcess(&pSteers[0]);
	steerProcess(&pSteers[1]);

	if(
		steerDirUse(&pSteers[0], DIRECTION_LEFT) ||
		(g_is2pPlaying && steerDirUse(&pSteers[1], DIRECTION_LEFT))
	) {
		hudSelect(0);
	}
	else if(
		steerDirUse(&pSteers[0], DIRECTION_RIGHT) ||
		(g_is2pPlaying && steerDirUse(&pSteers[1], DIRECTION_RIGHT))
	) {
		hudSelect(1);
	}
	else if(
		steerDirUse(&pSteers[0], DIRECTION_FIRE) ||
		(g_is2pPlaying && steerDirUse(&pSteers[1], DIRECTION_FIRE)) ||
		keyUse(KEY_SPACE) || keyUse(KEY_RETURN)
	) {
		if(hudGetSelection() == 0) {
			stateChange(g_pGameStateManager, &g_sStateGame);
		}
		else {
			if(g_eGameMode != GAME_MODE_CHALLENGE) {
				gameTriggerSave();
			}
			menuGsEnter(0);
		}
	}
	hudProcess();

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
