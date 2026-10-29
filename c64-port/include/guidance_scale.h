/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_SCALE_H
#define GUIDANCE_SCALE_H
#include "guidance_prepare.h"
/* Interpreter arithmetic scratch; MPAC, MODE and A/L alias preparation state. */
typedef struct { agc_word mptemp, ovfind; } p66_scale_scratch;
/* KPIP1 is the historical DP pair {00321,26706}, approximately .0128.
 * Requires vector MODE == -1 in mpac[7]. Scales the three DP pairs at
 * +0/+1, +3/+4, +5/+6. Preserves all other preparation words; clears +2
 * through VROUND, retains OVFIND unless rounding overflows, and exposes
 * final A/L exchange debris. Own both objects and exclude producers.
 * Stops at DANZIG before PDDL; no interpreter or scheduler dispatch. */
void p66_scale_velocity(p66_preparation *state, p66_scale_scratch *scratch);
#endif
