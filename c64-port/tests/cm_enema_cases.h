/* SPDX-License-Identifier: MIT */
#ifndef CM_ENEMA_CASES_H
#define CM_ENEMA_CASES_H
#include "cm_enema.h"
#include "cm_restart_cases.h"
#define CM_ENEMA_CASE_COUNT 4096
#define CM_ENEMA_WORD_COUNT 439
typedef struct { cm_restart_case previous;cm_enema_io io; } cm_enema_case;
void cm_enema_prepare(cm_enema_case *,uint16_t);
void cm_enema_words(const cm_enema_case *,void (*)(void *,agc_word),void *);
#endif
