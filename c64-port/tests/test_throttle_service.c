/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "throttle_service_cases.h"
#define THROTTLE_SERVICE_FROZEN_IMPLEMENTATION
#include "throttle_service_vectors.h"
static throttle_service_case c;
unsigned throttle_service_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<THROTTLE_SERVICE_FROZEN_ROWS;++row){uint16_t w[TS_WORDS],expected,path;frozen_words r;ts_input(&c,throttle_service_indices[row]);path=(uint16_t)ts_apply(&c);ts_words(&c,w);
        if(!throttle_service_reference_begin(&r,row)||!frozen_words_next(&r,&expected)||expected!=path)++failures;
        for(i=0;i<TS_WORDS;++i)if(!frozen_words_next(&r,&expected)||expected!=w[i])++failures;
        if(!frozen_words_finished(&r))++failures;
    }return failures;
}
#ifndef THROTTLE_SERVICE_CHECK_ONLY
int main(void){
    unsigned failures=throttle_service_check();uint32_t k;throttle_service_case before;
    if(sizeof(throttle_service_case)!=294u)++failures;
    for(k=010000u;k<=0177777u;++k){ts_input(&c,8192);c.service.q=(agc_word)k;before=c;if(ts_apply(&c)||memcmp(&c,&before,sizeof c))++failures;}
    for(k=0;k<=AGC_MASK;++k){if(k==0||k==020||k==040||k==060)continue;ts_input(&c,8200);c.service.superbank=(agc_word)k;before=c;if(ts_apply(&c)||memcmp(&c,&before,sizeof c))++failures;}
    {unsigned j;for(j=0;j<3u;++j){ts_input(&c,8200);if(j==0)c.service.bbank=062006;else if(j==1)c.service.bbank=064007;else c.service.interrupt_enabled=2;before=c;if(ts_apply(&c)||memcmp(&c,&before,sizeof c))++failures;}}
    printf("%u complete throttle_service states, unsupported Q/SUPER/BB/IRQ unchanged: %s\n",THROTTLE_SERVICE_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
