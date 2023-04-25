/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ace_mixer.h"
#include <ace/managers/memory.h>
#include <ace/managers/system.h>
#include <hardware/intbits.h>
#include <audio_mixer/mixer_control.h>

extern MXMixer mixer;
static ULONG s_ulBufferSize;
static void *s_pBuffer;

static void onChannelInterrupt(
	UNUSED_ARG REGARG(volatile tCustom *pCustom, "a0"),
	UNUSED_ARG REGARG(volatile void *pData, "a1")
) {
	MixerIRQHandler();
}

void audioMixerCreate(void) {
	s_ulBufferSize = MixerGetBufferSize();
	s_pBuffer = memAllocChip(s_ulBufferSize);

	MixerSetup(s_pBuffer, MIX_PAL);
	// MixerInstallHandler(0, 0); // todo: remove

	// Install interrupt handlers
	// systemSetInt(INTB_AUD0, onChannelInterrupt, 0);
	// systemSetInt(INTB_AUD1, onChannelInterrupt, 0);
	// systemSetInt(INTB_AUD2, onChannelInterrupt, 0);
	systemSetInt(INTB_AUD3, onChannelInterrupt, 0);
	mixer.mx_status = MIXER_RUNNING;
	mixer.mx_irq_bits = INTF_AUD3; // TODO: sync with config.mixer_output_channels

	MixerStart();
}

void audioMixerDestroy(void) {
	MixerStop();

	// Uninstall interrupt handlers
	mixer.mx_status = MIXER_STOPPED;
	systemSetInt(INTB_AUD3, 0, 0);

	memFree(s_pBuffer, s_ulBufferSize);
}
