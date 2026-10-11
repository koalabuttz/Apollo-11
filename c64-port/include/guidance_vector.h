/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_VECTOR_H
#define GUIDANCE_VECTOR_H
#include "guidance_elapsed.h"
#define P66_VECTOR_OUT_OF_SCOPE UINT16_C(0100000)
/* VAD/VSU with a stable ordinary six-word vector operand. Requires vector
 * MODE=-1, distinct owned state/scratch/operand objects and excluded producers.
 * MPAC pairs +3/+4,+5/+6,+0/+1 are processed in historical Y,Z,X order.
 * Preserve MPAC+2/MODE, operand, MPTEMP, PIPTIME and all unrelated preparation
 * words. BUF and OVFIND retain the original OVERFLOW effects; A/L retain the
 * final component's debris. Stop at DANZIG before interpreter dispatch.
 * Bits 0..2,3..5,6..8 describe X,Y,Z respectively: initial DAS overflow,
 * low TS 7 skip, high TS 7 skip. Unsupported MODE returns OUT_OF_SCOPE and
 * leaves every object unchanged. Other preconditions are caller obligations. */
uint16_t p66_vector_add(p66_preparation *state,p66_scale_scratch *scale,
                        p66_elapsed_state *elapsed,const agc_word operand[6]);
uint16_t p66_vector_subtract(p66_preparation *state,p66_scale_scratch *scale,
                             p66_elapsed_state *elapsed,const agc_word operand[6]);
#endif
