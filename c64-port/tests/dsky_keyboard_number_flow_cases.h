/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEYBOARD_NUMBER_FLOW_CASES_H
#define DSKY_KEYBOARD_NUMBER_FLOW_CASES_H
#include "dsky_keycom.h"
#define DSKY_KEYBOARD_NUMBER_FLOW_CASE_COUNT 1024
void dsky_keyboard_number_flow_prepare(apollo_overlay_state *,uint16_t);
uint16_t dsky_keyboard_number_flow_channel11(uint16_t);
uint16_t dsky_keyboard_number_flow_key(uint16_t);
#endif
