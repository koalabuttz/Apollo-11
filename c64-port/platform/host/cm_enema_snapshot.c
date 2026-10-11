/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_enema_cases.h"
static void emit(void *u,agc_word w) { (void)u;printf(" %u",w); }
int main(void) { uint16_t n;cm_enema_case c;for(n=0;n<4096;++n) { int status;cm_enema_prepare(&c,n);status=cm_enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io);printf("%u %d",n,status);cm_enema_words(&c,emit,0);putchar('\n'); }return 0; }
