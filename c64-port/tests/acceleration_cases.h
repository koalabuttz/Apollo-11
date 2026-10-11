/* SPDX-License-Identifier: MIT */
#ifndef ACCELERATION_CASES_H
#define ACCELERATION_CASES_H
#include "unit_cases.h"
#include "guidance_acceleration.h"
#define ACC_WORDS 112u
#define ACC_CASES 16384u
typedef struct {unit_case previous;agc_dp taurod;agc_word gravity[6];} acceleration_case;
void acc_input(acceleration_case *,uint16_t);
void acc_words(const acceleration_case *,uint16_t [ACC_WORDS]);
#ifndef ACCELERATION_INPUT_ONLY
unsigned acc_apply(acceleration_case *);
#endif
#endif
