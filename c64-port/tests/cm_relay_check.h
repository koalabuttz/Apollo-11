/* SPDX-License-Identifier: MIT */
#ifndef CM_RELAY_CHECK_H
#define CM_RELAY_CHECK_H
#include "cm_relay_cases.h"
uint16_t cm_relay_check(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
uint16_t cm_relay_display_check(apollo_overlay_state *,volatile uint8_t *,volatile uint8_t *);
#endif
