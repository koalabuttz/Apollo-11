/* SPDX-License-Identifier: MIT */
/* Native lm-imu-performance cases in the oracle's format: case, kind, all
 * 2048 erasable words and the interrupt-enable state. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_imu_performance_cases.h"
int main(int argc,char **argv){unsigned n,i,kind,first=0,last=IPT_CASES;uint16_t ie;
 if(argc==3){first=(unsigned)atoi(argv[1]);last=(unsigned)atoi(argv[2]);}
 for(n=first;n<last;++n){apollo_overlay_state s;
  kind=ipt_run(&s,(uint16_t)n,&ie);
  if(kind!=IPT_STOPPED){fprintf(stderr,"lm-imu-performance case %u kind %u at %05o loc %05o\n",n,kind,(unsigned)s.words[5],(unsigned)s.words[0164]);}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u\n",(unsigned)ie);
 }return 0;}
