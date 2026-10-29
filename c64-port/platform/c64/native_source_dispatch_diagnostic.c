/* SPDX-License-Identifier: MIT */
#include "native_source_dispatch_check.h"
static apollo_overlay_state image,before;
int main(void){uint8_t service,n,irq;uint16_t failures=native_source_dispatch_guards(),cases=0;volatile uint8_t *mail=(volatile uint8_t *)0xc080;
 mail[0]=0;
#ifdef NATIVE_DISPATCH_ONLY_SERVICE
 for(service=NATIVE_DISPATCH_ONLY_SERVICE;service<NATIVE_DISPATCH_ONLY_SERVICE+1u;++service)
#else
 for(service=0;service<2u;++service)
#endif
 for(n=0;n<NATIVE_SOURCE_DISPATCH_CASES;++n)for(irq=0;irq<2u;++irq){uint8_t p;
  if(irq)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");
  failures=(uint16_t)(failures+native_source_dispatch_check(service,n,irq,&image,&before));++cases;
  __asm__ volatile("php\npla":"=a"(p)::"memory");if(((p>>2)&1u)!=irq)++failures;
 }
 __asm__ volatile("cli":::"memory");
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)cases;mail[4]=(uint8_t)(cases>>8);mail[5]=4;mail[0]=0xa5;return failures!=0;
}
