/* SPDX-License-Identifier: MIT */
#ifndef LM_ADMISSION_RAW_FIXTURE_H
#define LM_ADMISSION_RAW_FIXTURE_H
#include "lm_admission_raw.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_admission_raw_fixture;
typedef char fixture_fits[(sizeof(lm_admission_raw_fixture)<=0x40u)?1:-1];
#define LM_ADMISSION_RAW_FIXTURE ((lm_admission_raw_fixture *)(uintptr_t)0xc000)
#endif
