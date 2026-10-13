/* SPDX-License-Identifier: MIT */
#ifndef LM_SVCT3_CASES_H
#define LM_SVCT3_CASES_H
#include "lm_svct3.h"
/* 512 states per SVCT3 phase (0-511 SVCT3, 512-1023 after NOVAC, 1024-1535
 * SVCT3X's TC SVCT3): every CCS class of FLAGWRD2 and IMUCADR. */
#define LM_SVCT3_CASES 1536u
void lm_svct3_input(apollo_overlay_state *,p66_bias_io *,uint16_t);
#endif
