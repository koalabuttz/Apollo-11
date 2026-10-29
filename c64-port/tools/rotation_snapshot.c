/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "rotation_cases.h"
int main(void){uint32_t n;for(n=0;n<ROT_CASES;++n){rotation_case c;uint16_t w[ROT_WORDS],i;unsigned result;rot_input(&c,(uint16_t)n);result=rot_apply(&c);rot_words(&c,w);printf("%u %u",(unsigned)n,result);for(i=0;i<ROT_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
