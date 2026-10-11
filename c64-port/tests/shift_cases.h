/* SPDX-License-Identifier: MIT */
#ifndef SHIFT_CASES_H
#define SHIFT_CASES_H
#include "elapsed_cases.h"
#include "guidance_shift.h"
#define SH_WORD_COUNT EL_WORD_COUNT
#define SH_GROUP_SIZE 320u
#define SH_CASE_COUNT (84u*SH_GROUP_SIZE)
#define SH_TARGET_ROWS 168u
typedef struct { elapsed_case previous; uint8_t operation,count,short_form; } shift_case;
void sh_prepare(shift_case *,uint16_t);
void sh_words(const shift_case *,uint16_t w[SH_WORD_COUNT]);
uint16_t sh_target_number(uint16_t row);
#ifndef SHIFT_INPUT_ONLY
uint8_t sh_apply(shift_case *);
unsigned sh_check(shift_case *);
#endif
#endif
