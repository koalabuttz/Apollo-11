/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_radar_leadin_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_RADAR_CASES;++n){apollo_overlay_state m;p66_bias_io io;enum lm_radar_result status;unsigned i;
 lm_radar_input(&m,&io,n);status=lm_radar_leadin(&m,&io);if(status==LM_RADAR_REJECT)return 2;
 printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",m.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);
 }return 0;}
