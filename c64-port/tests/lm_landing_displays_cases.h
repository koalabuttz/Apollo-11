/* SPDX-License-Identifier: MIT */
#ifndef LM_LANDING_DISPLAYS_CASES_H
#define LM_LANDING_DISPLAYS_CASES_H
#include "lm_landing_displays.h"
/* lm-landing-displays cases: case n enters entry slot n % 8 (LANDISP,
 * INTLZE, 21,2364, 21,2476, LANDISP, 21,2476, LANDISP, 21,2476) with FB bank
 * 21, EB 7, random channels 12 and 30 and channel 14 without the CDU and
 * gyro drive bits. The image is random; in three quarters of the cases every
 * word above the counters has a random bit width with +0 and -0 frequent.
 * The flag bits the file tests (FLAGWRD7 SWANDBIT, FLAGWRD0 R10FLBIT,
 * FLAGWRD1 DIDFLBIT, IMODES33 bits 7 and 8, channel 30 bit 6) are set or
 * clear one case in two, and LADQSAVE holds ALTROUT +1 or ALTOUT +1 in
 * three quarters of the cases. Each case runs through the trampoline (with
 * this file's routines only) until the original program would leave the
 * file. tools/lm_landing_displays_reference.c runs the same entry in the
 * original program. */
#define LM_LAD_ENTRIES 8u
#define LM_LAD_CASES 16384u
enum {LM_LAD_LEFT=0,LM_LAD_REFUSED=1,LM_LAD_NONE=2};
extern const agc_word lm_lad_entry_z[LM_LAD_ENTRIES];
void lm_lad_input(apollo_overlay_state *,lm_flight_io *,uint16_t n);
/* Runs case n natively; returns the outcome kind. */
unsigned lm_lad_run(apollo_overlay_state *,lm_flight_io *,uint16_t n,uint16_t *steps);
uint32_t lm_lad_digest(apollo_overlay_state *,uint16_t n);
#endif
