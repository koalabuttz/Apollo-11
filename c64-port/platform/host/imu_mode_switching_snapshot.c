/* SPDX-License-Identifier: MIT */
/* Native imu-mode-switching cases in the oracle's format: case, kind, all
 * 2048 erasable words, the interrupt-enable state and channels 11-14. */
#include <stdio.h>
#include "imu_mode_switching_cases.h"
int main(void){unsigned n,i,kind;uint16_t ie;agc_word ch[4];
 for(n=0;n<IMS_CASES;++n){apollo_overlay_state s;
  kind=ims_run(&s,(uint16_t)n,&ie,ch);
  if(kind!=IMS_STOPPED){fprintf(stderr,"imu-mode-switching case %u kind %u at %05o\n",n,kind,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u",(unsigned)ie);
  for(i=0;i<4u;++i)printf(" %u",(unsigned)ch[i]);
  printf("\n");
 }return 0;}
