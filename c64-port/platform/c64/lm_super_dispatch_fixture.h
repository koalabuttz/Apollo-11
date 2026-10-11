/* SPDX-License-Identifier: MIT */
#ifndef LM_SUPER_DISPATCH_FIXTURE_H
#define LM_SUPER_DISPATCH_FIXTURE_H
#include "lm_super_dispatch.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_super_dispatch_fixture;
typedef char fixture_fits[(sizeof(lm_super_dispatch_fixture)<=0x40u)?1:-1];
#define LM_SUPER_DISPATCH_FIXTURE ((lm_super_dispatch_fixture *)(uintptr_t)0xc000)
#endif
