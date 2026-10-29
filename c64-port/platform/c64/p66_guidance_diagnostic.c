/* SPDX-License-Identifier: MIT */
#include "p66_guidance_fixture.h"
#include "p66_guidance_live_cases.h"
#include "p66_guidance_invoke.h"
#include "overlay_platform.h"
#include "p66_guidance_modules.h"
static c64_overlay_io io={8,0,0,0,0,0};static apollo_overlay_runtime loader;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
typedef char executive_view_fits[(sizeof(executive)<=0x300u)?1:-1];
typedef char scheduler_metadata_fits[(sizeof(exec_native)<=0x400u)?1:-1];
/* Reserved leases: fixture$c000..$c003, mailbox$c080..$c086,
 * transient Executive view$c100..$c3ff, native metadata$c400..$c7ff.
 * CanonicalAGCwords remain the only persistent source state at$7000. */
static executive *const state=(executive *)(uintptr_t)0xc100;
static exec_native *const runtime=(exec_native *)(uintptr_t)0xc400;
static uint16_t failures,checkpoints;
static uint8_t enter(void*u){uint8_t p;(void)u;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(void*u,uint8_t p){(void)u;if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint16_t invoke(void*u,apollo_overlay_state*m,uint16_t selector){uint16_t result=0;apollo_overlay_runtime*l=u;if(l->state!=m||apollo_overlay_call(l,(uint8_t)selector,selector,&result)!=APOLLO_OVERLAY_OK)return P66_NATIVE_UNSUPPORTED;return result;}
static void observe(void*u,const apollo_overlay_state*m,agc_word loc){uint16_t result=1;(void)u;(void)m;if(apollo_overlay_call(&loader,249,loc,&result)!=APOLLO_OVERLAY_OK||result!=0)++failures;++checkpoints;}
static const exec_exclusion guard={enter,leave,0};
static p66_guidance_modules calls={invoke,&loader};
static p66_native_binding binding={(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS,(exec_native *)(uintptr_t)0xc400,&guard,observe,0,P66_NATIVE_OK,p66_guidance_invoke,&calls};
static void check(uint8_t mixed){uint8_t n,i;
 for(n=0;n<2;++n){uint16_t result=1;uint8_t count=mixed?PGL_MIXED_STAGES:PGL_STAGES;
  if(apollo_overlay_call(&loader,248,(uint16_t)(n+(mixed?256u:0u)),&result)!=APOLLO_OVERLAY_OK||result!=0){++failures;return;}
  state->alarm=0;p66_native_import_executive(state,image);if(p66_native_guidance_init(runtime,state,&binding)!=EXEC_OK)++failures;runtime->running=1;runtime->current.location=03275;runtime->current.bank=n&1u?064067:064007;
  for(i=0;i<count;++i){if(mixed&&i==1)image->words[0067]=12;if(p66_native_step(&binding)!=EXEC_OK){++failures;break;}}
  if(p66_native_step(&binding)!=EXEC_OUT_OF_SCOPE||binding.status!=P66_NATIVE_BOUNDARY||P66_GUIDANCE_FIXTURE->stage!=count)++failures;
  failures+=P66_GUIDANCE_FIXTURE->failures;
 }
}
int main(void){volatile uint8_t*mail=(volatile uint8_t*)0xc080;uint8_t token,p,port;apollo_overlay_init(&loader,p66_guidance_disk_modules,P66_GUIDANCE_MODULES,c64_overlay_backend(&io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);mail[0]=0;port=*(volatile uint8_t*)1;check(0);check(1);token=enter(0);check(0);check(1);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)==0)++failures;leave(0,token);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=port)++failures;mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=(uint8_t)(checkpoints>>8);mail[5]=2;mail[0]=0xa5;return failures!=0;}
