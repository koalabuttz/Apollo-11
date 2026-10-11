/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "throttle_feedback_cases.h"
#define THROTTLE_FEEDBACK_FROZEN_IMPLEMENTATION
#include "throttle_feedback_vectors.h"
static throttle_feedback_case c;
unsigned throttle_feedback_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<THROTTLE_FEEDBACK_FROZEN_ROWS;++row){uint16_t w[TF_WORDS],expected,path;frozen_words r;tf_input(&c,throttle_feedback_indices[row]);path=(uint16_t)tf_apply(&c);tf_words(&c,w);
        if(!throttle_feedback_reference_begin(&r,row)||!frozen_words_next(&r,&expected)||expected!=path)++failures;
        for(i=0;i<TF_WORDS;++i)if(!frozen_words_next(&r,&expected)||expected!=w[i])++failures;
        if(!frozen_words_finished(&r))++failures;
    }return failures;
}
#ifndef THROTTLE_FEEDBACK_CHECK_ONLY
int main(void){
    unsigned failures=throttle_feedback_check();uint32_t mode;throttle_feedback_case before;
    if(sizeof(throttle_feedback_case)!=248u)++failures;
    for(mode=0;mode<=AGC_MASK;++mode){if(mode==077776u)continue;tf_input(&c,4100);c.previous.base.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;before=c;if(tf_apply(&c)!=P66_FEEDBACK_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}
    {unsigned k;for(k=0;k<3u;++k){tf_input(&c,8200);if(k==0)c.previous.base.previous.previous.push.base=0400;else c.previous.base.previous.previous.push.pushloc=(agc_word)(c.previous.base.previous.previous.push.base+(k==1?2u:44u));before=c;if(tf_apply(&c)!=P66_FEEDBACK_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}}
    printf("%u complete throttle_feedback states, unsupported MODE/VAC/pointer unchanged: %s\n",THROTTLE_FEEDBACK_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
