/* SPDX-License-Identifier: MIT */
#ifndef LM_AGS_INIT_CASES_H
#define LM_AGS_INIT_CASES_H
#include "lm_ags_init.h"
/* lm-ags-init cases: every routine of lm_ags_init_routines and
 * lm_ags_init_interpretive, AGS_STATES states each, run from its key until
 * control leaves the file (include/lm_ags_init.h).
 * tools/lm_ags_init_reference.c runs the same image and interrupt state in
 * the unmodified rope from the same Z and stops where ags_leaves() says;
 * the 2048 erasable words and the interrupt-enable state are compared.
 * States are drawn inside the routines' domain; the refusals are exercised
 * by tests/test_lm_ags_init.c. */
#define AGS_STATES 768u
#define AGS_CASES 29952u     /* AGS_ROUTINES * AGS_STATES */
#define AGS_STOPPED 0u
#define AGS_REFUSED 2u
typedef struct {
 agc_word superbank;
 uint16_t interrupt_enabled;
} ags_case;
/* The QPRET each CALL SCALEVEC stores (at 32,2053 and 32,2061): checked
 * against lmi_call by the unit test. */
#define AGS_QPRET_2053 064055u
#define AGS_QPRET_2061 064063u
void ags_input(apollo_overlay_state *,ags_case *,uint16_t n);
/* Whether the original has left the file at Z (FB, superbank, and LOC and
 * BANKSET at NEWOPS). */
int ags_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset);
/* Runs case n natively; returns AGS_STOPPED, AGS_REFUSED or 3 (no stop). */
unsigned ags_run(apollo_overlay_state *,uint16_t n,uint16_t *interrupt_enabled);
uint32_t ags_digest(apollo_overlay_state *,uint16_t n);
#endif
