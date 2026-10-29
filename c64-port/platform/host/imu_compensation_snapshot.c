/* SPDX-License-Identifier: MIT */
/* Native imu-compensation cases in the oracle's format: case, kind, all
 * 2048 erasable words and the interrupt-enable state at the stop. */
#include <stdio.h>
#include "imu_compensation_cases.h"
int main(void){unsigned n,i,kind;uint16_t ie;
 for(n=0;n<ICP_CASES;++n){apollo_overlay_state s;
  kind=icp_run(&s,(uint16_t)n,&ie);
  if(kind!=ICP_STOPPED){fprintf(stderr,"imu-compensation case %u kind %u at %05o\n",n,kind,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u\n",(unsigned)ie);
 }return 0;}
