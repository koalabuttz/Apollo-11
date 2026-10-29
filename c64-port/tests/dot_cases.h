/* SPDX-License-Identifier: MIT */
#ifndef DOT_CASES_H
#define DOT_CASES_H
#include "elapsed_cases.h"
#include "guidance_dot.h"
#define DT_WORD_COUNT 100u
#define DT_CASE_COUNT 10272u
#define DT_TARGET_ROWS 96u
typedef struct {elapsed_case previous;p66_divide_scratch divide;agc_word buf_third,operand[6];} dot_case;
void dt_prepare(dot_case *,uint16_t);
void dt_words(const dot_case *,uint16_t words[DT_WORD_COUNT]);
uint16_t dt_target_number(uint16_t);
#ifndef DOT_INPUT_ONLY
uint8_t dt_apply(dot_case *);
unsigned dt_check(dot_case *);
#endif
#endif
