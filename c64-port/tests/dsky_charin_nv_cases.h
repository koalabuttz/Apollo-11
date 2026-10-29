/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_NV_CASES_H
#define DSKY_CHARIN_NV_CASES_H
#include "dsky_charin_nv.h"
#define DSKY_CHARIN_NV_CASE_COUNT 4096
#define DSKY_CHARIN_NV_TARGET_COUNT 64
uint16_t dsky_charin_nv_channel11(uint16_t);
void dsky_charin_nv_prepare(apollo_overlay_state *,uint16_t);
#endif
