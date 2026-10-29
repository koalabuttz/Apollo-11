/* SPDX-License-Identifier: MIT */
#include "p66_descent_runtime.h"
#include "p66_descent_control_payload.h"
#include "p66_display_job_payload.h"
#include "p66_complete_fixture.h"
#include "p66_complete_live_vectors.h"
#include "native_overlay_dispatch.h"
#include "overlay_platform.h"
#include "p66_complete_modules.h"
typedef struct {exec_native runtime;p66_display_job_binding binding;exec_irq_queue queue;} metadata;
typedef char metadata_fits[(sizeof(metadata)<=0x80u)?1:-1];
typedef char executive_fits[(sizeof(executive)<=0x288u)?1:-1];
/* Fixed unchanged framework: canonical$7000..7fff, overlay$8000..bfff,
 * proofmetadata$c000..c03f, mailbox$c080..c085, Executive$c100..c387,
 * taskleases$c388..c3d3 reserved, runtime/binding/queue$c400..c47d,
 * normalC softwarestack$c800..cfff. No stage-slot frame survives a swap. */
static metadata *const meta=(metadata *)(uintptr_t)0xc400;
static executive *const state=(executive *)(uintptr_t)0xc100;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
static c64_overlay_io io={8,0,0,0,0,0};static apollo_overlay_runtime loader;
static uint16_t failures,checkpoints;
static uint8_t enter(void*u){uint8_t p;(void)u;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(void*u,uint8_t p){(void)u;if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static const exec_exclusion guard={enter,leave,0};
static uint16_t invoke(void*u,apollo_overlay_state*m,uint16_t selector){uint16_t result=0;(void)u;if(loader.state!=m||apollo_overlay_call(&loader,(uint8_t)selector,selector,&result)!=APOLLO_OVERLAY_OK)return 0xffff;return result;}
static uint16_t control(void*u,apollo_overlay_state*m,p66_bias_io *external,agc_word *channel,uint16_t selector){p66_descent_control_arguments args={external,channel};uint16_t result=0;(void)u;if(loader.state!=m||apollo_overlay_call(&loader,(uint8_t)selector,(uint16_t)(uintptr_t)&args,&result)!=APOLLO_OVERLAY_OK)return 0xffff;return result;}
static uint16_t display(void*u,apollo_overlay_state*m,p66_bias_io*external,agc_word*channel,uint16_t selector){p66_display_job_arguments args={external,channel};uint16_t result=0;(void)u;if(selector<300u||selector>321u||loader.state!=m||apollo_overlay_call(&loader,(uint8_t)(60u+selector-300u),(uint16_t)(uintptr_t)&args,&result)!=APOLLO_OVERLAY_OK)return 0xffff;return result;}
static exec_continuation resolve_display(void*u,exec_address a,enum exec_context_kind kind){uint8_t i;(void)u;for(i=0;i<P66_DISPLAY_JOB_MAPPING_COUNT;++i){const exec_context_mapping*m=&p66_display_job_mappings[i];if(m->address.location==a.location&&m->address.bank==a.bank&&m->kind==kind)return m->call;}return 0;}
static uint16_t scheduler(void *u,exec_overlay_frame *frame,uint16_t stage){uint16_t result=0;(void)u;if(apollo_overlay_call(&loader,exec_overlay_module(stage,frame->action),(uint16_t)(uintptr_t)frame,&result)!=APOLLO_OVERLAY_OK)return 0xffff;return result;}
static const exec_overlay_dispatch services={scheduler,0};
static p66_guidance_modules modules={invoke,0};
static void observe(void *u,const apollo_overlay_state *m,agc_word loc){uint16_t result=1;(void)u;if(m!=image||apollo_overlay_call(&loader,249,loc,&result)!=APOLLO_OVERLAY_OK||result)++failures;++checkpoints;}
static void run(uint8_t number){
 {p66_display_job_binding *full=&meta->binding;p66_descent_binding *b=&full->descent;uint16_t result=1;uint8_t i,token;enum exec_status status=EXEC_OK;
  b->basic.arithmetic.image=image;b->basic.arithmetic.producers=&guard;b->basic.arithmetic.observe=observe;b->basic.arithmetic.observer=0;b->basic.arithmetic.status=0;
  b->math_modules=&modules;b->invoke_control=control;b->control_modules=0;b->resolve_extra=resolve_display;b->extra_registry=0;b->finish_source=p66_display_job_finish;full->invoke_display=display;full->display_modules=0;b->basic.invoke_basic=0;b->basic.tasks=0;b->basic.display=0;
  P66_COMPLETE_FIXTURE->io=&b->basic.control;P66_COMPLETE_FIXTURE->channel14=&b->channel14;P66_COMPLETE_FIXTURE->channel11=&full->channel11;
  token=enter(0);status=(apollo_overlay_call(&loader,248,number,&result)==APOLLO_OVERLAY_OK&&result==0)?EXEC_OK:EXEC_OUT_OF_SCOPE;leave(0,token);
  if(status!=EXEC_OK){++failures;return;}
  state->alarm=0;p66_native_import_executive(state,image);
  if(p66_descent_init_resolved(&meta->runtime,state,b)!=EXEC_OK){++failures;return;}
  exec_irq_init(&meta->queue);
  if(exec_native_bind_irq(&meta->runtime,&meta->queue,&guard)!=EXEC_OK||exec_overlay_bind(&meta->runtime,&services)!=EXEC_OK){++failures;return;}
  meta->runtime.running=1;meta->runtime.current.location=03275;meta->runtime.current.bank=number?064067:064007;
  for(i=0;i<100u;++i){status=p66_basic_step(&b->basic);if(status!=EXEC_OK)break;}
  if(status!=EXEC_IDLE||P66_COMPLETE_FIXTURE->stage!=pcl_counts[number]||state->newjob!=AGC_NEG_ZERO||meta->runtime.fault!=EXEC_OK)++failures;
  failures=(uint16_t)(failures+P66_COMPLETE_FIXTURE->failures);
 }
}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t token,p,port;mail[0]=0;port=*(volatile uint8_t *)1;
 apollo_overlay_init(&loader,p66_complete_disk_modules,P66_COMPLETE_MODULES,c64_overlay_backend(&io),(uint8_t *)(uintptr_t)0x8000,image);
 run(0);token=enter(0);run(1);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)==0)++failures;leave(0,token);
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t *)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=(uint8_t)(checkpoints>>8);mail[5]=2;mail[0]=0xa5;return failures!=0;
}
