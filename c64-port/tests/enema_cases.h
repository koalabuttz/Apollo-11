/* SPDX-License-Identifier: MIT */
#ifndef ENEMA_CASES_H
#define ENEMA_CASES_H
#include "enema.h"
#include "restart_cases.h"
#define ENEMA_CASE_COUNT 4096
#define ENEMA_WORD_COUNT (RS_ROW_WORDS+9)
typedef struct { restart_case previous; enema_io io; } enema_case;
void enema_prepare(enema_case *,uint16_t);
uint16_t enema_target(uint8_t);
void enema_words(const enema_case *,rs_word_sink,void *);
#endif
