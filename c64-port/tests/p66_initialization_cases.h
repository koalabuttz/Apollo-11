/* SPDX-License-Identifier: MIT */
#ifndef P66_INITIALIZATION_CASES_H
#define P66_INITIALIZATION_CASES_H
#include "p66_initialization.h"
#define P66_INIT_CASE_COUNT 1024
#define P66_INIT_WORD_COUNT 17
#define P66_INIT_TARGET_COUNT 128
typedef struct {p66_basic_initialization state; uint8_t fast;} p66_init_case;
void p66_init_prepare(p66_init_case *,uint16_t);
void p66_init_words(const p66_init_case *,void (*)(void *,agc_word),void *);
#endif
