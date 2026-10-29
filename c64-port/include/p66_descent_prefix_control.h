/* SPDX-License-Identifier: MIT */
#ifndef P66_DESCENT_PREFIX_CONTROL_H
#define P66_DESCENT_PREFIX_CONTROL_H
#include "overlay_runtime.h"
#include "agc_word.h"
/* Source register/edit-word effects at explicitly compiled P66 completed pairs.
 * The DDV Q argument is observed from its preoperation numerator/divisor.
 * Unknown completions and pending EDOP preserve the image. No opcode decoder. */
uint8_t p66_descent_prefix_control(apollo_overlay_state *,agc_word completed,agc_word ddv_q);
#endif
