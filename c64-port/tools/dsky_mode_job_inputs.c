/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_mode_job_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status=0;for(n=0;n<DSKY_MODE_JOB_CASES;++n){p66_bias_io io;enum dsky_mode_job_route route=DSKY_MODE_JOB_END;dsky_mode_job_input(&s,&io,n);printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}return 0;}
