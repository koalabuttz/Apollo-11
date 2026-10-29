/* SPDX-License-Identifier: MIT */
#ifndef LM_SCAN_EXCEPTION_FIXTURE_H
#define LM_SCAN_EXCEPTION_FIXTURE_H
#include "lm_scan_exception.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_scan_exception_fixture;
typedef char fixture_fits[(sizeof(lm_scan_exception_fixture)<=0x40u)?1:-1];
#define LM_SCAN_EXCEPTION_FIXTURE ((lm_scan_exception_fixture *)(uintptr_t)0xc000)
#endif
