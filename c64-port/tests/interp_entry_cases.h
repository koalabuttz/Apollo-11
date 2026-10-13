/* SPDX-License-Identifier: MIT */
#ifndef INTERP_ENTRY_CASES_H
#define INTERP_ENTRY_CASES_H
#include "standard_lead_routines.h"
#include "interp_entry.h"
/* interp-entry cases: 512 states at each of INTPRET, INTPRET +2 and INTRSM
 * of lm_interpreter_routines (mission 0) and cm_interpreter_routines
 * (mission 1), 1,536 per mission, entered at the original address and run
 * for one routine; tools/interp_entry_reference.c runs the original program
 * from the same entry until it first reaches NEWOPS. */
#define IE_PER_ENTRY 512u
#define IE_CASES 1536u
#define IE_RETURNED 0u
#define IE_REFUSED 2u
typedef struct {p66_bias_io lm;cm_native_io cm;} ie_io;
agc_word ie_entry(unsigned mission,uint16_t n);
void ie_input(unsigned mission,apollo_overlay_state *,ie_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not entered). */
unsigned ie_run(unsigned mission,apollo_overlay_state *,ie_io *,uint16_t n);
uint32_t ie_digest(unsigned mission,apollo_overlay_state *,uint16_t n);
#endif
