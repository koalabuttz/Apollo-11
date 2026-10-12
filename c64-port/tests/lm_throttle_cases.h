/* SPDX-License-Identifier: MIT */
#ifndef LM_THROTTLE_CASES_H
#define LM_THROTTLE_CASES_H
#include "lm_throttle.h"
/* lm-throttle cases: case n enters entry n % 4 (THROTTLE, THROTTLE +3,
 * FLATOUT, WHATOUT) from a random image with FB bank 31 and EB 7, random channel 14 bits 1-5, A and L, MODREG 66 in a quarter of
 * the cases, and a caller return in Q (SWRETURN, a fixed-fixed address or a
 * bank-31 address outside the routines), and runs through the trampoline to
 * the caller's return. tools/lm_throttle_reference.c runs the same entry in
 * the original program. */
#define LM_THR_ENTRIES 4u
#define LM_THR_CASES 16384u
typedef struct {agc_word stop_z,stop_fb,superbank;} lm_thr_case;
extern const agc_word lm_thr_entry_z[LM_THR_ENTRIES];
void lm_thr_input(apollo_overlay_state *,lm_flight_io *,lm_thr_case *,uint16_t n);
/* Runs case n natively; returns 0 at the caller's return, 1 otherwise. */
unsigned lm_thr_run(apollo_overlay_state *,lm_flight_io *,uint16_t n,uint16_t *steps);
uint32_t lm_thr_digest(apollo_overlay_state *,uint16_t n);
#endif
