/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ace/contrib/managers/audio_mixer.h>
#include <ace/managers/memory.h>
#include <ace/managers/system.h>
#include <hardware/intbits.h>
#include <hardware/dmabits.h>
#include "mixer.h"

typedef void (*tCbMixerIrqHandler)();

static ULONG s_ulBufferSize;
static void *s_pBuffer;
static tCbMixerIrqHandler s_cbMixerIrqHandler;

static void onChannelInterrupt(
	UNUSED_ARG REGARG(volatile tCustom *pCustom, "a0"),
	UNUSED_ARG REGARG(volatile void *pData, "a1")
) {
	s_cbMixerIrqHandler();
}

//---------------------------------------------------------------- ASM CALLBACKS

__attribute__((used))
void mixerSetIrqVector(void) {
	register volatile void *reg_irqHandler __asm("a0");
	s_cbMixerIrqHandler = reg_irqHandler;
}

__attribute__((used))
void mixerSetIrqBits(void) {
	register volatile ULONG reg_intena __asm("d0");
	UWORD uwIntena = reg_intena;
	if(uwIntena & INTF_AUD0) {
		systemSetInt(INTB_AUD0, onChannelInterrupt, 0);
	}
	if(uwIntena & INTF_AUD1) {
		systemSetInt(INTB_AUD1, onChannelInterrupt, 0);
	}
	if(uwIntena & INTF_AUD2) {
		systemSetInt(INTB_AUD2, onChannelInterrupt, 0);
	}
	if(uwIntena & INTF_AUD3) {
		systemSetInt(INTB_AUD3, onChannelInterrupt, 0);
	}
}

__attribute__((used))
void mixerRemoveIrqVectorOrClearIrqBits(void) {
	register volatile ULONG reg_intena __asm("d0");
	UWORD uwIntena = reg_intena;
	if(uwIntena & INTF_AUD0) {
		systemSetInt(INTB_AUD0, 0, 0);
	}
	if(uwIntena & INTF_AUD1) {
		systemSetInt(INTB_AUD1, 0, 0);
	}
	if(uwIntena & INTF_AUD2) {
		systemSetInt(INTB_AUD2, 0, 0);
	}
	if(uwIntena & INTF_AUD3) {
		systemSetInt(INTB_AUD3, 0, 0);
	}
}

__attribute__((used))
void mixerAcknowledgeIrq(void) {
	// done by ACE's interrupt handler system
}

__attribute__((used))
void mixerSetDmaCon(void) {
	register volatile ULONG reg_dmacon __asm("d0");
	UWORD uwDmaCon = reg_dmacon;
	systemSetDmaMask(uwDmaCon, BTST(uwDmaCon, DMAB_SETCLR));
}

static MXIRQDMACallbacks s_sCallbacks = {
	.mxicb_set_irq_vector = mixerSetIrqVector,
	.mxicb_remove_irq_vector = mixerRemoveIrqVectorOrClearIrqBits,
	.mxicb_set_irq_bits = mixerSetIrqBits,
	.mxicb_disable_irq = mixerRemoveIrqVectorOrClearIrqBits,
	.mxicb_acknowledge_irq = mixerAcknowledgeIrq,
	.mxicb_set_dmacon = mixerSetDmaCon,
};

//------------------------------------------------------------------- PUBLIC FNS

void audioMixerCreate(void) {
	s_ulBufferSize = MixerGetBufferSize();
	s_pBuffer = memAllocChip(s_ulBufferSize);

	MixerSetup(s_pBuffer, 0, 0, MIX_PAL, 0);
	MixerSetIRQDMACallbacks(&s_sCallbacks);
	MixerInstallHandler(0, 0);
	MixerStart();
}

void audioMixerDestroy(void) {
	MixerStop();
	MixerRemoveHandler();

	memFree(s_pBuffer, s_ulBufferSize);
}

void audioMixerPlaySfx(
	const tPtplayerSfx *pSfx, UBYTE ubMixerChannel, WORD wPriority, UBYTE isLoop
) {
	MixerPlayChannelSample(
		pSfx->pData, DMAF_AUD3 | (MIX_CH0 << ubMixerChannel),
		pSfx->uwWordLength * sizeof(UWORD), wPriority,
		isLoop ? MIX_FX_LOOP : MIX_FX_ONCE, 0
	);
}

void audioMixerStopSfxOnChannel(UBYTE ubMixerChannel) {
	MixerStopFX(DMAF_AUD3 | (MIX_CH0 << ubMixerChannel));
}

UBYTE audioMixerIsPlaybackDone(void) {
	return MixerGetChannelStatus(0) == MIX_CH_FREE;
}
