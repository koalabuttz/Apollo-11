/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_NORM_H
#define GUIDANCE_NORM_H
#include "guidance_dp_divide.h"
typedef struct {agc_word sr,cyl,addrwd,buf_third,tem1,vbuf[6];} p66_norm_scratch;
enum p66_norm_result {P66_NORM_OK=0,P66_NORM_ALARM_1302=1,P66_NORM_OUT_OF_SCOPE=2};
/* Source SQRT, TP input in MPAC0..2 and DP MODE=+0. Retains both signed
 * zeros and source Newton approximation. Negative aborts stop before
 * SQRTABRT, observing 1302 without implementing the executive alarm path.
 * Distinct owned ordinary-word objects and excluded producers required. */
enum p66_norm_result p66_square_root(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *);
/* SQRTSUB boundary used by UNIT: retain normalized root and shift count,
 * accepts DP +0 or vector -1 MODE without changing it. */
enum p66_norm_result p66_normalized_square_root(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *);
#endif
