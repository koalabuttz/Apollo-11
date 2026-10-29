/* SPDX-License-Identifier: MIT */
#ifndef HORIZONTAL_CASES_H
#define HORIZONTAL_CASES_H
#include "unit_cases.h"
#include "guidance_horizontal.h"
#define HH_WORDS 120u
#define HH_CASES 8192u
typedef struct {unit_case previous;agc_word delvs[6],position[6];agc_dp hdotdisp,vhoriz;} horizontal_case;
void hh_input(horizontal_case *,uint16_t);
void hh_words(const horizontal_case *,uint16_t [HH_WORDS]);
void hh_from_words(horizontal_case *,const uint16_t [HH_WORDS]);
#ifndef HORIZONTAL_INPUT_ONLY
unsigned hh_apply(horizontal_case *,uint8_t);
#endif
#endif
