/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_restart_cases.h"
static void emit(void *u,agc_word w) { (void)u;printf(" %u",w); }
int main(void) { uint16_t n;cm_restart_case c;for(n=0;n<1280;++n) { int status;cm_rs_prepare(&c,n);status=cm_rs_apply(&c,n);printf("%u %d",n,status);cm_rs_words(&c,emit,0);putchar('\n'); }return 0; }
