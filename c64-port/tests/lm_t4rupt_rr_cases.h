/* SPDX-License-Identifier: MIT */
#ifndef LM_T4RUPT_RR_CASES_H
#define LM_T4RUPT_RR_CASES_H
#include "lm_t4rupt_rr.h"
#include "p66_bias.h"
/* lm-t4rupt-rr cases: 2,048 states at RRAUTCHK and 128 at each other entry
 * of lm_t4rupt_rr_routines (3,456), entered at the original address and run
 * for one routine; tools/lm_t4rupt_rr_reference.c runs the original program
 * from the same entry until it first reaches the continuation the native
 * run left in Z (and FB in banked memory). */
#define LRR_FIRST_CASES 2048u
#define LRR_PER_ENTRY 128u
#define LRR_CASES (LRR_FIRST_CASES+(LM_T4RUPT_RR_ENTRIES-1u)*LRR_PER_ENTRY)
#define LRR_RETURNED 0u
#define LRR_REFUSED 2u
typedef struct {p66_bias_io lm;agc_hardware hw;} lrr_io;
agc_word lrr_entry(uint16_t n);
void lrr_input(apollo_overlay_state *,lrr_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not entered). */
unsigned lrr_run(apollo_overlay_state *,lrr_io *,uint16_t n);
uint32_t lrr_digest(apollo_overlay_state *,uint16_t n);
#endif
