/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_nv_busy_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;agc_word channel11;enum dsky_nv_mission mission;enum dsky_nv_busy_entry entry;uint16_t n,i;for(n=0;n<DSKY_NV_BUSY_CASES;++n){dsky_nv_busy_input(&s,&io,&channel11,n,&mission,&entry);printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel11);}return 0;}
