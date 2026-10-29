/* SPDX-License-Identifier: MIT */
#ifndef CM_RELAY_CASES_H
#define CM_RELAY_CASES_H
#include "cm_relay.h"
#define CM_RELAY_CASES 4096u
#define CM_RELAY_TARGET_COUNT 64u
void cm_relay_input(apollo_overlay_state *,cm_native_io *,cm_relay_output *,uint16_t);
#endif
