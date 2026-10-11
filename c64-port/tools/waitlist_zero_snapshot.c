/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_zero_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(void) { uint16_t n; for(n=0;n<WL_ZERO_CASE_COUNT;++n) { waitlist w; exec_address task; wl_zero_prepare(&w,n,&task); printf("%u %u",n,(unsigned)wl_schedule(&w,20,task)); wl_zero_words(&w,emit,0); putchar('\n'); } return 0; }
