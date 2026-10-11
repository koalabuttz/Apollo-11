/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_clocktask_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_CLOCKTASK_CASES;++n){enum cm_clocktask_exit status;cm_clocktask_input(&s,&io,(uint16_t)n);status=cm_clocktask_phase(&s,&io,(uint8_t)(n/512u));printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
