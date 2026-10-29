/* SPDX-License-Identifier: MIT */
#ifndef ELAPSED_CASES_H
#define ELAPSED_CASES_H
#include "guidance_elapsed.h"
#include "push_cases.h"
#define EL_WORD_COUNT 89u
#define EL_TARGET_COUNT 163u
#define EL_TARGET_ROWS 172u
#define EL_CASE_COUNT 3491u
#define EL_ROWS 4664u
typedef struct { push_case previous; p66_elapsed_state elapsed; } elapsed_case;
void el_prepare(elapsed_case *c,uint16_t n);
uint8_t el_steps(uint16_t n);
uint8_t el_integrated(uint16_t n);
void el_next(elapsed_case *c,uint16_t n,uint8_t step);
void el_words(const elapsed_case *c,uint16_t w[EL_WORD_COUNT]);
void el_from_words(elapsed_case *c,const uint16_t w[EL_WORD_COUNT]);
#ifndef ELAPSED_INPUT_ONLY
void el_prefix(elapsed_case *c,uint16_t n);
uint8_t el_apply(elapsed_case *c);
unsigned el_check(elapsed_case *c);
#endif
#endif
