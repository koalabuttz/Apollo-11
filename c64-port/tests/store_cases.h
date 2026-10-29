/* SPDX-License-Identifier: MIT */
#ifndef STORE_CASES_H
#define STORE_CASES_H
#include "guidance_store.h"
#include "elapsed_cases.h"
#define ST_WORD_COUNT EL_WORD_COUNT
#define ST_TARGET_COUNT 163u
#define ST_TARGET_EXTRA 4u
#define ST_TARGET_ROWS 176u
#define ST_CASE_COUNT 3681u
#define ST_ROWS 5044u
typedef elapsed_case store_case;
uint16_t st_target_extra(uint8_t index);
void st_prepare(store_case *c,uint16_t n);
uint8_t st_steps(uint16_t n);
uint8_t st_integrated(uint16_t n);
void st_next(store_case *c,uint16_t n,uint8_t step);
#ifndef STORE_INPUT_ONLY
uint8_t st_prefix(store_case *c,uint16_t n);
enum p66_store_status st_apply(store_case *c);
unsigned st_check(store_case *c);
#endif
#endif
