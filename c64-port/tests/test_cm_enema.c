/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_enema_check.h"
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"CM ENEMA failure%d\n",__LINE__);++errors; } } while(0)
int main(void) {
 cm_enema_case c,before;unsigned errors=0;
 errors+=cm_enema_check(&c);
 cm_enema_prepare(&c,0);c.io.bbank=02003;before=c;CHECK(cm_enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io)==CM_ENEMA_OUT_OF_SCOPE && memcmp(&c,&before,sizeof c)==0);
 cm_enema_prepare(&c,0);c.previous.wait.time3=040000;before=c;CHECK(cm_enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io)==CM_ENEMA_OUT_OF_SCOPE && memcmp(&c,&before,sizeof c)==0);
 cm_enema_prepare(&c,0);c.io.superbank=1;before=c;CHECK(cm_enema_begin(&c.previous.record,&c.previous.exec,&c.previous.wait,&c.previous.phases,&c.io)==CM_ENEMA_OUT_OF_SCOPE && memcmp(&c,&before,sizeof c)==0);
 if(errors!=0) return 1;
 puts("PASS:4096 CM original ENEMA states,8 full boundary vectors, unsupported inputs retain state");return 0;
}
