/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "native_source_finish_check.h"
int main(void){static apollo_overlay_state image,expected;uint8_t n,irq;uint16_t failures=native_source_finish_guards();for(n=0;n<64u;++n)for(irq=0;irq<2u;++irq){uint16_t f=native_source_finish_check(n,irq,&image,&expected);if(f)fprintf(stderr,"finish%u IRQ%u failures%u\n",n,irq,f);failures=(uint16_t)(failures+f);}if(failures)return 1;puts("PASS128 complete original ENDOFJOB basic/interpreter/idle adoptions, retained IRQ requests and7 domain guards");return 0;}
