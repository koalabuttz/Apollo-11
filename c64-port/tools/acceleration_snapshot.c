/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "acceleration_cases.h"
int main(void){uint16_t n;for(n=0;n<ACC_CASES;++n){acceleration_case c;uint16_t w[ACC_WORDS],i;unsigned result;acc_input(&c,n);result=acc_apply(&c);acc_words(&c,w);printf("%u %u",n,result);for(i=0;i<ACC_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
