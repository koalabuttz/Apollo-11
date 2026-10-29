/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_UPDATE_CHECK_H
#define DSKY_NV_UPDATE_CHECK_H
#include "dsky_nv_update_cases.h"
uint8_t dsky_nv_update_checkpoint_input(uint8_t,apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission *,enum dsky_nv_update_entry *);
uint16_t dsky_nv_update_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
