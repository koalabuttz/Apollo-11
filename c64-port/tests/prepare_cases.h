/* SPDX-License-Identifier: MIT */
#ifndef PREPARE_CASES_H
#define PREPARE_CASES_H
#include "guidance_prepare.h"
#define PP_WORD_COUNT 39u
#define PP_TARGET_COUNT 147u
#define PP_CASE_COUNT 1171u
#define PP_TARGET_ROWS 324u
typedef p66_preparation prepare_case;
void pp_prepare(prepare_case *c, uint16_t number);
uint8_t pp_steps(uint16_t number);
void pp_next(prepare_case *c, uint16_t number, uint8_t step);
void pp_words(const prepare_case *c, uint16_t words[PP_WORD_COUNT]);
void pp_from_words(prepare_case *c, const uint16_t words[PP_WORD_COUNT]);
#ifndef PREPARE_INPUT_ONLY
uint8_t pp_apply(prepare_case *c, uint8_t stage);
unsigned pp_check(prepare_case *c);
#endif
#endif
