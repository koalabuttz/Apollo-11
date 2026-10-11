/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "trig_cases.h"
unsigned tg_check(trig_case *);
static void reject(trig_case *c,uint8_t op){uint16_t before[TG_WORDS],after[TG_WORDS];tg_words(c,before);assert(!tg_apply(c,op));tg_words(c,after);assert(!memcmp(before,after,sizeof before));}
int main(void){trig_case c,original;uint16_t value;uint8_t op;assert(!tg_check(&c));tg_input(&original,3);for(value=0;value<=AGC_MASK;++value)for(op=0;op<4u;++op){c=original;if(value!=077776&&(value!=0||op==3u)){c.previous.base.previous.previous.previous.preparation.mpac[7]=value;reject(&c,op);}}
for(value=0;value<=AGC_MASK;++value){uint16_t before[TG_WORDS],after[TG_WORDS];c=original;tg_words(&c,before);if(value>07777u){assert(!p66_cdu_trig(&c.previous.base.previous.previous.previous.preparation,&c.previous.base.previous.previous.previous.scratch,&c.previous.base.scratch,&c.previous.norm,&c.trig,c.cdu,value,046007));tg_words(&c,after);assert(!memcmp(before,after,sizeof before));}c=original;if((value&077770u)!=046000u){assert(!p66_cdu_trig(&c.previous.base.previous.previous.previous.preparation,&c.previous.base.previous.previous.previous.scratch,&c.previous.base.scratch,&c.previous.norm,&c.trig,c.cdu,03663,value));tg_words(&c,after);assert(!memcmp(before,after,sizeof before));}}
puts("PASS128 complete original trig states and all unsupported MODE words reject unchanged");return 0;}
