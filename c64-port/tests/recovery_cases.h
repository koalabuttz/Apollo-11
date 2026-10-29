/* SPDX-License-Identifier: MIT */
#ifndef RECOVERY_CASES_H
#define RECOVERY_CASES_H
#include "recovery.h"
#define RECOVERY_CASE_COUNT 49152
#define RECOVERY_WORD_COUNT 47
typedef struct { recovery_abort_state state; enum recovery_abort_entry entry; } recovery_case;
void recovery_prepare(recovery_case *,uint16_t);
uint16_t recovery_target(uint8_t);
void recovery_words(const recovery_case *,void (*)(void *,agc_word),void *);
#endif
