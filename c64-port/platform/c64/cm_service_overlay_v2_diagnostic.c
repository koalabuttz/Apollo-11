/* SPDX-License-Identifier: MIT
 * The CM service overlay proof (COMANCHE_SERVICE_OVERLAY.md) on overlay ABI 2:
 * the same four native service bodies and two proof-data modules, loaded from
 * an REU store into the task slot (T3RUPT2/TASKOVER, CLOKTASK/SVCT3,
 * WAITLIST/FIXDELAY) or the job slot (NOVAC/FINDVAC, proof data), with
 * KERNAL banked out and resident RAM interrupt vectors. */
#include "cm_service_overlay_binding.h"
#include "overlay_reu.h"
#define CM 2u
#define SERVICES 0x0202u
extern void ov2_irq_entry(void);
extern void ov2_nmi_entry(void);
extern volatile uint16_t ov2_irq_count;
uint16_t c64_ov2_invoke(void *,uint8_t *,apollo_ov2_frame *,uint16_t);
static uint8_t record[APOLLO_REU_INDEX_RECORD],header[APOLLO_OV2_HEADER_BYTES];
static apollo_reu_store store;
static apollo_ov2_runtime rt;
static uint16_t failures,checkpoints,loads;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint16_t module(enum cm_service_group g){
 if(g==CM_SERVICE_T3_BEGIN||g==CM_SERVICE_TASK_FINISH)return APOLLO_OV2_ID(APOLLO_LANE_A,1);
 if(g>=CM_SERVICE_SETJTAG&&g<=CM_SERVICE_SVCT3_RECHECK)return APOLLO_OV2_ID(APOLLO_LANE_A,2);
 if(g==CM_SERVICE_WAITLIST||g==CM_SERVICE_FIXDELAY)return APOLLO_OV2_ID(APOLLO_LANE_A,3);
 if(g==CM_SERVICE_FINDVAC||g==CM_SERVICE_NOVAC)return APOLLO_OV2_ID(APOLLO_LANE_A,4);
 return 0;
}
/* Counts payload copies (the header-only read is 24 bytes). */
static uint8_t (*store_fetch)(void *,const apollo_ov2_dirent *,uint16_t,uint8_t *,uint16_t);
static uint8_t counted_fetch(void *u,const apollo_ov2_dirent *d,uint16_t o,uint8_t *dest,uint16_t n){
 if(n>APOLLO_OV2_HEADER_BYTES)++loads;return store_fetch(u,d,o,dest,n);
}
static uint8_t call(uint8_t context,uint16_t id,uint16_t selector,uint16_t *result){
 apollo_ov2_cont c,resume;uint8_t action;
 c.module=id;c.selector=selector;*result=1;
 return (uint8_t)(apollo_ov2_run(&rt,context,c,result,&action,&resume)==APOLLO_OV2_OK&&action==APOLLO_OV2_DONE);
}
static enum cm_service_status invoke(const cm_service_call *c,apollo_overlay_state *s,cm_native_io *io,void *user){
 uint16_t result,id=module(c->group);uint8_t token,ok;(void)user;
 if(s!=image||io!=CM_SERVICE_IO||!id)return CM_SERVICE_UNMAPPED;
 *CM_SERVICE_CALL=*c;token=enter();
 /* Interrupt-level bodies run in the task context; admission in a job's. */
 ok=call(APOLLO_OV2_LANE(id)==APOLLO_LANE_A&&(id&0xffu)<4u?APOLLO_OV2_TASK_CONTEXT:0u,id,(uint16_t)c->group,&result);
 leave(token);
 return ok&&result==0?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
}
static void replay(void){uint16_t n,step,result;cm_service_loop *r=CM_SERVICE_RUNTIME;cm_service_fixture *f=CM_SERVICE_FIXTURE;
 for(n=0;n<6u;++n){if(!call(0,APOLLO_OV2_ID(APOLLO_LANE_PROOF,1),n,&result)||result){++failures;return;}
  *r=(cm_service_loop){image,CM_SERVICE_IO,invoke,0,0,0};
  for(step=0;step<16u;++step){cm_service_call c;enum cm_service_status status=cm_service_identify(image,CM_SERVICE_IO,&c);
   if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME){++failures;return;}status=cm_service_step(r);f->group=(uint16_t)c.group;f->status=(uint16_t)status;
   if(!call(0,APOLLO_OV2_ID(APOLLO_LANE_PROOF,2),step,&result)||result){++failures;return;}++checkpoints;
   if(status==CM_SERVICE_RESUME)break;if(status!=CM_SERVICE_ADVANCED){++failures;return;}
  }
  if(step==16u||r->in_callback||r->completed_groups+1u!=f->step)++failures;failures+=f->failures;
 }
}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t token,p,port;
 mail[0]=0;port=*(volatile uint8_t*)1;
 __asm__ volatile("sei");
 *(volatile uint8_t *)1=0x35;
 *(volatile uint16_t *)0xfffe=(uint16_t)(uintptr_t)ov2_irq_entry;
 *(volatile uint16_t *)0xfffa=(uint16_t)(uintptr_t)ov2_nmi_entry;
 __asm__ volatile("cli");
 if(!apollo_reu_store_open(&store,0,CM,SERVICES,record))++failures;
 else if(apollo_reu_store_verify(&store,(uint8_t *)0xc100)!=APOLLO_OV2_OK)++failures;
 {apollo_ov2_store backend=apollo_reu_store_backend(&store,c64_ov2_invoke,0);
  store_fetch=backend.fetch;backend.fetch=counted_fetch;
  apollo_ov2_init(&rt,backend,(uint8_t *)0x8000,(uint8_t *)0xe000,header,image,CM,SERVICES,0);}
 token=enter();leave((uint8_t)(token&~4u));replay();
 __asm__ volatile("php\npla":"=a"(p)::"memory");if(p&4u)++failures;
 (void)enter();replay();__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;leave(token);
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=0x35)++failures;
 if(ov2_irq_count==0u)++failures;
 __asm__ volatile("sei");*(volatile uint8_t *)1=port;__asm__ volatile("cli");
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=6;
 mail[5]=(uint8_t)loads;mail[6]=(uint8_t)(loads>>8);mail[7]=(uint8_t)ov2_irq_count;mail[8]=(uint8_t)(ov2_irq_count>>8);
 mail[0]=0xa5;return failures!=0;
}
