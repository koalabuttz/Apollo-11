/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_UNIT_H
#define GUIDANCE_UNIT_H
#include "guidance_norm.h"
#include "guidance_push.h"
enum p66_unit_result {P66_UNIT_OK=0,P66_UNIT_ALARM_1302=1,P66_UNIT_OUT_OF_SCOPE=2,P66_UNIT_OVERFLOW=3};
/* Ordinary-word MPAC inputs, exclusive ownership and excluded producers.
 * ABS accepts DP MODE +0; ABVAL and UNIT require vector MODE -1 and a valid
 * owned VAC. ABVAL stores raw length square at34/35 then returns DP length;
 * UNIT stores square34/35 and length36/37 and returns the historical HALF
 * unit vector. Overflow preserves the source route and sets OVFIND=1.
 * Negative square-root aborts stop before SQRTABRT with1302 observation.
 * No interpreter/executive control dispatch is performed. */
enum p66_norm_result p66_absolute_value(p66_preparation *);
enum p66_norm_result p66_vector_length(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
enum p66_unit_result p66_unit_vector(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
#endif
