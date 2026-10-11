/* SPDX-License-Identifier: MIT */
#ifndef LM_SPECTEST_HANDOFF_FIXTURE_H
#define LM_SPECTEST_HANDOFF_FIXTURE_H
#include "lm_spectest_handoff.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_spectest_handoff_fixture;
typedef char fixture_fits[(sizeof(lm_spectest_handoff_fixture)<=0x40u)?1:-1];
#define LM_SPECTEST_HANDOFF_FIXTURE ((lm_spectest_handoff_fixture *)(uintptr_t)0xc000)
#endif
