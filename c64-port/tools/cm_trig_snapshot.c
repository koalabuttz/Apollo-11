/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_trig_cases.h"
int main(void){uint8_t selector;for(selector=0;selector<3u;++selector){uint32_t n;for(n=0;n<cm_trig_count(selector);++n){apollo_overlay_state m;agc_word irq;uint16_t head[3];cm_trig_input(&m,&irq,(uint16_t)n,selector);if((selector<2u?cm_single_trig(&m,selector):cm_quictrig(&m,&irq))!=CM_TRIG_OK)return 2;head[0]=(uint16_t)n;head[1]=selector;head[2]=irq;if(fwrite(head,sizeof head,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}}return 0;}
