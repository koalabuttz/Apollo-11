/* SPDX-License-Identifier: MIT */
#ifndef TRIG_CASES_H
#define TRIG_CASES_H
#include "unit_cases.h"
#include "guidance_trig.h"
#define TG_WORDS 130u
#define TG_CASES 65536u
typedef struct {unit_case previous;p66_trig_scratch trig;agc_word cdu[3];} trig_case;
uint8_t tg_operation(uint32_t);
void tg_input(trig_case *,uint32_t);
void tg_words(const trig_case *,uint16_t [TG_WORDS]);
#ifndef TRIG_INPUT_ONLY
unsigned tg_apply(trig_case *,uint8_t);
#endif
#endif
