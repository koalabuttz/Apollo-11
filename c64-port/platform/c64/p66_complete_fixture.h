/* SPDX-License-Identifier: MIT */
#ifndef C64_P66_COMPLETE_FIXTURE_H
#define C64_P66_COMPLETE_FIXTURE_H
#include "p66_complete_live_cases.h"
typedef struct {uint8_t number,stage,failures;p66_bias_io *io;agc_word *channel14,*channel11;} p66_complete_fixture;
typedef char complete_fixture_lease[(sizeof(p66_complete_fixture)<=0x40u)?1:-1];
#define P66_COMPLETE_FIXTURE ((p66_complete_fixture *)(uintptr_t)0xc000)
#endif
