/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_CASES_H
#define CM_RESTART_CASES_H
#include "cm_restart.h"
#define CM_RS_CASE_COUNT 1280
#define CM_RS_ROW_WORDS 429
typedef struct { cm_executive exec; cm_waitlist wait; cm_restart_state record; cm_restart_phases phases; uint8_t group; } cm_restart_case;
void cm_rs_prepare(cm_restart_case *,uint16_t);
void cm_rs_words(const cm_restart_case *,void (*)(void *,agc_word),void *);
#ifndef CM_RESTART_INPUT_ONLY
int cm_rs_apply(cm_restart_case *,uint16_t);
#endif
#endif
