/* SPDX-License-Identifier: MIT */
#ifndef ROTATION_CASES_H
#define ROTATION_CASES_H
#include "unit_cases.h"
#include "guidance_rotation.h"
#define ROT_WORDS 131u
#define ROT_CASES 32768u
typedef struct {unit_case previous;p66_trig_scratch trig;p66_rotation_scratch rotation;agc_word direction,return_word;} rotation_case;
void rot_input(rotation_case *,uint16_t);
void rot_words(const rotation_case *,uint16_t [ROT_WORDS]);
#ifndef ROTATION_INPUT_ONLY
unsigned rot_apply(rotation_case *);
#endif
#endif
