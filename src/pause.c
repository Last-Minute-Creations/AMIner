#include "pause.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include "hud.h"
#include "game.h"
#include "core.h"

void pauseGsCreate(void) {
	hudPause(1);
}

void pauseGsLoop(void) {
	UBYTE ubKey = 0;
	if(g_is1pKbd) {
		if(keyUse(KEY_D)) { ubKey = KEY_RIGHT; }
		if(keyUse(KEY_A)) { ubKey = KEY_LEFT; }
	}
	else {
		if(joyUse(JOY1_RIGHT)) { ubKey = KEY_RIGHT; }
		if(joyUse(JOY1_LEFT)) { ubKey = KEY_LEFT; }
		if(joyUse(JOY1_FIRE)) { ubKey = KEY_RETURN; }
	}
	if(keyUse(KEY_RETURN)) { ubKey = KEY_RETURN; }

	if(g_is2pPlaying) {
		if(g_is2pKbd) {
			if(keyUse(KEY_RIGHT)) { ubKey = KEY_RIGHT; }
			if(keyUse(KEY_LEFT)) { ubKey = KEY_LEFT; }
		}
		else {
			if(joyUse(JOY2_RIGHT)) { ubKey = KEY_RIGHT; }
			if(joyUse(JOY2_LEFT)) { ubKey = KEY_LEFT; }
			if(joyUse(JOY2_FIRE)) { ubKey = KEY_RETURN; }
		}
	}

	if(ubKey == KEY_LEFT) {
		hudSelect(0);
	}
	else if(ubKey == KEY_RIGHT) {
		hudSelect(1);
	}
	else if(ubKey == KEY_RETURN) {

		if(hudGetSelection() == 0) {
			gameChangeState(gameGsCreate, gameGsLoop, gameGsDestroy);
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
	vPortWaitForEnd(pView);
}

void pauseGsDestroy(void) {
	hudPause(0);
}
