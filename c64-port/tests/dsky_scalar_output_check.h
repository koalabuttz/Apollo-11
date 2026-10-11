/* SPDX-License-Identifier: MIT */
#ifndef DSKY_SCALAR_OUTPUT_CHECK_H
#define DSKY_SCALAR_OUTPUT_CHECK_H
#include "dsky_scalar_output_cases.h"
uint8_t dsky_scalar_output_checkpoint_input(uint8_t,apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission *,enum dsky_scalar_output_entry *,agc_word *);
uint16_t dsky_scalar_output_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
