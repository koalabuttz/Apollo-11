/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_H
#define GUIDANCE_H
#include "agc_multiword.h"
/* Raw historical words: no SI conversion, saturation or pair normalization. */
typedef struct {
    agc_word count, scale;
    agc_dp desired;
    agc_register_pair registers;
} p66_rate_state;
/* RODCOMP 32,3275..3302, before 32,3303. Caller owns the state and must
 * exclude its count producer for the whole call. Native scratch replaces A/L;
 * hardware bank switching and the subsequent interpreter are outside this API. */
void p66_rate_update(p66_rate_state *state);
#endif
