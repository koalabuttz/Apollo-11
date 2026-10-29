/* SPDX-License-Identifier: MIT */
#ifndef RUNTIME_BRIDGE_CASES_H
#define RUNTIME_BRIDGE_CASES_H
#include "p66_runtime_bridge.h"
#define RUNTIME_BRIDGE_CASES 16384u
void runtime_bridge_input(apollo_overlay_state *,p66_bias_io *,uint16_t);
uint16_t runtime_bridge_apply(apollo_overlay_state *,p66_bias_io *,uint16_t);
#endif
