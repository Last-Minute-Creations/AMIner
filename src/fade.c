#include "fade.h"

static tFadeState s_eState = FADE_STATE_IN_MORPHING;
static UBYTE s_ubLevel = 0;

tFadeState fadeGetState(void) {
	return s_eState;
}

UBYTE fadeGetLevel(void) {
	return s_ubLevel;
}

void fadeMorphTo(tFadeState eState) {
	if(eState == FADE_STATE_IN) {
		s_eState = FADE_STATE_IN_MORPHING; // set transition
	}
	else {
		s_eState = FADE_STATE_OUT_MORPHING;
	}
}

void fadeProcess(void) {
	if(s_eState == FADE_STATE_OUT_MORPHING) {
		if(s_ubLevel > 0) {
			--s_ubLevel;
		}
		else {
			s_eState = FADE_STATE_OUT;
		}
	}
	else if(s_eState == FADE_STATE_IN_MORPHING) {
		if(s_ubLevel < 0xF) {
			++s_ubLevel;
		}
		else {
			s_eState = FADE_STATE_IN;
		}
	}
}
