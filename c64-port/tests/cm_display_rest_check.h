/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPLAY_REST_CHECK_H
#define CM_DISPLAY_REST_CHECK_H
#include "cm_display_rest_cases.h"
uint8_t cm_display_rest_checkpoint_input(uint8_t,apollo_overlay_state *,cm_native_io *);
uint16_t cm_display_rest_checkpoint(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
