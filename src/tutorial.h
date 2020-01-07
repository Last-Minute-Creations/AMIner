/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TUTORIAL_H_
#define _TUTORIAL_H_

#include <ace/types.h>
#include "string_array.h"

/**
 * @brief Resets tutorial state.
 */
void tutorialReset(void);

/**
 * @brief Process tutorial.
 *
 * @return 1 if game loop should return early, otherwise 0.
 */
UBYTE tutorialProcess(void);

extern tStringArray g_sTutorialMsgs;

#endif // _TUTORIAL_H_
