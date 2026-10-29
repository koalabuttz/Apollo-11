/* SPDX-License-Identifier: MIT */
#ifndef LM_SERVICER_CASES_H
#define LM_SERVICER_CASES_H
#include "lm_servicer.h"
/* lm-servicer cases: every routine of lm_servicer_routines and
 * lm_servicer_interpretive, LMSV_STATES states each, run from its key until
 * control leaves the file (include/lm_servicer.h).
 * tools/lm_servicer_reference.c runs the same image, channels and interrupt
 * state in the unmodified rope from the same Z and stops where lmsv_leaves()
 * says; the 2048 erasable words, channels 11, 12, 32 and 33 and the
 * interrupt-enable state are compared. States are drawn mostly inside the
 * routines' domain (the EBANK the code runs under, the indexes INDEX reads
 * one of the values their producers store, LRADRET a return, a work area
 * at the interpretive keys); the rest exercise the refusals, which
 * tests/test_lm_servicer.c also checks. */
#define LMSV_STATES 128u
#define LMSV_BASE_CASES 40320u    /* LMSV_ROUTINES * LMSV_STATES */
/* Then the continuations (tests/lm_servicer_chains.h, from
 * tools/lm_servicer_chains.py): case LMSV_BASE_CASES + j starts where
 * case lmsv_chain_base[j]'s run reaches the end of its C64 segment group,
 * with the image that run leaves, at the key there; it runs the rest of
 * the original case in the group that holds that code. */
#define LMSV_CASES 40336u
#define LMSV_STOPPED 0u
#define LMSV_REFUSED 2u
typedef struct {
 agc_word superbank,chan11,chan12,chan32,chan33;
 uint16_t interrupt_enabled;
} lmsv_case;
void lmsv_input(apollo_overlay_state *,lmsv_case *,uint16_t n);
/* The key slot (lmsv_routine_keys) case n starts at. */
unsigned lmsv_slot(uint16_t n);
/* Whether the original has left the file at Z (FB, superbank, Q, and LOC
 * and BANKSET at NEWOPS). */
int lmsv_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word q,agc_word loc,agc_word bankset);
/* Runs case n natively; returns LMSV_STOPPED, LMSV_REFUSED, 3 (no stop) or
 * 4 (no routine has the key: a C64 group build without it).
 * io (optional) receives the final channels and interrupt state. */
unsigned lmsv_run(apollo_overlay_state *,uint16_t n,lmsv_case *io);
uint32_t lmsv_digest(apollo_overlay_state *,uint16_t n);
#endif
