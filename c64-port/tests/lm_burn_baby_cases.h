/* SPDX-License-Identifier: MIT */
#ifndef LM_BURN_BABY_CASES_H
#define LM_BURN_BABY_CASES_H
#include "lm_burn_baby.h"
/* lm-burn-baby cases: every routine of lm_burn_baby_routines and
 * lm_burn_baby_interpretive, LMBB_STATES states each, run from its key
 * until control leaves the file (include/lm_burn_baby.h).
 * tools/lm_burn_baby_reference.c runs the same image, channels and
 * interrupt state in the unmodified rope from the same Z and stops where
 * lmbb_leaves() says; the 2048 erasable words, channels 11, 14 and 30 and
 * the interrupt-enable state are compared. States are drawn mostly inside
 * the routines' domain (WHICH one of the six tables, DISPDEX one of the
 * display indexes, P40/RET one of NULLCLOK's returns, the PHASE words not
 * negative, KILLTASK under a caller's CADR); the rest exercise the
 * refusals, which tests/test_lm_burn_baby.c also checks. */
#define LMBB_STATES 128u
#define LMBB_CASES 20992u    /* LMBB_ROUTINES * LMBB_STATES */
#define LMBB_STOPPED 0u
#define LMBB_REFUSED 2u
typedef struct {
 agc_word superbank,chan11,chan14,chan30;
 uint16_t interrupt_enabled;
} lmbb_case;
void lmbb_input(apollo_overlay_state *,lmbb_case *,uint16_t n);
/* Whether the original has left the file at Z (FB, superbank, and LOC and
 * BANKSET at NEWOPS). */
int lmbb_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset);
/* Runs case n natively; returns LMBB_STOPPED, LMBB_REFUSED or 3 (no stop).
 * io (optional) receives the final channels and interrupt state. */
unsigned lmbb_run(apollo_overlay_state *,uint16_t n,lmbb_case *io);
uint32_t lmbb_digest(apollo_overlay_state *,uint16_t n);
#endif
