/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_initialization_cases.h"
static void emit(void *u,agc_word w){(void)u;printf(" %u",w);}
int main(void){uint16_t n;for(n=0;n<P66_INIT_CASE_COUNT;++n){p66_init_case c;enum p66_init_status result;p66_init_prepare(&c,n);result=c.fast ? p66_fastchng(&c.state) : p66_stoprate(&c.state);printf("%u %u",n,(unsigned)result);p66_init_words(&c,emit,0);putchar('\n');}return 0;}
