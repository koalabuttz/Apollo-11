/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_ELAPSED_H
#define GUIDANCE_ELAPSED_H
#include "guidance_scale.h"
/* Stable PIPTIME input and OVERFLOW's BUF scratch. OVFIND remains owned by
 * scaling scratch; MPAC/MODE/A/L remain owned by preparation state. */
typedef struct { agc_dp piptime; agc_word buf; } p66_elapsed_state;
/* DP DSU PIPTIME. Caller owns distinct objects, excludes producers, and
 * supplies DP MODE=+0 and ordinary 15-bit words. Stop at DANZIG before STORE
 * 30D. Preserve source overflow correction, with no saturation/normalization
 * beyond that path. Return observation bits: 0=initial DAS high overflow,
 * 1=low TS 7 skip, 2=high TS 7 skip. This mask is not historical memory. */
uint8_t p66_elapsed_time(p66_preparation *state,p66_scale_scratch *scale,
                         p66_elapsed_state *elapsed);
#endif
