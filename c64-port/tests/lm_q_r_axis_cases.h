/* SPDX-License-Identifier: MIT */
#ifndef LM_Q_R_AXIS_CASES_H
#define LM_Q_R_AXIS_CASES_H
#include "lm_q_r_axis.h"
/* lm-q-r-axis cases: case n enters entry slot n % 32 (CALLQERR, 17,2172,
 * 17,2214, 17,2454, 17,2620, QERRCALC, 17,2714, 17,2724, 17,2775, 17,3004,
 * 17,3042, 17,3074, 17,3103, JTLST, ROT-TOUV, 17,3224, CLOSEOUT, then
 * CALLQERR in the other fifteen slots) with FB bank 17, EB 6, random
 * channels 4, 13 and 31, an eight-bit channel 5 and a random
 * interrupt-enable bit. The image is random; in three quarters of the
 * cases every word above the counters has a random bit width with +0 and
 * -0 frequent. SKIPU and SKIPV are 0 or 4, RETJADR holds one of the file's
 * three return addresses in three quarters, and a return entry has AXISCTR
 * 0 or 1, NUMBERT 0-4 and SENSETYP 0-2 in most cases; one cohort in 32
 * keeps each of them random. A subroutine return in Q is SWRETURN, a
 * fixed-fixed word or a bank 17 word outside this file. Each case runs
 * through the trampoline (with this file's routines only) until the
 * original program would leave the file. tools/lm_q_r_axis_reference.c
 * runs the same entry in the original program. */
#define LM_QR_ENTRIES 32u
#define LM_QR_CASES 16384u
enum {LM_QR_LEFT=0,LM_QR_REFUSED=1,LM_QR_NONE=2};
extern const agc_word lm_qr_entry_z[LM_QR_ENTRIES];
void lm_qr_input(apollo_overlay_state *,lm_flight_io *,uint16_t n);
/* Runs case n natively; returns the outcome kind. */
unsigned lm_qr_run(apollo_overlay_state *,lm_flight_io *,uint16_t n,uint16_t *steps);
uint32_t lm_qr_digest(apollo_overlay_state *,uint16_t n);
#endif
