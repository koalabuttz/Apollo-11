/* SPDX-License-Identifier: MIT */
#ifndef PUSH_CASES_H
#define PUSH_CASES_H
#include "guidance_push.h"
#include "scale_cases.h"
#define PS_WORD_COUNT 86u
#define PS_TARGET_COUNT 191u
#define PS_CASE_COUNT 2323u
#define PS_TARGET_ROWS 200u
#define PS_ROWS 3496u
typedef struct { scale_case previous; p66_pushdown push; } push_case;
void ps_prepare(push_case *c,uint16_t n);
uint8_t ps_steps(uint16_t n);
uint8_t ps_integrated(uint16_t n);
void ps_next(push_case *c,uint16_t n,uint8_t step);
void ps_words(const push_case *c,uint16_t w[PS_WORD_COUNT]);
void ps_from_words(push_case *c,const uint16_t w[PS_WORD_COUNT]);
#ifndef PUSH_INPUT_ONLY
void ps_prefix(push_case *c,uint16_t n);
enum p66_push_status ps_apply(push_case *c);
unsigned ps_check(push_case *c);
#endif
#endif
