/* SPDX-License-Identifier: MIT */
#ifndef P66_DESCENT_CONTROL_H
#define P66_DESCENT_CONTROL_H
#include "p66_descent_runtime.h"
/* Explicit external I/O stays outside erasable RAM. Module entry returns
 * before any overlay swap.0xffff reports unsupported/transport failure. */
uint16_t p66_descent_control_entry(apollo_overlay_state *,p66_bias_io *,agc_word *channel14,uint16_t selector);
#endif
