/* SPDX-License-Identifier: MIT */
/* Native AGC_BLOCK_TWO_SELF-CHECK cases in the oracle's format
 * (tools/self_check_reference.c); argv[1] is the mission (0 Luminary099, 1 Comanche055). */
#include <stdio.h>
#include <stdlib.h>
#include "self_check_cases.h"
int main(int argc,char **argv){unsigned n,i,mission;
 if(argc!=2)return 2;
 mission=(unsigned)atoi(argv[1])!=0u;
 for(n=0;n<SELF_CHECK_CASES;++n){apollo_overlay_state s;p66_bias_io io;uint16_t steps=0;
  if(self_check_run(mission,&s,&io,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"SELF-CHECK %u/%u stopped at %05o\n",mission,n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)io.superbank);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
