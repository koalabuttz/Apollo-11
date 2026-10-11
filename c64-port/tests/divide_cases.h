/* SPDX-License-Identifier: MIT */
#ifndef DIVIDE_CASES_H
#define DIVIDE_CASES_H
#include "guidance_divide.h"
#include "store_cases.h"
#define DV_WORD_COUNT 93u
#define DV_TARGET_COUNT 147u
#define DV_TARGET_ROWS 156u
#define DV_CASE_COUNT 6735u
#define DV_ROWS 8098u
typedef struct { store_case previous; p66_divide_scratch scratch; } divide_case;
void dv_prepare(divide_case *c,uint16_t n);
void dv_next(divide_case *c,uint16_t n,uint8_t step);
uint8_t dv_steps(uint16_t n);
uint8_t dv_integrated(uint16_t n);
void dv_words(const divide_case *c,uint16_t w[DV_WORD_COUNT]);
void dv_from_words(divide_case *c,const uint16_t w[DV_WORD_COUNT]);
#ifndef DIVIDE_INPUT_ONLY
void dv_prefix(divide_case *c,uint16_t n);
enum p66_divide_result dv_apply(divide_case *c);
unsigned dv_check(divide_case *c);
#endif
#endif
