/* SPDX-License-Identifier: MIT */
#ifndef VECTOR_CASES_H
#define VECTOR_CASES_H
#include "guidance_vector.h"
#include "prepare_cases.h"
#define VE_WORD_COUNT 50u
#define VE_CASE_COUNT 6800u
#define VE_TARGET_COUNT 64u
#define VE_TARGET_ROWS (VE_TARGET_COUNT*2u)
#define VE_ROWS (VE_CASE_COUNT*2u)
typedef struct {
    p66_preparation preparation;
    p66_scale_scratch scale;
    p66_elapsed_state elapsed;
    agc_word operand[6];
} vector_case;
uint16_t ve_target_number(uint16_t index);
void ve_prepare(vector_case *c,uint16_t n,uint8_t subtract);
void ve_words(const vector_case *c,uint16_t w[VE_WORD_COUNT]);
void ve_from_words(vector_case *c,const uint16_t w[VE_WORD_COUNT]);
#ifndef VECTOR_INPUT_ONLY
uint16_t ve_apply(vector_case *c,uint8_t subtract);
unsigned ve_check(vector_case *c);
#endif
#endif
