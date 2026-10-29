/* SPDX-License-Identifier: MIT */
#include "cm_abort_snapshot_binding.h"
#include "overlay_platform.h"
#include "cm_abort_snapshot_modules.h"
static c64_overlay_io disk_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;static uint16_t failures,checkpoints;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;cm_abort_fixture *f=CM_ABORT_FIXTURE;uint16_t run,phase,result;uint8_t token,port,p;
 apollo_overlay_init(&loader,cm_abort_snapshot_modules,3,c64_overlay_backend(&disk_io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);mail[0]=0;port=*(volatile uint8_t*)1;token=enter();
 for(run=0;run<18u;++run){leave((uint8_t)((token&~4u)|((run&1u)?4u:0u)));result=1;if(apollo_overlay_call(&loader,254,run,&result)!=APOLLO_OVERLAY_OK||result){++failures;break;}
  for(phase=0;phase<(((f->source_case/2u)%7u)?2u:3u);++phase){uint8_t saved=enter();result=1;if(apollo_overlay_call(&loader,216,phase,&result)!=APOLLO_OVERLAY_OK||result)++failures;leave(saved);result=1;if(apollo_overlay_call(&loader,255,phase,&result)!=APOLLO_OVERLAY_OK||result)++failures;++checkpoints;}
  failures+=f->failures;__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(run&1u?4u:0u))++failures;
 }
 leave(token);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=(uint8_t)(checkpoints>>8);mail[0]=0xa5;return failures!=0;}
