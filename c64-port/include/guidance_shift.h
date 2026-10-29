/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_SHIFT_H
#define GUIDANCE_SHIFT_H
#include "guidance_elapsed.h"
/* Explicit native shift operations, not interpreter opcodes. All operands
 * are ordinary words; MODE must be DP +0 or vector -1 as appropriate.
 * General counts 0..14; short scalar counts 1..4, vector 1..8. Scalar right
 * preserves the triple low word; vector right rounds and clears it. Left
 * shifts preserve original overflow correction rather than saturating.
 * Counts zero retain scratch and L while clearing A. General SR14 leaves
 * L=03400 and MPTEMP=-0; vector SR14 retains its last rounded low scratch.
 * Invalid mode/count leaves all objects unchanged and returns 0. Distinct
 * owned objects and producer exclusion are caller obligations. */
uint8_t p66_scalar_shift_left(p66_preparation *,p66_scale_scratch *,uint8_t count,uint8_t short_form);
uint8_t p66_scalar_shift_right(p66_preparation *,p66_scale_scratch *,uint8_t count,uint8_t short_form);
uint8_t p66_vector_shift_left(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,uint8_t count,uint8_t short_form);
uint8_t p66_vector_shift_right(p66_preparation *,p66_scale_scratch *,uint8_t count,uint8_t short_form);
#endif
