/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "acceleration_phased_cases.h"
int main(void){uint16_t number;for(number=0;number<ACC_CASES;++number){acceleration_case c;uint8_t phase;acc_input(&c,number);for(phase=0;phase<4u;++phase){uint16_t w[ACC_WORDS],i;unsigned result=acc_phase(&c,phase);acc_words(&c,w);printf("%u %u %u",number,phase,result);for(i=0;i<ACC_WORDS;++i)printf(" %u",w[i]);puts("");if(result)break;}}return 0;}
