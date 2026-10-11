/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "interpreted_sign_cases.h"
#define INTERPRETED_SIGN0_FROZEN_IMPLEMENTATION
#define INTERPRETED_SIGN1_FROZEN_IMPLEMENTATION
#define INTERPRETED_SIGN2_FROZEN_IMPLEMENTATION
#define INTERPRETED_SIGN3_FROZEN_IMPLEMENTATION
#include "interpreted_sign_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row)
{
    switch(selector){
#if !defined(INTERPRETED_SIGN_SELECTED) || INTERPRETED_SIGN_SELECTED==0
    case 0:return interpreted_sign0_reference_begin(r,row);
#endif
#if !defined(INTERPRETED_SIGN_SELECTED) || INTERPRETED_SIGN_SELECTED==1
    case 1:return interpreted_sign1_reference_begin(r,row);
#endif
#if !defined(INTERPRETED_SIGN_SELECTED) || INTERPRETED_SIGN_SELECTED==2
    case 2:return interpreted_sign2_reference_begin(r,row);
#endif
#if !defined(INTERPRETED_SIGN_SELECTED) || INTERPRETED_SIGN_SELECTED==3
    case 3:return interpreted_sign3_reference_begin(r,row);
#endif
    default:return 0;
    }
}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m)
{
    uint16_t count,i,address,value;
    if(!frozen_words_next(r,&count)||count>2048u)return 0;
    for(i=0;i<count;++i){
        if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;
        m->words[address]=value;
    }
    while(frozen_words_next(r,&value))if(value)return 0;
    return frozen_words_finished(r);
}
unsigned interpreted_sign_check(void)
{
    unsigned fail=0,row;
    uint8_t selector;
#ifdef INTERPRETED_SIGN_SELECTED
    for(selector=INTERPRETED_SIGN_SELECTED;selector<=INTERPRETED_SIGN_SELECTED;++selector){
#else
    for(selector=0;selector<4u;++selector){
#endif
        for(row=0;row<64u;++row){
            frozen_words r;uint16_t low,high,expected_irq;uint32_t n;
            agc_word irq;agc_dp operand;
            enum interpreted_sign_mission mission=(enum interpreted_sign_mission)(selector>>1);
            enum interpreted_sign_kind kind=(enum interpreted_sign_kind)(selector&1u);
            if(!begin(&r,selector,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}
            n=(uint32_t)low+((uint32_t)high<<16);
            interpreted_sign_input(&image,&irq,mission,n);expected=image;
            operand.high=image.words[0260];operand.low=image.words[0261];
            if(!frozen_words_next(&r,&expected_irq)||!patch(&r,&expected)){++fail;continue;}
            if(interpreted_sign(&image,mission,kind,&operand)||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;
        }
    }
    return fail;
}
#ifndef INTERPRETED_SIGN_CHECK_ONLY
static unsigned rejected(void)
{
    unsigned i,f=0;agc_word irq;agc_dp operand={0,0};
    static const unsigned fields[]={3,5,6,7};
    for(i=0;i<4u;++i){
        interpreted_sign_input(&image,&irq,INTERPRETED_SIGN_LM,0);image.words[fields[i]]^=1;expected=image;
        if(interpreted_sign(&image,INTERPRETED_SIGN_LM,INTERPRETED_COMP,0)!=INTERPRETED_SIGN_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;
    }
    interpreted_sign_input(&image,&irq,INTERPRETED_SIGN_LM,65536);image.words[0163]=077777;expected=image;
    if(interpreted_sign(&image,INTERPRETED_SIGN_LM,INTERPRETED_SIGN,&operand)!=INTERPRETED_SIGN_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;
    interpreted_sign_input(&image,&irq,INTERPRETED_SIGN_LM,65536);expected=image;
    if(interpreted_sign(&image,INTERPRETED_SIGN_LM,INTERPRETED_SIGN,0)!=INTERPRETED_SIGN_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;
    return f;
}
int main(void){unsigned f=interpreted_sign_check()+rejected();printf("256 complete original LM/CM COMP/SIGN states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
