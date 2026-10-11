/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_CASES_H
#define GUIDANCE_CASES_H
#include "guidance.h"
#define GN_SCALAR_COUNT 544u
#define GN_TARGET_COUNT 547u
#define GN_CASE_COUNT 12835u
#define GN_TARGET_ROWS 568u
enum gn_operation { GN_MP, GN_DAS, GN_ROD };
typedef struct { p66_rate_state state; enum gn_operation operation; } guidance_case;
void gn_prepare(guidance_case *c, uint16_t number);
uint8_t gn_steps(uint16_t number);
void gn_next(guidance_case *c, uint16_t number, uint8_t step);
/* Input-only implementation uses local word encoding and no native routines. */
void gn_words(const guidance_case *c, uint16_t words[6]);
#ifndef GUIDANCE_INPUT_ONLY
void gn_apply(guidance_case *c);
unsigned gn_check(guidance_case *c);
#endif
#endif
