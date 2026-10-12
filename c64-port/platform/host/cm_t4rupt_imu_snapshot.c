/* SPDX-License-Identifier: MIT */
/* Native cm-t4rupt-imu cases in the oracle's format: case, outcome kind and,
 * unless refused, all 2048 erasable words, SUPERBNK and channels 11, 12, 14,
 * 30 and 33. */
#include <stdio.h>
#include "cm_t4rupt_imu_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<CTI_CASES;++n){static apollo_overlay_state s;static cti_io io;
  kind=cti_run(&s,&io,(uint16_t)n);
  if(kind>CTI_REFUSED){fprintf(stderr,"cm-t4rupt-imu case %u not entered at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=CTI_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u %u %u %u",(unsigned)io.cm.superbank,(unsigned)io.hw.channel11,(unsigned)io.hw.channel12,(unsigned)io.hw.channel14,(unsigned)io.hw.channel30,(unsigned)io.hw.channel33);}
  putchar('\n');
 }return 0;}
