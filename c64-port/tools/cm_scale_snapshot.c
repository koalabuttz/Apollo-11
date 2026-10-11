/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_scale_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_SCALE_CASES;++n){cm_native_io io;enum exec_status status;cm_scale_input(&s,&io,n);status=cm_scale_output(&s,&io,(enum dsky_scale_entry)(n%3u));printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
