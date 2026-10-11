/* SPDX-License-Identifier: MIT */
#ifndef SBAND_DOT_H
#define SBAND_DOT_H
#include "sband_vector_linear.h"
/* Canonical original vector DOT entry, MODE-1, ordinary RAM/Q and raw16 A/L.
 * Same live ERA/nonaliasing immutable six-word fixed operand domain as the
 * linear vector kernels. Producer exclusion required. Stops before actual
 * mission DANZIG with scalar MODE+0; source scratch/TP result/Q preserved.
 * No instruction decoder, source scheduler or external latch update. */
uint8_t sband_dot(apollo_overlay_state *,enum apollo_fixed_mission,const agc_word *);
#endif
