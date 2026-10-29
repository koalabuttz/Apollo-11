/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_mode_admit_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status=0;for(n=0;n<DSKY_MODE_ADMIT_CASES;++n){p66_bias_io io;uint16_t alarm=0;enum dsky_mode_admit_route route=DSKY_MODE_ADMIT_CALLER;dsky_mode_admit_input(&s,&io,n);printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,alarm,(unsigned)route);}return 0;}
