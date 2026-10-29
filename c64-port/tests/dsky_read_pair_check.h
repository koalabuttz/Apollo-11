/* SPDX-License-Identifier: MIT */
#ifndef DSKY_READ_PAIR_CHECK_H
#define DSKY_READ_PAIR_CHECK_H
#include "dsky_read_pair_cases.h"
uint8_t dsky_read_pair_checkpoint_input(uint8_t,apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission *);
uint16_t dsky_read_pair_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
