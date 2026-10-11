/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_scalar_output_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;enum dsky_scalar_output_entry entry;agc_word channel;uint16_t n,i;for(n=0;n<DSKY_SCALAR_OUTPUT_CASES;++n){dsky_scalar_output_input(&s,&io,n,&mission,&entry,&channel);if(dsky_scalar_output(&s,&io,mission,entry,channel)!=P66_BIAS_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
