/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_svct3_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_SVCT3_CASES;++n){enum cm_svct3_exit status;cm_svct3_input(&s,&io,(uint16_t)n);status=cm_svct3_phase(&s,&io,(uint8_t)(n/512u));printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
