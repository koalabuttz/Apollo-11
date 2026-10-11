/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_charin_nv_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_CHARIN_NV_CASES;++n){cm_native_io io;uint16_t wrote;enum exec_status status;cm_charin_nv_input(&s,&io,&wrote,n);status=cm_charin_nv(&s,&io,&wrote);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11,wrote);}return 0;}
