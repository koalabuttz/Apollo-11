/* SPDX-License-Identifier: MIT */
#ifndef SUM_CASES_H
#define SUM_CASES_H
#include "vector_cases.h"
#include "guidance_sum.h"
#define SU_WORD_COUNT VE_WORD_COUNT
#define SU_CASE_COUNT VE_CASE_COUNT
#define SU_ROWS (SU_CASE_COUNT*4u)
#define SU_TARGET_ROWS (VE_TARGET_COUNT*4u)
void su_prepare(vector_case *,uint16_t n,uint8_t operation);
#ifndef SUM_INPUT_ONLY
uint16_t su_apply(vector_case *,uint8_t operation);
unsigned su_check(vector_case *);
#endif
#endif
