/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_single_trig_cases.h"
#define LM_SINGLE_TRIG0_FROZEN_IMPLEMENTATION
#define LM_SINGLE_TRIG1_FROZEN_IMPLEMENTATION
#define LM_SINGLE_TRIG2_FROZEN_IMPLEMENTATION
#define LM_SINGLE_TRIG3_FROZEN_IMPLEMENTATION
#include "lm_single_trig_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){switch(selector){
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==0
case 0:return (uint8_t)lm_single_trig0_reference_begin(r,row);
#endif
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==1
case 1:return (uint8_t)lm_single_trig1_reference_begin(r,row);
#endif
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==2
case 2:return (uint8_t)lm_single_trig2_reference_begin(r,row);
#endif
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==3
case 3:return (uint8_t)lm_single_trig3_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned lm_single_trig_check(void){uint8_t selector;unsigned row,fail=0;
#ifdef LM_SINGLE_TRIG_SELECTED
for(selector=LM_SINGLE_TRIG_SELECTED;selector<=LM_SINGLE_TRIG_SELECTED;++selector){
#else
for(selector=0;selector<4u;++selector){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;agc_word irq;enum lm_single_trig_status result;if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&n)){++fail;continue;}lm_single_trig_input(&image,&irq,n,(uint8_t)(selector%2u));expected=image;if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}result=lm_single_trig(&image,(uint8_t)(selector%2u));if(result==LM_SINGLE_TRIG_OK&&selector>=2u)result=lm_single_trig_return(&image);if(result!=LM_SINGLE_TRIG_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef LM_SINGLE_TRIG_CHECK_ONLY
int main(void){unsigned f=lm_single_trig_check();printf("128 complete2048-word original LM SPSIN/SPCOS numerical/full-return states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
