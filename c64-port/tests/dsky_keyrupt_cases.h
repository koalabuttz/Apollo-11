/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEYRUPT_CASES_H
#define DSKY_KEYRUPT_CASES_H
#include "dsky_keyrupt.h"
#define DSKY_KEYRUPT_CASE_COUNT 4096
#define DSKY_KEYRUPT_TARGET_COUNT 64
uint16_t dsky_keyrupt_channel15(uint16_t);
void dsky_keyrupt_prepare(apollo_overlay_state *,uint16_t);
#endif
