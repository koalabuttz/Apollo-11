/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_DOT_H
#define GUIDANCE_DOT_H
#include "guidance_divide.h"
/* DOT with a stable six-word operand and vector MODE=-1. Produces the
 * original unrounded triple MPAC0/1/2 and DP MODE=+0. BUF0130/0131 alias
 * elapsed.buf/divide.buf_low; BUF0132 is the pointed-to word (can be the
 * normalization scratch field). DOTINC0136/DOTRET0137 alias divide.dvsign /
 * divide.dvnormct. Original return address is07332; these are arithmetic
 * debris, not a native continuation. Preserve unrelated words/operand and
 * MAXDVSW. A/L and MPAC3..6 retain source exchanges, not input vectors.
 * Distinct owned objects/ordinary words and producer exclusion are caller
 * obligations. Unsupported MODE rejects unchanged and returns0; success1.
 * Stop at DANZIG; interpreter ADDRWD/bank/dispatch are outside this boundary. */
uint8_t p66_dot(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,
               p66_divide_scratch *,agc_word *buf_third,const agc_word operand[6]);
#endif
