/* SPDX-License-Identifier: MIT */
/* Native lm-downtelemetry cases in the oracle's format (tools/lm_downtelemetry_reference.c). */
#include <stdio.h>
#include "lm_downtelemetry_cases.h"
int main(void){unsigned n,i,k;
 for(n=0;n<LM_DNTM_CASES;++n){apollo_overlay_state s;lm_dntm_io io;unsigned steps=lm_dntm_steps((uint16_t)n);
  lm_dntm_begin(&s,&io,(uint16_t)n);printf("%u",n);
  for(k=0;k<steps;++k){lm_dntm_entry(&s,&io,(uint16_t)n,(uint16_t)k);
   if(lm_dntm_downrupt(&s,&io)!=EXEC_OK){fprintf(stderr,"LM downtelemetry %u step %u stopped at %05o\n",n,k,(unsigned)s.words[5]);return 1;}
   printf(" %u %u %u",(unsigned)io.channel13,(unsigned)io.channel34,(unsigned)io.channel35);}
  printf(" %u %u %u",(unsigned)io.trap31a,(unsigned)io.trap31b,(unsigned)io.trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;}
