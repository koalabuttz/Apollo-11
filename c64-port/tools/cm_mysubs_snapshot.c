/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_mysubs_cases.h"
static int row(apollo_overlay_state *m,agc_word irq,uint32_t n,uint8_t selector,uint8_t phase){uint16_t h[5];h[0]=(uint16_t)n;h[1]=(uint16_t)(n>>16);h[2]=selector;h[3]=phase;h[4]=irq;return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(void){uint8_t selector;for(selector=0;selector<2u;++selector){uint32_t n;for(n=0;n<CM_MYSUBS_CASES;++n){apollo_overlay_state m;agc_word irq;cm_mysubs_input(&m,&irq,n,selector);if((selector?cm_dpadd(&m):cm_smallmp(&m))!=CM_MYSUBS_OK||row(&m,irq,n,selector,0)||cm_mysubs_return(&m)!=CM_MYSUBS_OK||row(&m,irq,n,selector,1))return 2;}}return 0;}
