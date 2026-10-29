/* SPDX-License-Identifier: MIT */
#ifndef WAITLIST_ZERO_CASES_H
#define WAITLIST_ZERO_CASES_H
#include "waitlist.h"
#define WL_ZERO_CASE_COUNT 256
#define WL_ZERO_WORD_COUNT 29
void wl_zero_prepare(waitlist *,uint16_t,exec_address *);
void wl_zero_words(const waitlist *,void (*)(void *,agc_word),void *);
#endif
