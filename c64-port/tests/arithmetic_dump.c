/* SPDX-License-Identifier: MIT */
#include "agc_word.h"
#define ARITH_EXTEND(x) agc_extend(x)
#define ARITH_CORRECT(x) agc_correct(x)
#define ARITH_ADD(a,b) agc_add_register(a,b)
#include "arithmetic_stream.h"
int main(void) { return arithmetic_stream(); }
