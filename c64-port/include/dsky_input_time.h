/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_TIME_H
#define DSKY_INPUT_TIME_H
#include "dsky_input_mode.h"
enum dsky_input_time_route {DSKY_INPUT_TIME_LOADED,DSKY_INPUT_TIME_RECYCLE,DSKY_INPUT_TIME_CALLER};
/* Actual HMSIN, RND/TST, MPACTST and SIZETST; source exceptional handoffs retained. */
enum p66_bias_status dsky_input_time(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_time_route *);
enum exec_status cm_input_time(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_input_time_route *);
#endif
