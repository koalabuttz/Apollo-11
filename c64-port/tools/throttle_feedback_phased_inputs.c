/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_feedback_cases.h"
int main(void){uint16_t n;for(n=0;n<TF_CASES;++n){throttle_feedback_case c;uint16_t w[TF_WORDS],i;tf_input(&c,n);tf_words(&c,w);printf("%u",n);for(i=0;i<TF_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
