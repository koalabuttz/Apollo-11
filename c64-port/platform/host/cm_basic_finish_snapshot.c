/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_basic_finish_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_BASIC_FINISH_CASES;++n){exec_transfer next;enum exec_status status;cm_basic_finish_input(&s,&io,(uint16_t)n);status=cm_basic_finish(&s,&io,&next);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11,next.interpretive,next.address.location,next.address.bank);}return 0;}
