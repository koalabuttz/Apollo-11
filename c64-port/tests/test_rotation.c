/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "rotation_cases.h"
#define ROTATION_FROZEN_IMPLEMENTATION
#include "rotation_vectors.h"
static rotation_case c;
unsigned rotation_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<ROTATION_FROZEN_ROWS;++row){uint16_t w[ROT_WORDS],expected,path;frozen_words r;rot_input(&c,rotation_indices[row]);path=(uint16_t)rot_apply(&c);rot_words(&c,w);
        if(!rotation_reference_begin(&r,row)||!frozen_words_next(&r,&expected)||expected!=path)++failures;
        for(i=0;i<ROT_WORDS;++i)if(!frozen_words_next(&r,&expected)||expected!=w[i])++failures;
        if(!frozen_words_finished(&r))++failures;
    }return failures;
}
#ifndef ROTATION_CHECK_ONLY
int main(void){
    unsigned failures=rotation_check();uint32_t mode;rotation_case before;
    if(sizeof(rotation_case)!=262u)++failures;
    for(mode=0;mode<=AGC_MASK;++mode){if(mode==3u||mode==077774u)continue;rot_input(&c,4100);c.direction=(agc_word)mode;before=c;if(rot_apply(&c)!=0||memcmp(&c,&before,sizeof c))++failures;}
    for(mode=010000u;mode<=UINT16_MAX;++mode){rot_input(&c,8200);c.return_word=(agc_word)mode;before=c;if(rot_apply(&c)!=0||memcmp(&c,&before,sizeof c))++failures;}
    printf("%u complete rotation states, unsupported direction/return unchanged: %s\n",ROTATION_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
