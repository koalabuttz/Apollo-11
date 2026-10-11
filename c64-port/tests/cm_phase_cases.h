/* SPDX-License-Identifier: MIT */
#ifndef CM_PHASE_CASES_H
#define CM_PHASE_CASES_H
#include "cm_phase.h"
#define CM_PH_CASE_COUNT 49152
#define CM_PH_WORD_COUNT 56
typedef struct { cm_phase_state state; agc_word words[5]; uint8_t two,count; } cm_phase_case;
uint16_t cm_phase_target(uint8_t);
void cm_phase_prepare(cm_phase_case *,uint16_t);
void cm_phase_words(const cm_phase_case *,void (*)(void *,agc_word),void *);
#endif
