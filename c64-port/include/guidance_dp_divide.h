/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_DP_DIVIDE_H
#define GUIDANCE_DP_DIVIDE_H
#include "guidance_divide.h"
/* General DDV/BDDV for ordinary 15-bit operand words, DP MODE=+0.
 * MPAC and scratch words are ordinary 15-bit caller inputs; only operand
 * bits and MODE are checked at runtime. backward swaps the operand and MPAC roles, as BDDV. All scratch aliases
 * have the same ownership contract as the fixed-four-seconds routine.
 * Uses the original two-stage quotient approximation and near-one branch;
 * this is not a mathematically rounded division. Retains MPAC[2] on early
 * exits unless BUFZERO ran; successful division clears it. Leaves OVFIND
 * unchanged except on overflow, which writes 1. Preserves MPTEMP and all
 * other owned state; A/L and scratch retain the source sequence's debris.
 * Stops before DANZIG dispatch. Unsupported MODE/operand bits reject without
 * changing state. Caller excludes all producers and uses distinct objects. */
enum p66_divide_result p66_dp_divide(p66_preparation *state,
    p66_scale_scratch *scale,p66_elapsed_state *elapsed,
    p66_divide_scratch *scratch,agc_dp operand,uint8_t backward);
#endif
