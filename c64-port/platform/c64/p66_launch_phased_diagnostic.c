/* SPDX-License-Identifier: MIT */
#include "overlay_platform.h"
#include "lm_interpreter_change_fixture.h"
#include "p66_application_service_payload.h"
#include "p66_launch_phased_target.h"
static c64_overlay_io backend_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
static lm_interpreter_change_fixture *const fixture=(lm_interpreter_change_fixture *)(uintptr_t)0xc000;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(p&4u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");}
static uint32_t fingerprint(void){uint32_t h=2166136261UL;uint16_t i,w;for(i=0;i<2050u;++i){w=i<2048u?image->words[i]:i==2048u?fixture->io.superbank:fixture->io.interrupt_enabled;h=(h^(uint8_t)w)*16777619UL;h=(h^(uint8_t)(w>>8))*16777619UL;}return h;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t i,phase,result,failures=0,checks=0;uint8_t initial,p,port;static const uint8_t stage[3]={50,83,52};p66_application_service_arguments args={&fixture->io,0,0,0,0};mail[0]=0;port=*(volatile uint8_t *)1;initial=enter();leave(initial);
 apollo_overlay_init(&loader,p66_launch_modules,19,c64_overlay_backend(&backend_io),(uint8_t *)(uintptr_t)0x8000,image);
 for(i=0;i<32u;++i){if(i&1u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");p=enter();fixture->row=(uint16_t)(i&1u);result=0xffff;
  if(apollo_overlay_call(&loader,(uint8_t)(1u+i/2u),(uint16_t)(uintptr_t)fixture,&result)!=APOLLO_OVERLAY_OK||result)++failures;
  else for(phase=0;phase<p66_launch_count[i];++phase){result=0xffff;if(apollo_overlay_call(&loader,stage[phase],(uint16_t)(uintptr_t)&args,&result)!=APOLLO_OVERLAY_OK||result!=p66_launch_status[i][phase]||fingerprint()!=p66_launch_hash[i][phase])++failures;++checks;}
  leave(p);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(i&1u?4u:0u)||*(volatile uint8_t *)1!=port)++failures;
 }
 leave(initial);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[5]=2;mail[0]=0xa5;return failures!=0;
}
