/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_nv_consumer_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;agc_word channel11;enum dsky_nv_mission mission;uint16_t n,i;uint8_t phase;for(n=0;n<DSKY_NV_CONSUMER_CASES;++n){dsky_nv_consumer_input(&s,&io,&channel11,n,&mission);for(phase=0;phase<dsky_nv_consumer_count(n);++phase){if(dsky_nv_consumer_step(&s,&io,&channel11,mission,dsky_nv_consumer_stage_at(n,phase))!=P66_BIAS_OK){fprintf(stderr,"consumer domain %u phase%u PC%o\n",n,phase,s.words[5]);return 2;}printf("%u %u",n,phase);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel11);}}return 0;}
