/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_overlay_binding.h"
#include "overlay_platform.h"
#include "cm_clock_display_overlay_modules.h"
static c64_overlay_io disk_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;static uint16_t failures,checkpoints;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint8_t module(unsigned g){
 if(g>=210u&&g<=218u)return 7;
 if(g>=200u&&g<=204u)return 6;
 if(g>=100u&&g<=103u)return 5;
 if(g==CM_SERVICE_T3_BEGIN||g==CM_SERVICE_TASK_FINISH)return 1;
 if(g>=CM_SERVICE_SETJTAG&&g<=CM_SERVICE_SVCT3_RECHECK)return 2;
 if(g==CM_SERVICE_WAITLIST||g==CM_SERVICE_FIXDELAY)return 3;
 if(g==CM_SERVICE_FINDVAC||g==CM_SERVICE_NOVAC)return 4;return 0;
}
static enum cm_service_status invoke(const cm_mission_call *c,apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq,exec_transfer *next,void *user){uint16_t result=1;uint8_t id=module(c->group),token;enum apollo_overlay_status status;(void)user;(void)irq;(void)next;
 if(s!=image||io!=CM_CLOCK_DISPLAY_IO||!id)return CM_SERVICE_UNMAPPED;
 *CM_CLOCK_DISPLAY_CALL=*c;token=enter();status=apollo_overlay_call(&loader,id,(uint16_t)c->group,&result);leave(token);
 return status==APOLLO_OVERLAY_OK&&result==0?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
}
static void replay(void){uint16_t n,step,result;cm_mission_loop *r=CM_CLOCK_DISPLAY_RUNTIME;cm_mission_fixture *f=CM_CLOCK_DISPLAY_FIXTURE;
 for(n=0;n<3u;++n){result=1;if(apollo_overlay_call(&loader,250,n,&result)!=APOLLO_OVERLAY_OK||result){++failures;return;}
  *r=(cm_mission_loop){image,CM_CLOCK_DISPLAY_IO,invoke,0,{0,0},{{0,0},0},0,1,0};
  for(step=0;step<40u;++step){cm_mission_call c;enum cm_service_status status=cm_clock_display_identify(r,&c);
   if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME){++failures;return;}status=cm_clock_display_step(r);f->group=(uint16_t)c.group;f->status=(uint16_t)status;
   result=1;if(apollo_overlay_call(&loader,251,step,&result)!=APOLLO_OVERLAY_OK||result){++failures;return;}++checkpoints;
   if(image->words[5]==03312u||(image->words[5]==06037u&&!r->pending_t3&&!r->irq.in_isr))break;if(status!=CM_SERVICE_ADVANCED){++failures;return;}
  }
  if(step==40u||r->in_callback||r->completed_groups!=f->step||r->irq.in_isr||r->irq.pending_danzig||r->pending_t3)++failures;failures+=f->failures;
 }
}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t token,p,port;
 apollo_overlay_init(&loader,cm_clock_display_overlay_modules,9,c64_overlay_backend(&disk_io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);
 mail[0]=0;port=*(volatile uint8_t*)1;token=enter();leave((uint8_t)(token&~4u));replay();
 __asm__ volatile("php\npla":"=a"(p)::"memory");if(p&4u)++failures;
 (void)enter();replay();__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;leave(token);
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=3;mail[0]=0xa5;return failures!=0;
}
