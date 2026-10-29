/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_THROTTLE_PHASED_H
#define GUIDANCE_THROTTLE_PHASED_H
#include "guidance_throttle.h"
/* Fixed overlay phases; canonical producer exclusion spans all calls.
 * Numerical boundaries LOC03436,03437,03446,03464,03472,03507.
 * Length ends after ABVAL with the paired PDDL still pending in EDOP;
 * other endpoints have EDOP0. No interpreter execution or scheduler yield.
 * MODE -1 for vector/length, MODE0 afterward. PUSH offsets0/0/0/2/6/0.
 * Caller supplies ordinary words, distinct owned objects and stable inputs.
 * Invalid phase ownership rejects unchanged; square-root alarm terminates.
 */
enum p66_feedback_status p66_feedback_vector_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]);
enum p66_feedback_status p66_feedback_length_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]);
enum p66_feedback_status p66_feedback_elapsed_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]);
enum p66_feedback_status p66_feedback_correction_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]);
enum p66_feedback_status p66_feedback_limits_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]);
enum p66_feedback_status p66_feedback_clamp_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]);
#endif
