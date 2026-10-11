/* SPDX-License-Identifier: MIT */
#include "lm_recurrence_bridge_fixture.h"
#include "lm_taskover_resume.h"
#include "overlay_platform.h"
#include "lm_recurrence_bridge_modules.h"
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
static apollo_overlay_runtime loader;static c64_overlay_io io={8,0,0,0,0,0};
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(!(p&4u))__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){uint16_t n,failures=0,result=0;uint8_t entry=enter(),port=*(volatile uint8_t*)1;volatile uint8_t *mail=(volatile uint8_t*)0xc080;lm_recurrence_bridge_fixture *f=LM_RECURRENCE_FIXTURE;mail[0]=0;
 apollo_overlay_init(&loader,lm_recurrence_bridge_modules,32,c64_overlay_backend(&io),(uint8_t*)(uintptr_t)0x8000,image);f->failures=0;
 for(n=0;n<64u;++n){exec_address next;enum exec_status status;uint8_t token=enter();if(apollo_overlay_call(&loader,(uint8_t)(160u+n/2u),(uint16_t)(n&1u),&result)!=APOLLO_OVERLAY_OK||result){++failures;leave(token);continue;}
  status=(n&1u)?lm_taskover_resume(image,&f->io,&f->in_interrupt):lm_idle_start(image,&f->io,&f->channel11,&next);
  if(status!=EXEC_OK||((n&1u)==0&&(next.location!=image->words[5]||next.bank!=(uint16_t)(image->words[6]|f->io.superbank))))++failures;
  if(apollo_overlay_call(&loader,(uint8_t)(160u+n/2u),(uint16_t)(2u+(n&1u)),&result)!=APOLLO_OVERLAY_OK||result)++failures;leave(token);
 }
 if(*(volatile uint8_t*)1!=port)++failures;failures=(uint16_t)(failures+f->failures);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=0;mail[5]=2;mail[0]=0xa5;leave(entry);return failures!=0;
}
