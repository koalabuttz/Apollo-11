/* SPDX-License-Identifier: MIT */
/* Every returned word is captured for external exact comparison. */
#include "overlay_runtime.h"
#include "overlay_platform.h"
#include "passive_storage_modules.h"
static c64_overlay_io disk_io={8,0,0,0,0,0};
static apollo_overlay_runtime loader;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static volatile uint16_t *const output=(volatile uint16_t *)(uintptr_t)0x6000;
static uint16_t failures;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(p&4u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");}
static uint16_t pattern(uint16_t i){return (uint16_t)(i*29u+0x1234u);}
int main(void){
 volatile uint8_t *mail=(volatile uint8_t *)(uintptr_t)0xc080;
 uint16_t i,bank,address,word;uint8_t run,token,irq,port;
 mail[0]=0;port=*(volatile uint8_t*)1;token=enter();
 for(i=0;i<2048u;++i)image->words[i]=pattern(i);
 apollo_overlay_init(&loader,passive_storage_modules,6,c64_overlay_backend(&disk_io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);
 for(run=0;run<2u;++run){
  leave((uint8_t)((token&~4u)|(run?4u:0u)));
  for(bank=0;bank<40u;++bank){
   for(i=0;i<1024u;++i){
    address=(uint16_t)(bank*1024u+i);word=0xffffu;irq=enter();
    if(apollo_overlay_call(&loader,(uint8_t)(200u+address/7168u),address,&word)!=APOLLO_OVERLAY_OK)++failures;
    leave(irq);output[i]=word;
   }
   for(i=0;i<2048u;++i)if(image->words[i]!=pattern(i))++failures;
   __asm__ volatile("php\npla":"=a"(irq)::"memory");
   if(((irq&4u)!=0u)!=(run!=0u)||*(volatile uint8_t*)1!=port)++failures;
   mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)bank;mail[4]=run;mail[0]=0xa5;
  }
 }
 leave(token);mail[0]=0x5a;return failures!=0u;
}
