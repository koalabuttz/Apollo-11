/* SPDX-License-Identifier: MIT */
#ifndef LM_T4RUPT_CHAIN_CASES_H
#define LM_T4RUPT_CHAIN_CASES_H
#include "t4rupt_core.h"
#include "lm_t4rupt_imu.h"
#include "lm_t4rupt_rr.h"
#include "p66_bias.h"
/* lm-t4rupt-chain cases: 4,096 T4RUPT interrupts of Luminary099 run through
 * the trampoline (agc_transfer_run) over the three routine tables of
 * T4RUPT_PROGRAM.agc (lm_t4rupt_core_routines, lm_t4rupt_imu_routines,
 * lm_t4rupt_rr_routines) from T4RUPT 06,2000 until control leaves the file
 * (RESUME, NOVAC, ALARM, WAITLIST, IBNKCALL, ...: the first unmapped
 * continuation). tools/lm_t4rupt_chain_reference.c runs the original
 * program from T4RUPT until it first reaches that continuation. */
#define LCH_CASES 4096u
#define LCH_RETURNED 0u
#define LCH_REFUSED 2u
#define LCH_MAX_STEPS 24u
typedef struct {p66_bias_io lm;agc_hardware hw;} lch_io;
void lch_input(apollo_overlay_state *,lch_io *,uint16_t n);
/* Runs case n through the trampoline; returns the outcome kind and the
 * number of routines run. */
unsigned lch_run(apollo_overlay_state *,lch_io *,uint16_t n,uint16_t *steps);
uint32_t lch_digest(apollo_overlay_state *,uint16_t n);
#endif
