/* SPDX-License-Identifier: MIT */
#ifndef LM_LUNAR_LANDING_CASES_H
#define LM_LUNAR_LANDING_CASES_H
#include "lm_lunar_landing.h"
/* lm-lunar-landing cases: every routine of lm_lunar_landing_routines and
 * lm_lunar_landing_interpretive, LLD_STATES states each, run from its key
 * until control leaves the file (include/lm_lunar_landing.h).
 * tools/lm_lunar_landing_reference.c runs the same image, channels and
 * interrupt state in the unmodified rope from the same Z and stops where
 * lld_leaves() says; the 2048 erasable words, channels 12 and 33 and the
 * interrupt-enable state are compared. States are drawn inside the
 * routines' domain; the refusals are exercised by
 * tests/test_lm_lunar_landing.c. */
#define LLD_STATES 512u
#define LLD_CASES 15872u     /* LLD_ROUTINES * LLD_STATES */
#define LLD_STOPPED 0u
#define LLD_REFUSED 2u
typedef struct {
 agc_word superbank,chan12,chan33;
 uint16_t interrupt_enabled;
} lld_case;
void lld_input(apollo_overlay_state *,lld_case *,uint16_t n);
/* Whether the original has left the file at Z (FB, superbank, and LOC and
 * BANKSET at NEWOPS). */
int lld_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset);
/* Runs case n natively; returns LLD_STOPPED, LLD_REFUSED or 3 (no stop).
 * io (optional) receives the final channels and interrupt state. */
unsigned lld_run(apollo_overlay_state *,uint16_t n,lld_case *io);
uint32_t lld_digest(apollo_overlay_state *,uint16_t n);
#endif
