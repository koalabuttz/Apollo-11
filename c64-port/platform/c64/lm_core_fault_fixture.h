/* SPDX-License-Identifier: MIT */
#ifndef LM_CORE_FAULT_FIXTURE_H
#define LM_CORE_FAULT_FIXTURE_H
#include "lm_core_fault.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_core_fault_fixture;
typedef char fixture_fits[(sizeof(lm_core_fault_fixture)<=0x40u)?1:-1];
#define LM_CORE_FAULT_FIXTURE ((lm_core_fault_fixture *)(uintptr_t)0xc000)
#endif
