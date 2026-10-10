/* SPDX-License-Identifier: MIT */
#ifndef RESTART_CASES_H
#define RESTART_CASES_H
#include "restart.h"
#define RS_CASE_COUNT 190
#define RS_ROW_WORDS 448
typedef struct {
    executive exec;
    waitlist wait;
    restart_state record;
    restart_phases phases;
    agc_word code;
    exec_address caller;
} restart_case;
void rs_prepare(restart_case *, uint16_t);
uint8_t rs_steps(uint16_t);
int rs_apply(restart_case *, uint16_t, uint8_t);
typedef void (*rs_word_sink)(void *, agc_word);
void rs_words(const restart_case *, rs_word_sink, void *);
uint32_t rs_digest(const restart_case *, int);
#endif
