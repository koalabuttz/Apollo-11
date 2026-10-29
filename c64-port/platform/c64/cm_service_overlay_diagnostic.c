/* SPDX-License-Identifier: MIT */
#include "cm_service_overlay_binding.h"
#include "overlay_platform.h"
#include "cm_service_overlay_modules.h"
static c64_overlay_io disk_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;static uint16_t failures,checkpoints;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint8_t module(enum cm_service_group g){
 if(g==CM_SERVICE_T3_BEGIN||g==CM_SERVICE_TASK_FINISH)return 1;
 if(g>=CM_SERVICE_SETJTAG&&g<=CM_SERVICE_SVCT3_RECHECK)return 2;
 if(g==CM_SERVICE_WAITLIST||g==CM_SERVICE_FIXDELAY)return 3;
 if(g==CM_SERVICE_FINDVAC||g==CM_SERVICE_NOVAC)return 4;return 0;
}
static enum cm_service_status invoke(const cm_service_call *c,apollo_overlay_state *s,cm_native_io *io,void *user){uint16_t result=1;uint8_t id=module(c->group),token;enum apollo_overlay_status status;(void)user;
 if(s!=image||io!=CM_SERVICE_IO||!id)return CM_SERVICE_UNMAPPED;
 *CM_SERVICE_CALL=*c;token=enter();status=apollo_overlay_call(&loader,id,(uint16_t)c->group,&result);leave(token);
 return status==APOLLO_OVERLAY_OK&&result==0?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
}
static void replay(void){uint16_t n,step,result;cm_service_loop *r=CM_SERVICE_RUNTIME;cm_service_fixture *f=CM_SERVICE_FIXTURE;
 for(n=0;n<6u;++n){result=1;if(apollo_overlay_call(&loader,250,n,&result)!=APOLLO_OVERLAY_OK||result){++failures;return;}
  *r=(cm_service_loop){image,CM_SERVICE_IO,invoke,0,0,0};
  for(step=0;step<16u;++step){cm_service_call c;enum cm_service_status status=cm_service_identify(image,CM_SERVICE_IO,&c);
   if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME){++failures;return;}status=cm_service_step(r);f->group=(uint16_t)c.group;f->status=(uint16_t)status;
   result=1;if(apollo_overlay_call(&loader,251,step,&result)!=APOLLO_OVERLAY_OK||result){++failures;return;}++checkpoints;
   if(status==CM_SERVICE_RESUME)break;if(status!=CM_SERVICE_ADVANCED){++failures;return;}
  }
  if(step==16u||r->in_callback||r->completed_groups+1u!=f->step)++failures;failures+=f->failures;
 }
}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t token,p,port;
 apollo_overlay_init(&loader,cm_service_overlay_modules,6,c64_overlay_backend(&disk_io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);
 mail[0]=0;port=*(volatile uint8_t*)1;token=enter();leave((uint8_t)(token&~4u));replay();
 __asm__ volatile("php\npla":"=a"(p)::"memory");if(p&4u)++failures;
 (void)enter();replay();__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;leave(token);
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=6;mail[0]=0xa5;return failures!=0;
}
