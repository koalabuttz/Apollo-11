/* SPDX-License-Identifier: MIT */
#ifndef GRAVITY_CASES_H
#define GRAVITY_CASES_H
#include "divide_cases.h"
#include "guidance_gravity.h"
#define GV_WORD_COUNT 99u
#define GV_CASE_COUNT 3865u
#define GV_ROWS 5218u
#define GV_TARGET_COUNT 147u
#define GV_TARGET_ROWS 156u
typedef struct { divide_case previous; agc_word gravity[6]; } gravity_case;
void gv_prepare(gravity_case *c,uint16_t n);
void gv_next(gravity_case *c,uint16_t n,uint8_t step);
uint8_t gv_integrated(uint16_t n);
uint8_t gv_steps(uint16_t n);
void gv_words(const gravity_case *c,uint16_t w[GV_WORD_COUNT]);
void gv_from_words(gravity_case *c,const uint16_t w[GV_WORD_COUNT]);
#ifndef GRAVITY_INPUT_ONLY
void gv_prefix(gravity_case *c,uint16_t n);
enum p66_push_status gv_apply(gravity_case *c);
unsigned gv_check(gravity_case *c);
#endif
#endif
