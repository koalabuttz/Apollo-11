/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_nv_update_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;enum dsky_nv_update_entry entry;uint16_t n,i;for(n=0;n<DSKY_NV_UPDATE_CASES;++n){dsky_nv_update_input(&s,&io,n,&mission,&entry);if(dsky_nv_update(&s,&io,mission,entry)!=P66_BIAS_OK){fprintf(stderr,"updateunsupported %u PC%oEB%o\n",n,s.words[5],s.words[3]);return 2;}printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
