/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_service_loop_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,step,i;for(n=0;n<CM_SERVICE_LOOP_CASES;++n){cm_service_loop r={&s,&io,cm_service_builtin_invoke,0,0,0};cm_service_loop_input(&s,&io,(uint16_t)n);
 for(step=0;step<16u;++step){cm_service_call call;enum cm_service_status status=cm_service_identify(&s,&io,&call);if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME)return 2;status=cm_service_step(&r);
 printf("%u %u %u %u",n,step,(unsigned)call.group,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);
 if(status==CM_SERVICE_RESUME)break;
 if(status!=CM_SERVICE_ADVANCED)return 2;}
 if(step==16u)return 2;}return 0;}
