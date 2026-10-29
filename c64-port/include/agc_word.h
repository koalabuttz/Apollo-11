/* SPDX-License-Identifier: MIT */
#ifndef AGC_WORD_H
#define AGC_WORD_H
#include <stdint.h>
typedef uint16_t agc_word;
typedef uint16_t agc_register;
#define AGC_MASK UINT16_C(077777)
#define AGC_SIGN UINT16_C(040000)
#define AGC_POS_ZERO UINT16_C(0)
#define AGC_NEG_ZERO AGC_MASK
enum agc_ccs_class { AGC_POSITIVE, AGC_PLUS_ZERO, AGC_NEGATIVE, AGC_MINUS_ZERO };
agc_word agc_negate(agc_word value);
int16_t agc_to_int(agc_word value);
/* Caller supplies -16383..16383. Zero is encoded as +0. */
agc_word agc_from_int(int16_t value);
int agc_compare(agc_word a, agc_word b);
agc_word agc_add_word(agc_word a, agc_word b);
agc_register agc_extend(agc_word value);
agc_word agc_correct(agc_register value);
agc_register agc_add_register(agc_register a, agc_register b);
/* +1 / -1 if the two register sign bits disagree, otherwise 0. */
int agc_overflow(agc_register value);
enum agc_ccs_class agc_classify(agc_word value);
agc_word agc_diminished_abs(agc_word value);
#endif
