/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "acceleration_cases.h"
#define ACCELERATION_FROZEN_IMPLEMENTATION
#include "acceleration_vectors.h"
static acceleration_case c;
unsigned acceleration_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<ACCELERATION_FROZEN_ROWS;++row){uint16_t w[ACC_WORDS],expected,path;frozen_words r;acc_input(&c,acceleration_indices[row]);path=(uint16_t)acc_apply(&c);acc_words(&c,w);
        if(!acceleration_reference_begin(&r,row)||!frozen_words_next(&r,&expected)||expected!=path)++failures;
        for(i=0;i<ACC_WORDS;++i)if(!frozen_words_next(&r,&expected)||expected!=w[i])++failures;
        if(!frozen_words_finished(&r))++failures;
    }return failures;
}
#ifndef ACCELERATION_CHECK_ONLY
int main(void){
    unsigned failures=acceleration_check();uint32_t mode;acceleration_case before;
    if(sizeof(acceleration_case)!=224u)++failures;
    for(mode=1;mode<=AGC_MASK;++mode){acc_input(&c,4100);c.previous.base.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;before=c;if(acc_apply(&c)!=P66_ACCELERATION_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}
    {unsigned k;for(k=0;k<3u;++k){acc_input(&c,8200);if(k==0)c.previous.base.previous.previous.push.base=0400;else c.previous.base.previous.previous.push.pushloc=(agc_word)(c.previous.base.previous.previous.push.base+(k==1?2u:44u));before=c;if(acc_apply(&c)!=P66_ACCELERATION_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}}
    printf("%u complete acceleration states, unsupported MODE/VAC/pointer unchanged: %s\n",ACCELERATION_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
