/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ACE_MIXER_H
#define ACE_MIXER_H

#include <ace/managers/ptplayer.h>

void audioMixerCreate(void);

void audioMixerDestroy(void);

/**
 * @brief Plays given sound effect on selected mixer-enabled audio channel.
 *
 * @param pSfx Sound effect to play.
 * @param ubChannel Hardware audio channel to use.
 * @param wPriority Playback priority. Bigger is more important.
 * @param isLoop Set to 1 to play sound effect in a loop, zero for one-shot.
 */
void audioMixerPlaySfx(
	const tPtplayerSfx *pSfx, UBYTE ubChannel, WORD wPriority, UBYTE isLoop
);

#endif // ACE_MIXER_H
