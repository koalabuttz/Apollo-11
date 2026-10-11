/* SPDX-License-Identifier: MIT */
#ifndef CM_MINUTES_SECONDS_CHECK_H
#define CM_MINUTES_SECONDS_CHECK_H
#include "cm_minutes_seconds_cases.h"
#include "cm_decimal_pair_cases.h"
uint8_t cm_ms_checkpoint_input(uint8_t,apollo_overlay_state *,cm_native_io *);
uint16_t cm_ms_checkpoint_check(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
