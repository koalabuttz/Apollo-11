/* SPDX-License-Identifier: MIT */
#ifndef SCALE_CASES_H
#define SCALE_CASES_H
#include "guidance_scale.h"
#include "prepare_cases.h"
#define SC_WORD_COUNT 41u
#define SC_TARGET_COUNT 283u
#define SC_CASE_COUNT 3355u
#define SC_TARGET_ROWS 292u
#define SC_ROWS 3364u
typedef struct { p66_preparation preparation; p66_scale_scratch scratch; } scale_case;
void sc_prepare(scale_case *c, uint16_t n);
uint8_t sc_steps(uint16_t n);
uint8_t sc_integrated(uint16_t n);
void sc_next(scale_case *c,uint16_t n,uint8_t step);
void sc_words(const scale_case *c,uint16_t w[SC_WORD_COUNT]);
void sc_from_words(scale_case *c,const uint16_t w[SC_WORD_COUNT]);
#ifndef SCALE_INPUT_ONLY
void sc_apply(scale_case *c,uint16_t n);
unsigned sc_check(scale_case *c);
#endif
#endif
