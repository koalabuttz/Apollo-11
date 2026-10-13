/* SPDX-License-Identifier: MIT */
#ifndef CM_LEAD_TESTS_CASES_H
#define CM_LEAD_TESTS_CASES_H
/* Cases for Comanche055's system-test entry words (milestone cm-lead-tests):
 * COMPVER, GTSCPSS1 and REDO at 33,2000-2002, entered with FB 033 (case
 * n%3 picks the word), random erasable (L sign-extended), any A and Q,
 * superbank 0 or 0060. clt_run
 * takes the trampoline from the word to its unmapped TC target. */
#include "agc_transfer.h"
#define CLT_CASES 384u
void clt_input(apollo_overlay_state *s, uint16_t *superbank, uint16_t n);
unsigned clt_run(apollo_overlay_state *s, uint16_t *superbank, uint16_t n);
uint32_t clt_digest(apollo_overlay_state *s, uint16_t n);
#endif
