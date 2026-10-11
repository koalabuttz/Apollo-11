/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_clockjob_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_CLOCKJOB_CASES;++n){cm_native_io io;enum cm_clockjob_boundary status;cm_clockjob_input(&s,&io,n);status=cm_clockjob_apply(&s,&io,n);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
