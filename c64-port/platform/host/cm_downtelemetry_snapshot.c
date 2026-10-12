/* SPDX-License-Identifier: MIT */
/* Native cm-downtelemetry cases in the oracle's format (tools/cm_downtelemetry_reference.c). */
#include <stdio.h>
#include "cm_downtelemetry_cases.h"
int main(void){unsigned n,i,k;
 for(n=0;n<CM_DNTM_CASES;++n){apollo_overlay_state s;cm_dntm_io io;unsigned steps=cm_dntm_steps((uint16_t)n);
  cm_dntm_begin(&s,&io,(uint16_t)n);printf("%u",n);
  for(k=0;k<steps;++k){cm_dntm_entry(&s,&io,(uint16_t)n,(uint16_t)k);
   if(cm_dntm_downrupt(&s,&io)!=EXEC_OK){fprintf(stderr,"CM downtelemetry %u step %u stopped at %05o\n",n,k,(unsigned)s.words[5]);return 1;}
   printf(" %u %u %u",(unsigned)io.io.channel13,(unsigned)io.channel34,(unsigned)io.channel35);}
  printf(" %u %u %u",(unsigned)io.io.trap31a,(unsigned)io.io.trap31b,(unsigned)io.io.trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;}
