/* SPDX-License-Identifier: MIT */
#ifndef DSKY_SCALE_CASES_H
#define DSKY_SCALE_CASES_H
#include "dsky_scale.h"
#define DSKY_SCALE_CASE_COUNT 4096
#define DSKY_SCALE_TARGET_COUNT 64
void dsky_scale_prepare(apollo_overlay_state *,uint16_t);
enum dsky_scale_entry dsky_scale_entry_for(uint16_t);
#endif
