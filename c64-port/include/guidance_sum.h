/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_SUM_H
#define GUIDANCE_SUM_H
#include "guidance_elapsed.h"
#define P66_SUM_OUT_OF_SCOPE UINT16_C(0100000)
/* General stable ordinary DP/six-word operands, distinct from owned state;
 * producer exclusion is a caller obligation. Scalar functions require DP
 * MODE=+0; vector reverse subtraction requires MODE=-1. Preserve MODE,
 * MPAC+2, MPTEMP, PIPTIME, operands and unrelated preparation words. BUF,
 * OVFIND and A/L retain original DAS/OVERFLOW effects without saturation.
 * Return bits0..2 observe initial overflow / lowTSskip / highTSskip. Vector
 * observations occupy3bits for each X,Y,Z (processed Y,Z,X). These observations
 * are not historical memory. Invalid MODE returnsOUT_OF_SCOPE unchanged.
 * Stop at DANZIG; bank/address/interpreter dispatch are outside this boundary. */
uint16_t p66_scalar_add(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,const agc_dp *);
uint16_t p66_scalar_subtract(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,const agc_dp *);
uint16_t p66_scalar_reverse_subtract(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,const agc_dp *);
uint16_t p66_vector_reverse_subtract(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,const agc_word operand[6]);
#endif
