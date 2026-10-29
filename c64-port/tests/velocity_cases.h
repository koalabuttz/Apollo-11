/* SPDX-License-Identifier: MIT */
#ifndef VELOCITY_CASES_H
#define VELOCITY_CASES_H
#include "gravity_cases.h"
#include "guidance_velocity.h"
#define VL_WORD_COUNT 117u
#define VL_CASE_COUNT 3475u
#define VL_ROWS 4648u
#define VL_TARGET_COUNT 147u
#define VL_TARGET_ROWS 156u
typedef struct { gravity_case previous; agc_word bias[6],velocity[6],position[6]; } velocity_case;
void vl_prepare(velocity_case *,uint16_t n);
void vl_next(velocity_case *,uint16_t n,uint8_t step);
uint8_t vl_integrated(uint16_t n);
uint8_t vl_steps(uint16_t n);
void vl_words(const velocity_case *,uint16_t words[VL_WORD_COUNT]);
void vl_from_words(velocity_case *,const uint16_t words[VL_WORD_COUNT]);
#ifndef VELOCITY_INPUT_ONLY
void vl_prefix(velocity_case *,uint16_t n);
enum p66_velocity_status vl_apply(velocity_case *);
unsigned vl_check(velocity_case *);
#endif
#endif
