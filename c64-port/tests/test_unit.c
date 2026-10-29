/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "unit_cases.h"
#define UNIT_FROZEN_IMPLEMENTATION
#include "unit_vectors.h"
static unit_case c;
unsigned unit_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<UNIT_FROZEN_ROWS;++row){uint32_t index=unit_reference_cases[row];uint16_t w[UNIT_WORDS],expected,path;frozen_words r;unit_input(&c,index);path=(uint16_t)unit_apply(&c,(uint8_t)(index%3u));unit_words(&c,w);
        if(!unit_reference_begin(&r,row)||!frozen_words_next(&r,&expected)||expected!=path)++failures;
        for(i=0;i<UNIT_WORDS;++i)if(!frozen_words_next(&r,&expected)||expected!=w[i])++failures;
        if(!frozen_words_finished(&r))++failures;
    }return failures;
}
#ifndef UNIT_CHECK_ONLY
int main(void){
    unsigned failures=unit_check();uint32_t mode;unit_case before;
    if(sizeof(unit_case)!=208u)++failures;
    for(mode=0;mode<=077777u;++mode){uint8_t op;for(op=0;op<3u;++op){
        if(mode==(op?077776u:0u))continue;
        unit_input(&c,10u+op);c.base.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;before=c;
        if(unit_apply(&c,op)!=P66_NORM_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;
    }}
    {uint8_t op;for(op=1;op<3u;++op){unit_input(&c,op);c.base.previous.previous.push.base=0400;before=c;if(unit_apply(&c,op)!=P66_NORM_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}}
    printf("%u complete ABS/ABVAL/UNIT states, unsupported MODE/VAC unchanged: %s\n",UNIT_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
