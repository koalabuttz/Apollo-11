/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_single_return_cases.h"
#define CM_SINGLE_RETURN0_FROZEN_IMPLEMENTATION
#define CM_SINGLE_RETURN1_FROZEN_IMPLEMENTATION
#include "cm_single_return_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){switch(selector){
#if !defined(CM_SINGLE_RETURN_SELECTED) || CM_SINGLE_RETURN_SELECTED==0
case 0:return (uint8_t)cm_single_return0_reference_begin(r,row);
#endif
#if !defined(CM_SINGLE_RETURN_SELECTED) || CM_SINGLE_RETURN_SELECTED==1
case 1:return (uint8_t)cm_single_return1_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_single_return_check(void){uint8_t selector;unsigned row,fail=0;
#ifdef CM_SINGLE_RETURN_SELECTED
for(selector=CM_SINGLE_RETURN_SELECTED;selector<=CM_SINGLE_RETURN_SELECTED;++selector){
#else
for(selector=0;selector<2u;++selector){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;agc_word irq;enum cm_trig_status result;if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&n)){++fail;continue;}cm_single_return_input(&image,&irq,n);expected=image;if(!patch(&reader,&expected)){++fail;continue;}
#ifndef CM_SINGLE_RETURN_COMPLETE
image=expected;
#endif
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
#ifdef CM_SINGLE_RETURN_COMPLETE
result=cm_single_trig(&image,selector);if(result!=CM_TRIG_OK){++fail;continue;}
#endif
result=cm_single_return(&image);if(result!=CM_TRIG_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_SINGLE_RETURN_CHECK_ONLY
int main(void){unsigned f=cm_single_return_check();printf("64 complete2048word source SPSIN/SPCOS TCQ return states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
