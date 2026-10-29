/* SPDX-License-Identifier: MIT */
#ifndef SBAND_VECTOR_LINEAR_H
#define SBAND_VECTOR_LINEAR_H
#include "fixed_data.h"
#include "overlay_runtime.h"
#include "agc_word.h"
enum sband_vector_operation {SBAND_VXSC,SBAND_VAD,SBAND_VSU};
/* Explicit original VXSC (MODE0 or -1), VAD/VSU (MODE-1) entry. A/L raw16,
 * ordinary image/Q, coherent banks/ZERO, and stable producer exclusion.
 * NULL operand selects live erasable words/aliases through ADDRWD and EB;
 * otherwise caller supplies actual immutable source fixed data, six words
 * (two used by vector-times-scalar), non-aliasing image, no bank crossing.
 * Stops before genuine mission DANZIG; no PUSH/VAC or external latch update.
 * Unsupported source context preserves the complete image. */
uint8_t sband_vector_linear(apollo_overlay_state *,enum apollo_fixed_mission,
                           enum sband_vector_operation,const agc_word *fixed_operand);
#endif
