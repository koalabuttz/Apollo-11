/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "horizontal_phased_cases.h"
static void emit(horizontal_phased_case *c,uint16_t n,uint8_t stage,unsigned result){uint16_t w[HHP_WORDS],head[3]={n,stage,(uint16_t)result};hhp_words(c,w);if(fwrite(head,sizeof head,1,stdout)!=1||fwrite(w,sizeof w,1,stdout)!=1)fprintf(stderr,"write failed\n");}
int main(void){uint16_t n;for(n=0;n<HH_CASES;++n){horizontal_phased_case c;hhp_input(&c,n);emit(&c,n,13,0);while(c.next<13u){uint8_t stage;unsigned result=hhp_step(&c,&stage);if(result==P66_HORIZONTAL_OUT_OF_SCOPE||result==P66_HORIZONTAL_ALARM_1302)return 2;emit(&c,n,stage,result);}}return 0;}
