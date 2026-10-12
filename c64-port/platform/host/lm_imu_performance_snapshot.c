/* SPDX-License-Identifier: MIT */
/* Native lm-imu-performance cases in the oracle's format: case, kind, all
 * 2048 erasable words and the interrupt-enable state. With -DIPT_C64_PART=0
 * an argument selects the C64 part whose chained steps to replay. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_imu_performance_cases.h"
int main(int argc,char **argv){unsigned n,i,kind;uint16_t ie;
#if !(defined IPT_C64_PART && IPT_C64_PART>0)
 if(argc==2)ipt_part=(unsigned)atoi(argv[1]);
#else
 (void)argc;(void)argv;
#endif
 for(n=0;n<IPT_CASES;++n){apollo_overlay_state s;
  kind=ipt_run(&s,(uint16_t)n,&ie);
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u\n",(unsigned)ie);
 }return 0;}
