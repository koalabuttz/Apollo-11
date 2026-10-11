/* SPDX-License-Identifier: MIT */
#include "p66_status_check.h"
static uint8_t enter(void *u){uint8_t p;(void)u;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(void *u,uint8_t p){(void)u;if(p&4u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");}
typedef char view_fits[(sizeof(executive)<=0x300)?1:-1];
typedef char runtime_fits[(sizeof(exec_native)<=0x400)?1:-1];
int main(void){static const exec_exclusion guard={enter,leave,0};
 volatile uint8_t *mail=(volatile uint8_t *)(uintptr_t)0xc080;
 uint16_t errors;uint8_t original,after,token,port;
 mail[0]=0;port=*(volatile uint8_t *)1;
 __asm__ volatile("php\npla":"=a"(original)::"memory");
 errors=p66_status_check((apollo_overlay_state *)(uintptr_t)0x7000,
  (executive *)(uintptr_t)0xc100,(exec_native *)(uintptr_t)0xc400,&guard);
 __asm__ volatile("php\npla":"=a"(after)::"memory");if((original&4u)!=(after&4u))++errors;
 token=enter(0);
 errors+=p66_status_check((apollo_overlay_state *)(uintptr_t)0x7000,
  (executive *)(uintptr_t)0xc100,(exec_native *)(uintptr_t)0xc400,&guard);
 __asm__ volatile("php\npla":"=a"(after)::"memory");if(!(after&4u))++errors;
 leave(0,token);if(*(volatile uint8_t *)1!=port)++errors;
 mail[1]=(uint8_t)errors;mail[2]=(uint8_t)(errors>>8);mail[3]=10;mail[4]=(uint8_t)sizeof(((p66_native_binding *)0)->status);
 mail[0]=0xa5;return errors!=0;
}
