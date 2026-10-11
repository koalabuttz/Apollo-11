/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_feedback_phased_cases.h"
int main(void){uint16_t number;for(number=0;number<TF_CASES;++number){throttle_feedback_case c;uint8_t phase;tf_input(&c,number);for(phase=0;phase<6u;++phase){uint16_t w[TF_WORDS],i;unsigned result=tf_phase(&c,phase);tf_words(&c,w);printf("%u %u %u",number,phase,result);for(i=0;i<TF_WORDS;++i)printf(" %u",w[i]);puts("");if(result==P66_FEEDBACK_ALARM_1302)break;}}return 0;}
