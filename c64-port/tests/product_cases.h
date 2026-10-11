/* SPDX-License-Identifier: MIT */
#ifndef PRODUCT_CASES_H
#define PRODUCT_CASES_H
#include "guidance_product.h"
#include "vector_cases.h"
#define PR_CASE_COUNT 4096u
#define PR_TARGET_COUNT 64u
#define PR_TARGET_ROWS 128u
#define PR_WORD_COUNT VE_WORD_COUNT
typedef vector_case product_case;
void pr_prepare(product_case *c,uint16_t n,uint8_t vector);
uint16_t pr_target_number(uint16_t index);
#ifndef PRODUCT_INPUT_ONLY
uint8_t pr_apply(product_case *c,uint8_t vector);
unsigned pr_check(product_case *c);
#endif
#endif
