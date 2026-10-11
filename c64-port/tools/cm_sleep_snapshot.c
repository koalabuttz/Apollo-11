/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_sleep_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_SLEEP_CASES;++n){cm_native_io io;exec_transfer transfer;enum exec_status status;cm_sleep_input(&s,&io,n);status=cm_basic_sleep(&s,&io,&transfer);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}return 0;}
