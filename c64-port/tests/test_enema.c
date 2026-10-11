/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "enema_check.h"
#include "enema_vectors.h"
int main(void) {
 enema_case c,before; unsigned n;
 for(n=0;n<ENEMA_TARGET_COUNT;++n) assert(enema_check((uint8_t)n,&c)==0);
 enema_prepare(&c,0); c.io.bbank=0; before=c;
 assert(enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io)==ENEMA_OUT_OF_SCOPE);
 assert(memcmp(&before,&c,sizeof c)==0);
 enema_prepare(&c,0); c.io.main_keys=0100000; before=c;
 assert(enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io)==ENEMA_OUT_OF_SCOPE);
 assert(memcmp(&before,&c,sizeof c)==0);
 puts("PASS:128 whole457word original assembly ENEMA states; unsupported inputs retain state");
 return 0;
}
