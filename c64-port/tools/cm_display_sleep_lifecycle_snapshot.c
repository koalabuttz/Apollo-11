/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_display_sleep_lifecycle_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;uint8_t phase;for(n=0;n<CM_DISPLAY_SLEEP_LIFECYCLE_CASES;++n){cm_native_io io;cm_display_sleep_lifecycle_input(&s,&io,n);for(phase=0;phase<4u;++phase){uint16_t status=cm_display_sleep_lifecycle_apply(&s,&io,phase);printf("%u %u",n*4u+phase,status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}}return 0;}
