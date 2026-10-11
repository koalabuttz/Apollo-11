/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_clockjob_cases.h"
#include "cm_quictrig.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,p;for(n=0;n<512u;++n){cm_native_io io;cm_clockjob_input(&s,&io,n);for(p=0;p<4u;++p){unsigned status;if(p==0u)status=(unsigned)cm_clockjob_enter(&s,&io);else if(p==1u){if(cm_quictrig(&s,&io.interrupt_enabled)!=CM_TRIG_OK)return 2;status=6;}else if(p==2u)status=(unsigned)cm_clockjob_return(&s,&io);else status=(unsigned)cm_clockjob_select(&s,&io);printf("%u %u %u",n,p,status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
