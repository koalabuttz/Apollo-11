/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "overlay_platform.h"
#include "native_overlay_dispatch.h"
#include "p66_native.h"
#include "lm_interpreter_exchange_native.h"
#include "lm_interpreter_change_fixture.h"
#include "lm_interpreter_exchange_payload.h"
#include "lm_interpreter_exchange_target.h"
static c64_overlay_io backend_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
static lm_interpreter_change_fixture *const fixture=(lm_interpreter_change_fixture *)(uintptr_t)0xc000;
typedef struct {executive state;exec_native runtime;exec_irq_queue queue;lm_interpreter_exchange_binding binding;} exchange_metadata;
static exchange_metadata *const meta=(exchange_metadata *)(uintptr_t)0xc100;
typedef char metadata_fits_reserved_lease[(sizeof(exchange_metadata)<=0x700)?1:-1];
/* c000..c01f fixture, c080mail; c100..c7ff transient scheduler view/metadata.
 * All metadata remains resident, canonical source image remains7000. */
static uint8_t enter(void *u){uint8_t p;(void)u;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}static void leave(void *u,uint8_t p){(void)u;if(p&4u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");}static const exec_exclusion guard={enter,leave,0};
static uint16_t change(void *u,apollo_overlay_state *m,p66_bias_io *io,exec_transfer *transfer){lm_interpreter_exchange_arguments args={io,transfer};uint16_t result=0xffff;(void)u;if(m!=image||apollo_overlay_call(&loader,154,(uint16_t)(uintptr_t)&args,&result)!=APOLLO_OVERLAY_OK)return 0xffff;return result;}
static enum exec_action callback(executive *state,void *user,exec_address *resume){enum exec_action action=lm_interpreter_exchange_callback(state,user,resume);exec_request ticket={037000,{02001,062007},0};if(exec_irq_post(&meta->queue,&ticket)!=EXEC_OK)meta->runtime.fault=EXEC_QUEUE_FULL;return action;}
static exec_continuation resolve(void *u,exec_address address,enum exec_context_kind kind){if(address.location==05126&&(address.bank&076000u)==064000u&&kind==EXEC_CONTEXT_INTERPRETIVE)return callback;return lm_interpreter_exchange_resolve(u,address,kind);}
static uint16_t scheduler(void *u,exec_overlay_frame *frame,uint16_t stage){uint16_t result=0xffff;(void)u;if(apollo_overlay_call(&loader,exec_overlay_module(stage,frame->action),(uint16_t)(uintptr_t)frame,&result)!=APOLLO_OVERLAY_OK)return 0xffff;return result;}static const exec_overlay_dispatch services={scheduler,0};
static uint32_t fingerprint(void){uint32_t h=2166136261UL;uint16_t i,w;for(i=0;i<2052u;++i){w=i<2048u?image->words[i]:i==2048u?fixture->io.superbank:i==2049u?fixture->io.interrupt_enabled:i==2050u?fixture->channel11:fixture->channel14;h=(h^(uint8_t)w)*16777619UL;h=(h^(uint8_t)(w>>8))*16777619UL;}return h;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t i,result,failures=0;uint8_t initial,p,port;mail[0]=0;port=*(volatile uint8_t *)1;initial=enter(0);leave(0,initial);apollo_overlay_init(&loader,lm_exchange_modules,35,c64_overlay_backend(&backend_io),(uint8_t *)(uintptr_t)0x8000,image);
 for(i=0;i<64u;++i){if(i&1u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");p=enter(0);fixture->row=(uint16_t)(i&1u);result=0xffff;
  if(apollo_overlay_call(&loader,(uint8_t)(1u+i/2u),(uint16_t)(uintptr_t)fixture,&result)!=APOLLO_OVERLAY_OK||result){++failures;leave(0,p);continue;}
  memset(meta,0,sizeof *meta);p66_native_import_executive(&meta->state,image);meta->binding.runtime=&meta->runtime;meta->binding.image=image;meta->binding.io=&fixture->io;meta->binding.invoke_change=change;
  if(exec_native_resolver_init(&meta->runtime,&meta->state,resolve,&meta->binding,&meta->binding)!=EXEC_OK||exec_native_bind_irq(&meta->runtime,&meta->queue,&guard)!=EXEC_OK||exec_overlay_bind(&meta->runtime,&services)!=EXEC_OK)++failures;
  meta->runtime.running=1;meta->runtime.current_kind=EXEC_CONTEXT_INTERPRETIVE;meta->runtime.current.location=05126;meta->runtime.current.bank=(uint16_t)(image->words[6]|fixture->io.superbank);meta->runtime.interpreter_control.edop=0;meta->runtime.interpreter_control.intbit15=040000;leave(0,p);
  if(exec_native_step(&meta->runtime)!=EXEC_OK||meta->binding.callbacks!=1||meta->binding.status!=EXEC_OK||meta->queue.read!=0||meta->queue.write!=1||fingerprint()!=lm_change_hash[i]||meta->runtime.current.location!=lm_change_next[i][0]||meta->runtime.current.bank!=lm_change_next[i][1]||meta->runtime.current_kind!=(lm_change_next[i][2]?EXEC_CONTEXT_INTERPRETIVE:EXEC_CONTEXT_BASIC))++failures;
  __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(i&1u?4u:0u)||*(volatile uint8_t *)1!=port)++failures;
 }
 leave(0,initial);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=0;mail[5]=3;mail[0]=0xa5;return failures!=0;
}
