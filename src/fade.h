#ifndef _FADE_H_
#define _FADE_H_

#include <ace/types.h>

typedef enum _tFadeState {
	FADE_STATE_IN_MORPHING,
	FADE_STATE_IN,
	FADE_STATE_OUT_MORPHING,
	FADE_STATE_OUT,
} tFadeState;

tFadeState fadeGetState(void);

UBYTE fadeGetLevel(void);

UWORD fadeGetSecondaryColor(void);

void fadeMorphTo(tFadeState eState, UWORD uwSecondaryColor);

void fadeProcess(void);


#endif // _FADE_H_
