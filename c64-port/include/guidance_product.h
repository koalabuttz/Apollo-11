/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_PRODUCT_H
#define GUIDANCE_PRODUCT_H
#include "guidance_scale.h"
/* Arbitrary ordinary DP factor, with distinct exclusively owned objects and
 * producers excluded. Vector VXSC requires MODE=-1, rounds each component
 * via historical VROUND, and stops at DANZIG. Scalar DMP1 requires MODE=+0,
 * leaves the unrounded TP product in MPAC,+1,+2 and retains MODE. Both preserve
 * all unrelated state/factor, and expose exact MPTEMP/OVFIND/A/L effects.
 * Return 1 on supported MODE, 0 with every object unchanged otherwise.
 * This is direct native arithmetic; no opcode decoding or dispatch. */
uint8_t p66_vector_multiply(p66_preparation *state,p66_scale_scratch *scratch,
                             const agc_dp *factor);
uint8_t p66_scalar_product(p66_preparation *state,p66_scale_scratch *scratch,
                            const agc_dp *factor);
#endif
