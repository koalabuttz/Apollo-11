/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_ANGLE_H
#define DSKY_INPUT_ANGLE_H
#include "dsky_input_scale.h"
enum dsky_input_angle_route {DSKY_INPUT_ANGLE_COMPONENT,DSKY_INPUT_ANGLE_RECYCLE,DSKY_INPUT_ANGLE_FALLTHROUGH,DSKY_INPUT_ANGLE_CCSHOLE,DSKY_INPUT_ANGLE_CALLER};
/* Actual DEGINSF/2/SIGNFIX/NEG180/SGNTO1/ENDSCALE, LM360-CDU and
 * CMOPTDEGIN/OPDEGIN2. Preserves genuine exceptional source frontiers. */
enum p66_bias_status dsky_input_angle(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_angle_route *);
enum exec_status cm_input_angle(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_input_angle_route *);
#endif
