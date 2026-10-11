/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_NUMBER_CASES_H
#define DSKY_CHARIN_NUMBER_CASES_H
#include "dsky_charin_number.h"
#define DSKY_CHARIN_NUMBER_CASE_COUNT 8192
#define DSKY_CHARIN_NUMBER_TARGET_COUNT 64
uint16_t dsky_charin_number_channel11(uint16_t);
void dsky_charin_number_prepare(apollo_overlay_state *,uint16_t);
#endif
