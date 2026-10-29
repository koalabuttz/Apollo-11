/* SPDX-License-Identifier: MIT */
#include "lm_scan_exception_fixture.h"
#include "overlay_platform.h"
#include "lm_scan_exception_modules.h"
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
static apollo_overlay_runtime loader;static c64_overlay_io io={8,0,0,0,0,0};
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(!(p&4u))__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){uint16_t n,failures=0,result=0;uint8_t entry=enter(),port=*(volatile uint8_t*)1;volatile uint8_t *mail=(volatile uint8_t*)0xc080;lm_scan_exception_fixture *f=LM_SCAN_EXCEPTION_FIXTURE;mail[0]=0;apollo_overlay_init(&loader,lm_scan_exception_modules,32,c64_overlay_backend(&io),(uint8_t*)(uintptr_t)0x8000,image);f->failures=0;
 for(n=0;n<64u;++n){uint8_t token=enter();if(apollo_overlay_call(&loader,(uint8_t)(160u+n/2u),(uint16_t)(n&1u),&result)!=APOLLO_OVERLAY_OK||result){++failures;leave(token);continue;}f->status=(uint16_t)lm_scan_exception(image,&f->io,(enum lm_scan_entry)f->entry);if(apollo_overlay_call(&loader,(uint8_t)(160u+n/2u),(uint16_t)(2u+(n&1u)),&result)!=APOLLO_OVERLAY_OK||result)++failures;leave(token);}
 if(*(volatile uint8_t*)1!=port)++failures;failures=(uint16_t)(failures+f->failures);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=0;mail[5]=2;mail[0]=0xa5;leave(entry);return failures!=0;}
