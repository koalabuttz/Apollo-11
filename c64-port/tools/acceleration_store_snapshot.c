/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "acceleration_phased_cases.h"
#include "guidance_acceleration_store.h"
int main(void){uint16_t number;for(number=0;number<ACC_CASES;++number){acceleration_case c;uint8_t phase;acc_input(&c,number);for(phase=0;phase<5u;++phase){uint16_t w[ACC_WORDS],i;unsigned result; p66_preparation *s=&c.previous.base.previous.previous.previous.preparation; p66_pushdown *p=&c.previous.base.previous.previous.push; if(phase<3u)result=acc_phase(&c,phase);else if(phase==3u)result=(unsigned)p66_acceleration_store_phase(s,&c.previous.norm,p);else result=(unsigned)p66_acceleration_add_phase(s,&c.previous.base.previous.previous.previous.scratch,&c.previous.base.previous.elapsed,&c.previous.norm,p);acc_words(&c,w);printf("%u %u %u",number,phase,result);for(i=0;i<ACC_WORDS;++i)printf(" %u",w[i]);puts("");if(result)break;}}return 0;}
