/* SPDX-License-Identifier: MIT */
#ifndef C64_P66_RECURRENCE_FIXTURE_H
#define C64_P66_RECURRENCE_FIXTURE_H
#include "p66_recurrence_live_cases.h"
typedef struct {uint8_t number,stage,failures;p66_bias_io *io;agc_word *channel14,*channel11;uint16_t *in_interrupt;} p66_recurrence_fixture;
typedef char recurrence_fixture_lease[(sizeof(p66_recurrence_fixture)<=0x40u)?1:-1];
#define P66_RECURRENCE_FIXTURE ((p66_recurrence_fixture *)(uintptr_t)0xc000)
#endif
