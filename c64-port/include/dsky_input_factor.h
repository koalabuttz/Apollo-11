/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_FACTOR_H
#define DSKY_INPUT_FACTOR_H
#include "dsky_input_select.h"
enum dsky_input_factor_route {DSKY_INPUT_FACTOR_SELECTED=0,DSKY_INPUT_FACTOR_LAST=13};
/* PUTDECSF/PUTSFNOR/PUTDCSF2 through actual GTSFIN and BANKJUMP.
 * Route is selected literal original input-routine index0..13, while image
 * carries its actual source PC/banks/registers. No scaling body fabricated. */
enum p66_bias_status dsky_input_factor(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_factor_route *);
enum exec_status cm_input_factor(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_input_factor_route *);
#endif
