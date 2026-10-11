/* SPDX-License-Identifier: MIT */
#ifndef AGC_MULTIWORD_H
#define AGC_MULTIWORD_H
#include "agc_word.h"
typedef struct { agc_word high, low; } agc_dp;
typedef struct { agc_register a, l; } agc_register_pair;
/* MP with a normal 15-bit memory operand, including accumulator correction.
 * Each output register is sign extended. No register-address aliases. */
agc_register_pair agc_multiply(agc_register accumulator, agc_word multiplier);
/* DAS to an ordinary erasable pair. L must be a sign-extended SP word.
 * Independent word signs are supported. Output A is overflow indicator
 * (-1/+1/+0); output L is +0. Neither pointer may alias the other. */
void agc_double_add(agc_register_pair *registers, agc_dp *destination);
#endif
