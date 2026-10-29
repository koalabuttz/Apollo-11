/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_SCALE_H
#define DSKY_INPUT_SCALE_H
#include "dsky_input_arithmetic.h"
enum dsky_input_scale_route {DSKY_INPUT_SCALE_COMPONENT,DSKY_INPUT_SCALE_RECYCLE,DSKY_INPUT_SCALE_FALLTHROUGH};
/* Actual arithmetic/DP input-scale bodies through real PUTCOM2 or source
 * ALMCYCLE/TESTOFUF -zero frontier. Canonical source limbs/constants;
 * selected ordinary bank-local DP destination, no runtime opcode execution. */
enum p66_bias_status dsky_input_scale(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_scale_route *);
enum exec_status cm_input_scale(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_input_scale_route *);
#endif
