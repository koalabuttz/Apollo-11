/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "phase_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(void) { uint16_t n; for(n=0;n<PH_CASE_COUNT;++n) { phase_case c;
 phase_prepare(&c,n); printf("%u %u",n,(unsigned)phase_change(&c.state,c.words,c.count,c.two));
 phase_words(&c,emit,0); putchar('\n'); } return 0; }
