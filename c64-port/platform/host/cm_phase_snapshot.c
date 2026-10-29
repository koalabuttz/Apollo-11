/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_phase_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(void) { uint16_t n; for(n=0;n<CM_PH_CASE_COUNT;++n) { cm_phase_case c;
 cm_phase_prepare(&c,n); printf("%u %u",n,(unsigned)cm_phase_change(&c.state,c.words,c.count,c.two));
 cm_phase_words(&c,emit,0); putchar('\n'); } return 0; }
