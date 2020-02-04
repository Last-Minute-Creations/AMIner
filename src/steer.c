/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/utils/file.h>
#include "steer.h"

static UWORD s_uwSteerUnused = 0xFFFF;

void steerReset(void) {
	s_uwSteerUnused = 0xFFFF;
}

void steerUpdateFromInput(UBYTE is1pKbd, UBYTE is2pKbd) {
	g_uwSteer = 0;
	if(is1pKbd) {
		if(keyCheck(KEY_W)) { g_uwSteer |= STEER_P1_UP; }
		if(keyCheck(KEY_S)) { g_uwSteer |= STEER_P1_DOWN; }
		if(keyCheck(KEY_A)) { g_uwSteer |= STEER_P1_LEFT; }
		if(keyCheck(KEY_D)) { g_uwSteer |= STEER_P1_RIGHT; }
		if(keyCheck(KEY_SPACE)) { g_uwSteer |= STEER_P1_FIRE; }
	}
	else {
		if(joyCheck(JOY1_UP   )) { g_uwSteer |= STEER_P1_UP; }
		if(joyCheck(JOY1_DOWN )) { g_uwSteer |= STEER_P1_DOWN; }
		if(joyCheck(JOY1_LEFT )) { g_uwSteer |= STEER_P1_LEFT; }
		if(joyCheck(JOY1_RIGHT)) { g_uwSteer |= STEER_P1_RIGHT; }
		if(joyCheck(JOY1_FIRE)) { g_uwSteer |= STEER_P1_FIRE; }
	}

	if(is2pKbd) {
		if(keyCheck(KEY_UP)) { g_uwSteer |= STEER_P2_UP; }
		if(keyCheck(KEY_DOWN)) { g_uwSteer |= STEER_P2_DOWN; }
		if(keyCheck(KEY_LEFT)) { g_uwSteer |= STEER_P2_LEFT; }
		if(keyCheck(KEY_RIGHT)) { g_uwSteer |= STEER_P2_RIGHT; }
		if(keyCheck(KEY_RETURN) || keyCheck(KEY_NUMENTER)) { g_uwSteer |= STEER_P2_FIRE; }
	}
	else {
		if(joyCheck(JOY2_UP)) { g_uwSteer |= STEER_P2_UP; }
		if(joyCheck(JOY2_DOWN)) { g_uwSteer |= STEER_P2_DOWN; }
		if(joyCheck(JOY2_LEFT)) { g_uwSteer |= STEER_P2_LEFT; }
		if(joyCheck(JOY2_RIGHT)) { g_uwSteer |= STEER_P2_RIGHT; }
		if(joyCheck(JOY2_FIRE)) { g_uwSteer |= STEER_P2_FIRE; }
	}
	s_uwSteerUnused |= ~g_uwSteer;
}

UBYTE steerUpdateFromFile(tFile *pFile) {
	UBYTE isRead = fileRead(pFile, &g_uwSteer, sizeof(g_uwSteer));
	return isRead;
}

UBYTE steerUse(UWORD uwInput) {
	UBYTE isPressed = ((steerGet(uwInput) & s_uwSteerUnused) == uwInput);
	if(isPressed) {
		s_uwSteerUnused &= ~uwInput;
	}
	return isPressed;
}

UWORD g_uwSteer;
