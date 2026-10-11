/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "recovery_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(void) { uint16_t n; for(n=0;n<RECOVERY_CASE_COUNT;++n) { recovery_case c;
 recovery_prepare(&c,n); printf("%u %u",n,(unsigned)recovery_abort(&c.state,c.entry));
 recovery_words(&c,emit,0); putchar('\n'); } return 0; }
