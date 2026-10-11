/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_SELECT_H
#define DSKY_INPUT_SELECT_H
#include "dsky_lamp_release.h"
enum dsky_input_select_route {DSKY_INPUT_SELECT_CALLER};
/* SFRUTNOR/MIX, SFCONUM/CONUMNOR/SFRET/1 plus GOQ and cyclic5 primitives.
 * Real retained canonical CPU, packed type/routine words and selected caller;
 * source MIXBR1/2 and DECOUNT0..2 only where selected branches read them. */
enum p66_bias_status dsky_input_select(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_select_route *);
enum exec_status cm_input_select(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_input_select_route *);
#endif
