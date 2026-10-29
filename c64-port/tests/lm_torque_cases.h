/* SPDX-License-Identifier: MIT */
#ifndef LM_TORQUE_CASES_H
#define LM_TORQUE_CASES_H
#include "lm_torque.h"
#define LM_TORQUE_CASE_COUNT 36864u
#define LM_TORQUE_ROW_WORDS 2052u
void lm_torque_prepare(apollo_overlay_state *,lm_torque_io *,uint16_t);
#endif
