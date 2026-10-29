/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_clock_display_mission.h"
#include "cm_clock_display_mission_cases.h"
#include "cm_clock_blocked_display.h"
static apollo_overlay_state s;static cm_native_io io;
static const unsigned selected[7]={1,4,5,8,9,12,13};
int main(void){unsigned n,step,i,index;for(index=0;index<7u;++index){n=selected[index];cm_mission_loop r={&s,&io,cm_clock_display_invoke,0,{0,0},{{0,0},0},0,1,0};cm_clock_display_input(&s,&io,(uint16_t)n);r.pending_t3=1;
 for(step=0;step<40u;++step){cm_mission_call call;enum cm_service_status status=cm_clock_display_identify(&r,&call);if(status!=CM_SERVICE_ADVANCED)return 2;status=cm_clock_display_step(&r);
 printf("%u %u %u %u",n,step,call.group,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11,r.irq.in_isr,r.irq.pending_danzig,r.transfer.interpretive,r.transfer.address.location,r.transfer.address.bank);
 if(s.words[5]==03312u||(s.words[5]==06037u&&!r.pending_t3&&!r.irq.in_isr))break;
 if(status!=CM_SERVICE_ADVANCED)return 2;}if(step==40u||s.words[5]!=03312u)return 2;
 for(step=7;step<=11u;++step){enum cm_service_status status=cm_clock_blocked_display_step(&r,(uint8_t)step);
 if(status!=CM_SERVICE_ADVANCED){fprintf(stderr,"normal mission n%u p%u unsupportedPC%oBB%o\n",n,step,s.words[5],s.words[6]);return 2;}
 printf("%u %u %u %u",n,step+40u,300u+step,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11,r.irq.in_isr,r.irq.pending_danzig,r.transfer.interpretive,r.transfer.address.location,r.transfer.address.bank);
 }
 }return 0;}
