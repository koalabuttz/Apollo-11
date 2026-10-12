/* SPDX-License-Identifier: MIT */
#ifndef LM_P_AXIS_CASES_H
#define LM_P_AXIS_CASES_H
#include "lm_p_axis.h"
/* lm-p-axis cases: case n enters entry n % 16 (PAXIS, 16,2223, 16,2224,
 * BACKP, 16,2653, SUPERJOB, 16,2773, 16,3154, 16,3403, 16,3424, 16,3443,
 * CALCPERR, 16,3500, 16,3512, 16,3550, and BACKP again) with FB bank 16,
 * EB 6, random channels 12, 13 and 31 and a random interrupt-enable bit.
 * The image is random; in three quarters of the cases every word above the
 * counters has a random bit width with +0 and -0 frequent, and a 16,3403
 * entry has ROTINDEX 0 or 1 in seven eighths, and OLDPMIN is -0 only in
 * one cohort in 32. A CALCPERR return is
 * SWRETURN, a fixed-fixed word or a bank 16 word outside this file.
 * Each case runs through the trampoline (with this file's routines only)
 * until the original program would leave the file: a TC, TCF, DTCB or
 * TC Q whose destination is outside 16,2213-3640. tools/lm_p_axis_reference.c
 * runs the same entry in the original program. */
#define LM_PA_ENTRIES 16u
#define LM_PA_CASES 16384u
enum {LM_PA_LEFT=0,LM_PA_REFUSED=1,LM_PA_NONE=2};
extern const agc_word lm_pa_entry_z[LM_PA_ENTRIES];
void lm_pa_input(apollo_overlay_state *,lm_flight_io *,uint16_t n);
/* Runs case n natively; returns the outcome kind. */
unsigned lm_pa_run(apollo_overlay_state *,lm_flight_io *,uint16_t n,uint16_t *steps);
uint32_t lm_pa_digest(apollo_overlay_state *,uint16_t n);
#endif
