/* SPDX-License-Identifier: MIT */
#ifndef LM_BASIC_CHANGE_FIXTURE_H
#define LM_BASIC_CHANGE_FIXTURE_H
#include "lm_basic_change.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_transfer next;} lm_basic_change_fixture;
typedef char fixture_fits[(sizeof(lm_basic_change_fixture)<=0x40u)?1:-1];
#define LM_BASIC_CHANGE_FIXTURE ((lm_basic_change_fixture *)(uintptr_t)0xc000)
#endif
