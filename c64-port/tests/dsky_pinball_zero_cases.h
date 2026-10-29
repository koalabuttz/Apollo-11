/* SPDX-License-Identifier: MIT */
#ifndef DSKY_PINBALL_ZERO_CASES_H
#define DSKY_PINBALL_ZERO_CASES_H
#include "dsky_pinball.h"
#define DSKY_PINBALL_ZERO_CASE_COUNT 4096
#define DSKY_PINBALL_ZERO_TARGET_COUNT 64
void dsky_pinball_zero_prepare(apollo_overlay_state *,uint16_t);
enum dsky_pinball_entry dsky_pinball_zero_entry_for(uint16_t);
#endif
