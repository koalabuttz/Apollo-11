/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_BUSY_CHECK_H
#define DSKY_NV_BUSY_CHECK_H
#include "dsky_nv_busy_cases.h"
uint8_t dsky_nv_busy_checkpoint_input(uint8_t,apollo_overlay_state *,p66_bias_io *,agc_word *,enum dsky_nv_mission *,enum dsky_nv_busy_entry *);
uint16_t dsky_nv_busy_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
