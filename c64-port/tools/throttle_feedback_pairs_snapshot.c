/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_feedback_pairs_cases.h"
static void emit(throttle_feedback_pairs_case *c,uint16_t n,unsigned status){uint16_t w[TFP_WORDS],head[3]={n,c->control.loc,(uint16_t)status};tfp_words(c,w);if(fwrite(head,sizeof head,1,stdout)!=1||fwrite(w,sizeof w,1,stdout)!=1){fprintf(stderr,"write failure\n");}}
int main(void){uint32_t n;for(n=0;n<TF_CASES;++n){throttle_feedback_pairs_case c;unsigned count=0;tfp_input(&c,(uint16_t)n);emit(&c,(uint16_t)n,0);while(c.control.next_pair!=255){unsigned status=tfp_apply(&c);if(status==P66_FEEDBACK_OUT_OF_SCOPE||status==P66_FEEDBACK_ALARM_1302||++count>20){fprintf(stderr,"case%u status%u loc%o\n",(unsigned)n,status,c.control.loc);return 2;}emit(&c,(uint16_t)n,status);}}return 0;}
