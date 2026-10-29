/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_mission_loop_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,step,i;for(n=0;n<CM_MISSION_LOOP_CASES;++n){cm_mission_loop r={&s,&io,cm_mission_builtin_invoke,0,{0,0},{{0,0},0},0,1,0};cm_mission_loop_input(&s,&io,(uint16_t)n);
 for(step=0;step<20u;++step){cm_mission_call call;enum cm_service_status status=cm_mission_identify(&r,&call);if(status!=CM_SERVICE_ADVANCED)return 2;status=cm_mission_step(&r);
 printf("%u %u %u %u",n,step,call.group,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,r.irq.in_isr,r.irq.pending_danzig,r.transfer.interpretive,r.transfer.address.location,r.transfer.address.bank);
 if(call.group==CM_MISSION_DANZIG)break;
 if(status!=CM_SERVICE_ADVANCED)return 2;}if(step==20u)return 2;}return 0;}
