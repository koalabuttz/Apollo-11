/* SPDX-License-Identifier: MIT */
#ifndef P66_STARTUP_CASES_H
#define P66_STARTUP_CASES_H
#include "p66_startup.h"
#define P66_STARTUP_CASE_COUNT 73728UL
#define P66_STARTUP_MAX_WORDS 332
#define P66_STARTUP_TARGET_COUNT 128
typedef struct {
 uint8_t mode;
 union {
  p66_operator_state op;
  struct { executive exec; p66_task_io io; } task;
  struct { phase_state phase; waitlist wait; } vert;
 } value;
} p66_startup_case;
void p66_startup_prepare(p66_startup_case *,uint32_t);
uint32_t p66_startup_target(uint8_t);
void p66_startup_words(const p66_startup_case *,void (*)(void *,agc_word),void *);
#ifndef P66_STARTUP_INPUT_ONLY
agc_word p66_startup_apply(p66_startup_case *);
#endif
#endif
