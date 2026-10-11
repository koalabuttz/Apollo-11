/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "native_source_dispatch_check.h"
int main(void){static apollo_overlay_state image,before;uint8_t service,n,irq;uint16_t failures=native_source_dispatch_guards();
 for(service=0;service<2u;++service)for(n=0;n<64u;++n)for(irq=0;irq<2u;++irq){uint16_t f=native_source_dispatch_check(service,n,irq,&image,&before);if(f)fprintf(stderr,"service%u case%u IRQ%u failures%u\n",service,n,irq,f);failures=(uint16_t)(failures+f);}
 if(failures)return 1;
 puts("PASS256 complete original priority/sleep dispatch transitions, deferred IRQ requests and4 domain guards");return 0;
}
