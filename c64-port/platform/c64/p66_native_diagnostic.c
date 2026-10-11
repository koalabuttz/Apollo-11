/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_native.h"
#include "p66_native_fixture.h"
#include "overlay_platform.h"
#include "p66_native_modules.h"
static c64_overlay_io io={8,0,0,0,0,0};static apollo_overlay_runtime modules;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
/* Explicit reserved scratch lease: fixture$c000..$c03f, mailbox$c080,
 * transient Executive view$c100..$c3ff, native metadata$c400..$c7ff.
 * Every source boundary commits image; every reuse imports it again. */
typedef char executive_view_fits[(sizeof(executive)<=0x300u)?1:-1];
typedef char scheduler_metadata_fits[(sizeof(exec_native)<=0x400u)?1:-1];
static executive *const state=(executive *)(uintptr_t)0xc100;
static exec_native *const runtime=(exec_native *)(uintptr_t)0xc400;
static uint16_t failures;
static uint8_t enter(void*u){uint8_t p;(void)u;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(void*u,uint8_t p){(void)u;if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint16_t invoke(void *user,apollo_overlay_state *m,uint16_t selector) {
 uint16_t result=0;apollo_overlay_runtime *loader=user;
 if(loader->state!=m)return P66_NATIVE_UNSUPPORTED;
 if(apollo_overlay_call(loader,(uint8_t)selector,selector,&result)!=APOLLO_OVERLAY_OK)return P66_NATIVE_UNSUPPORTED;
 return result;
}
static void observe(void*u,const apollo_overlay_state*m,agc_word loc){
 uint16_t result=1;(void)u;(void)m;
 if(apollo_overlay_call(&modules,251,loc,&result)!=APOLLO_OVERLAY_OK || result!=0)++failures;
}
#ifndef P66_NATIVE_DIAGNOSTIC_CASES
#define P66_NATIVE_DIAGNOSTIC_CASES 3u
#endif
static void check(uint8_t mixed){
 static const uint8_t samples[8]={0,1,4,2,3,5,6,7};
 uint8_t n;unsigned i;static const exec_exclusion guard={enter,leave,0};
 p66_native_binding binding={image,runtime,&guard,observe,0,P66_NATIVE_OK,invoke,&modules};
 for(n=0;n<P66_NATIVE_DIAGNOSTIC_CASES;++n){uint16_t result=1;
  if(apollo_overlay_call(&modules,250,(uint16_t)(samples[n]+(mixed?256u:0u)),&result)!=APOLLO_OVERLAY_OK || result!=0){++failures;return;}
  state->alarm=0;p66_native_import_executive(state,image);
  if(exec_native_context_init(runtime,state,p66_native_mappings,P66_NATIVE_MAPPING_COUNT,&binding)!=EXEC_OK)++failures;
  runtime->running=1;runtime->current.location=03275;runtime->current.bank=samples[n]&1u?064067:064007;
  if(mixed){if(p66_native_step(&binding)!=EXEC_OK)++failures;image->words[0067]=12;
   for(i=0;i<6;++i)if(p66_native_step(&binding)!=EXEC_OK)++failures;
  }else for(i=0;i<5;++i)if(p66_native_step(&binding)!=EXEC_OK)++failures;
  if(p66_native_step(&binding)!=EXEC_OUT_OF_SCOPE || binding.status!=P66_NATIVE_BOUNDARY || P66_NATIVE_FIXTURE->stage!=(mixed?7u:5u))++failures;
  failures+=P66_NATIVE_FIXTURE->failures;
 }
}
int main(void){
 volatile uint8_t*mail=(volatile uint8_t*)0xc080;uint8_t token,p,port;
 apollo_overlay_init(&modules,p66_native_modules,7,c64_overlay_backend(&io),
 (uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);
 mail[0]=0;port=*(volatile uint8_t*)1;check(0);check(1);
 token=enter(0);check(0);check(1);
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)==0)++failures;leave(0,token);
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u) || *(volatile uint8_t*)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)(P66_NATIVE_DIAGNOSTIC_CASES*12u);mail[4]=2;
 mail[0]=0xa5;return failures!=0;
}
