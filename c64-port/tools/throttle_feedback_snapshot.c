/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_feedback_cases.h"
int main(void){uint16_t n;for(n=0;n<TF_CASES;++n){throttle_feedback_case c;uint16_t w[TF_WORDS],i;unsigned result;tf_input(&c,n);result=tf_apply(&c);tf_words(&c,w);printf("%u %u",n,result);for(i=0;i<TF_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
