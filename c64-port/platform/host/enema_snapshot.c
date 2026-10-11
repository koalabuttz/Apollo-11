/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "enema_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(void) { uint16_t n; for(n=0;n<ENEMA_CASE_COUNT;++n) { enema_case c;
 enema_prepare(&c,n); printf("%u %u",n,(unsigned)enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io));
 enema_words(&c,emit,0); putchar('\n'); } return 0; }
