/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_DIVIDE_H
#define GUIDANCE_DIVIDE_H
#include "guidance_elapsed.h"
typedef struct { agc_word buf_low,dvsign,dvnormct,maxdvsw; } p66_divide_scratch;
enum p66_divide_result { P66_DIVIDE_ZERO=0, P66_DIVIDE_QUOTIENT=1,
    P66_DIVIDE_OVERFLOW=2, P66_DIVIDE_OUT_OF_SCOPE=3 };
/* DDV 4SEC(28) only: fixed raw divisor {0,400}, DP MODE=+0, ordinary
 * 15-bit words and distinct owned objects; caller excludes producers.
 * BUF high aliases elapsed.buf. No rounding beyond the original routine.
 * Stop at DANZIG before pending PDVL. Unsupported MODE rejects unchanged.
 * Result enum observes the source path; overflow also sets original OVFIND. */
enum p66_divide_result p66_divide_four_seconds(p66_preparation *state,
    p66_scale_scratch *scale,p66_elapsed_state *elapsed,p66_divide_scratch *scratch);
#endif
