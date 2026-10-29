/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCK_NORMAL_DISPLAY_CHECK_H
#define CM_CLOCK_NORMAL_DISPLAY_CHECK_H
#include "cm_clock_normal_display.h"
uint8_t cm_clock_normal_checkpoint_input(uint8_t,apollo_overlay_state *,cm_native_io *,cm_mission_loop *);
uint16_t cm_clock_normal_checkpoint_check(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
