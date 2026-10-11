/* SPDX-License-Identifier: MIT */
#ifndef LM_RECURRENCE_BRIDGE_FIXTURE_H
#define LM_RECURRENCE_BRIDGE_FIXTURE_H
#include "lm_idle_start.h"
typedef struct {p66_bias_io io;agc_word channel11,channel14;uint16_t in_interrupt,stage,failures;} lm_recurrence_bridge_fixture;
#define LM_RECURRENCE_FIXTURE ((lm_recurrence_bridge_fixture *)(uintptr_t)0xc000)
typedef char recurrence_fixture_bounds[(sizeof(lm_recurrence_bridge_fixture)<=0x40u)?1:-1];
#endif
