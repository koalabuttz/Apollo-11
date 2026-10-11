/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_trig_cases.h"
#define CM_TRIG0_FROZEN_IMPLEMENTATION
#define CM_TRIG1_FROZEN_IMPLEMENTATION
#define CM_TRIG2_FROZEN_IMPLEMENTATION
#define CM_TRIG3_FROZEN_IMPLEMENTATION
#include "cm_trig_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){switch(selector){
#if !defined(CM_TRIG_SELECTED) || CM_TRIG_SELECTED==0
case 0:return (uint8_t)cm_trig0_reference_begin(r,row);
#endif
#if !defined(CM_TRIG_SELECTED) || CM_TRIG_SELECTED==1
case 1:return (uint8_t)cm_trig1_reference_begin(r,row);
#endif
#if !defined(CM_TRIG_SELECTED) || CM_TRIG_SELECTED==2
case 2:return (uint8_t)cm_trig2_reference_begin(r,row);
#endif
#if !defined(CM_TRIG_SELECTED) || CM_TRIG_SELECTED==3
case 3:return (uint8_t)cm_trig3_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_trig_check(void){uint8_t selector;unsigned row,fail=0;
#ifdef CM_TRIG_SELECTED
for(selector=CM_TRIG_SELECTED;selector<=CM_TRIG_SELECTED;++selector){
#else
for(selector=0;selector<4u;++selector){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;agc_word irq;enum cm_trig_status result;if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&n)){++fail;continue;}cm_trig_input(&image,&irq,n,selector<3u?selector:2u);if(!patch(&reader,&image)){++fail;continue;}expected=image;if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}result=selector<2u?cm_single_trig(&image,selector):cm_quictrig(&image,&irq);if(result!=CM_TRIG_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_TRIG_CHECK_ONLY
int main(void){unsigned f=cm_trig_check();agc_word irq;cm_trig_input(&image,&irq,7,2);expected=image;image.words[5]=03554;expected=image;if(cm_quictrig(&image,&irq)!=CM_TRIG_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||irq!=1u)++f;printf("128 complete2048word original SPSIN/SPCOS/QUICTRIG states including CLOCKJOB caller: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
