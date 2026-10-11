/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_rotation_cases.h"
#define CM_ROTATION_FROZEN_IMPLEMENTATION
#include "cm_rotation_vectors.h"
static rotation_case c;
unsigned cm_rotation_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<CM_ROTATION_FROZEN_ROWS;++row){uint16_t w[ROT_WORDS],expected,path;frozen_words r;rot_input(&c,cm_rotation_indices[row]);path=(uint16_t)cm_rot_apply(&c);rot_words(&c,w);
        if(!cm_rotation_reference_begin(&r,row)||!frozen_words_next(&r,&expected)||expected!=path)++failures;
        for(i=0;i<ROT_WORDS;++i)if(!frozen_words_next(&r,&expected)||expected!=w[i])++failures;
        if(!frozen_words_finished(&r))++failures;
    }return failures;
}
#ifndef CM_ROTATION_CHECK_ONLY
int main(void){
    unsigned failures=cm_rotation_check();uint32_t mode;rotation_case before;
    if(sizeof(rotation_case)!=262u)++failures;
    for(mode=0;mode<=AGC_MASK;++mode){if(mode==3u||mode==077774u)continue;rot_input(&c,4100);c.direction=(agc_word)mode;before=c;if(cm_rot_apply(&c)!=0||memcmp(&c,&before,sizeof c))++failures;}
    for(mode=010000u;mode<=UINT16_MAX;++mode){rot_input(&c,8200);c.return_word=(agc_word)mode;before=c;if(cm_rot_apply(&c)!=0||memcmp(&c,&before,sizeof c))++failures;}
    printf("%u complete rotation states, unsupported direction/return unchanged: %s\n",CM_ROTATION_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
