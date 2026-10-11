/* SPDX-License-Identifier: MIT */
#include "overlay_platform.h"
#include "lm_interpreter_change_fixture.h"
#include "lm_interpreter_change_target.h"
static c64_overlay_io backend_io={8,0,0,0,0,0};
static apollo_overlay_runtime loader;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
/* Diagnostic metadata$c000..c01f, mailbox$c080..c085; production canonical
 * and fixed slot/stack leases unchanged. No fixture constructors on target. */
static lm_interpreter_change_fixture *const fixture=(lm_interpreter_change_fixture *)(uintptr_t)0xc000;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(p&4u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");}
static uint32_t fingerprint(void){uint32_t hash=2166136261UL;uint16_t i,word;for(i=0;i<2052u;++i){word=i<2048u?image->words[i]:i==2048u?fixture->io.superbank:i==2049u?fixture->io.interrupt_enabled:i==2050u?fixture->channel11:fixture->channel14;hash=(hash^(uint8_t)word)*16777619UL;hash=(hash^(uint8_t)(word>>8))*16777619UL;}return hash;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t i,failures=0,result;uint8_t initial,p,port;exec_transfer next;mail[0]=0;port=*(volatile uint8_t *)1;initial=enter();leave(initial);
 apollo_overlay_init(&loader,lm_change_modules,32,c64_overlay_backend(&backend_io),(uint8_t *)(uintptr_t)0x8000,image);
 for(i=0;i<64u;++i){if(i&1u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");p=enter();fixture->row=(uint16_t)(i&1u);result=0xffff;
  if(apollo_overlay_call(&loader,(uint8_t)(1u+i/2u),(uint16_t)(uintptr_t)fixture,&result)!=APOLLO_OVERLAY_OK||result)++failures;
  else if(lm_interpreter_change(image,&fixture->io,&next)!=EXEC_OK||fingerprint()!=lm_change_hash[i]||next.address.location!=lm_change_next[i][0]||next.address.bank!=lm_change_next[i][1]||next.interpretive!=lm_change_next[i][2])++failures;
  leave(p);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(i&1u?4u:0u)||*(volatile uint8_t *)1!=port)++failures;
 }
 leave(initial);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=0;mail[5]=2;mail[0]=0xa5;return failures!=0;
}
