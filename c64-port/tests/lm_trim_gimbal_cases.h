/* SPDX-License-Identifier: MIT */
#ifndef LM_TRIM_GIMBAL_CASES_H
#define LM_TRIM_GIMBAL_CASES_H
#include "lm_trim_gimbal.h"
/* lm-trim-gimbal cases: case n enters entry n % 4 (GTS, ACDT+C12, TIMEGMBL,
 * 21,3435) from a random image with FB bank 21 (bank 16 for ACDT+C12) and
 * EB 6 and a random channel 12. In fifteen sixteenths of the cases the
 * trim gimbal inputs (AOSQ/R, EDOTQ/R, KQ/R, QDIFF/RDIFF, ACCDOTQ/R,
 * NEGUQ/R) have random bit widths (KQ/R and ACCDOTQ/R positive except in
 * one sixteenth), and SR is below 2^13 in three quarters of those.
 * Each case runs through the trampoline (with the native inter-bank
 * routines) until the original program would leave this file: GTS's DTCB
 * to CLOSEOUT, ACDT+C12's return to Q, TIMEGMBL's IBNKCALL arriving at
 * ACDT+C12 (whose own return through ISWRETRN is the lane A kernel's), or
 * 21,3435's TC RUPTREG2. tools/lm_trim_gimbal_reference.c runs the same
 * entry in the original program. */
#define LM_TG_ENTRIES 4u
#define LM_TG_CASES 16384u
enum {LM_TG_RETURNED=0,LM_TG_CLOSED=1,LM_TG_REFUSED=2,LM_TG_NONE=3};
typedef struct {agc_word stop_z,stop_fb,superbank;} lm_tg_case;
extern const agc_word lm_tg_entry_z[LM_TG_ENTRIES];
#define LM_TG_SITES 1u
extern const agc_site_operand lm_tg_sites[LM_TG_SITES];
void lm_tg_input(apollo_overlay_state *,lm_flight_io *,lm_tg_case *,uint16_t n);
/* Runs case n natively; returns the outcome kind (LM_TG_REFUSED: GTS
 * refused with the image unchanged; LM_TG_NONE: none). */
unsigned lm_tg_run(apollo_overlay_state *,lm_flight_io *,uint16_t n,uint16_t *steps);
uint32_t lm_tg_digest(apollo_overlay_state *,uint16_t n);
#endif
