/* SPDX-License-Identifier: MIT */
#ifndef UNIT_CASES_H
#define UNIT_CASES_H
#include "divide_cases.h"
#include "guidance_unit.h"
#define UNIT_CASES 50736u
#define UNIT_WORDS 104u
typedef struct{divide_case base;p66_norm_scratch norm;}unit_case;
void unit_input(unit_case*,uint32_t);
void unit_words(const unit_case*,uint16_t[UNIT_WORDS]);
#ifndef UNIT_INPUT_ONLY
unsigned unit_apply(unit_case*,uint8_t);
#endif
#endif
