/* SPDX-License-Identifier: MIT */
#ifndef CM_T4RUPT_CHAIN_CASES_H
#define CM_T4RUPT_CHAIN_CASES_H
#include "t4rupt_core.h"
#include "cm_t4rupt_imu.h"
#include "cm_t4rupt_optics.h"
#include "agc_input_channels.h"
/* cm-t4rupt-chain cases: 4,096 T4RUPT interrupts of Comanche055 run through
 * the trampoline (agc_transfer_run) over the three routine tables of
 * T4RUPT_PROGRAM.agc (cm_t4rupt_core_routines, cm_t4rupt_imu_routines,
 * cm_t4rupt_optics_basic) from T4RUPT 06,2000 until control leaves the file
 * (RESUME, NOVAC, IBNKCALL, ALARM, WAITLIST, ...: the first unmapped
 * continuation). tools/cm_t4rupt_chain_reference.c runs the original
 * program from T4RUPT until it first reaches that continuation.
 *
 * Channels: the core and IMU tables keep channels 10-14, 30 and 33 in
 * agc_hardware; the optics routines keep 11-14 in cm_native_io and 30-33 in
 * cm_channels (Lane G's record, until the shared channel model reaches
 * them). Each run starts both copies equal. T4JUMP runs at most one monitor
 * per interrupt, so at most one copy changes, and cch_channel() reads the
 * one that did. */
#define CCH_CASES 4096u
#define CCH_RETURNED 0u
#define CCH_REFUSED 2u
#define CCH_MAX_STEPS 24u
typedef struct {cm_channel_io cm;agc_hardware hw,in;} cch_io;
void cch_input(apollo_overlay_state *,cch_io *,uint16_t n);
/* Runs case n through the trampoline; returns the outcome kind (3: ran past
 * CCH_MAX_STEPS routines, 4: both channel copies changed) and the number of
 * routines run. */
unsigned cch_run(apollo_overlay_state *,cch_io *,uint16_t n,uint16_t *steps);
/* Channel k (010-014, 030-033) after a run. */
agc_word cch_channel(const cch_io *,unsigned k);
uint32_t cch_digest(apollo_overlay_state *,uint16_t n);
#endif
