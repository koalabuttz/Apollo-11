/* SPDX-License-Identifier: MIT */
#ifndef PHASE_CASES_H
#define PHASE_CASES_H
#include "phase.h"
#define PH_CASE_COUNT 49152
#define PH_WORD_COUNT 56
typedef struct { phase_state state; agc_word words[5]; uint8_t two,count; } phase_case;
uint16_t phase_target(uint8_t);
void phase_prepare(phase_case *,uint16_t);
void phase_words(const phase_case *,void (*)(void *,agc_word),void *);
#endif
