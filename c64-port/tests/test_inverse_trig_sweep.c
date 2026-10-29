/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "inverse_trig_cases.h"
#define INVERSE_TRIG0_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG1_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG2_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG3_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG4_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG5_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG6_FROZEN_IMPLEMENTATION
#define INVERSE_TRIG7_FROZEN_IMPLEMENTATION
#include "inverse_trig_sweep_vectors.h"
#include "inverse_trig_constants.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){switch(selector){
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==0
case 0:return inverse_trig0_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==1
case 1:return inverse_trig1_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==2
case 2:return inverse_trig2_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==3
case 3:return inverse_trig3_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==4
case 4:return inverse_trig4_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==5
case 5:return inverse_trig5_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==6
case 6:return inverse_trig6_reference_begin(r,row);
#endif
#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED==7
case 7:return inverse_trig7_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned inverse_trig_check(void){uint8_t selector;unsigned row,fail=0;
#ifdef INVERSE_TRIG_SELECTED
for(selector=INVERSE_TRIG_SELECTED;selector<=INVERSE_TRIG_SELECTED;++selector){
#else
for(selector=0;selector<8u;++selector){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq,expected_status;uint8_t phase=(uint8_t)(selector&3u);enum inverse_trig_mission mission=(enum inverse_trig_mission)(selector>>2);agc_word irq;enum inverse_trig_status result;
if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&n)){++fail;continue;}inverse_trig_input(&image,&irq,mission,n);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!frozen_words_next(&reader,&expected_status)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=inverse_trig_prepare(&image,mission,(uint8_t)(n&1u));if(phase>=1u&&result==INVERSE_TRIG_OK)result=inverse_trig_root(&image,mission);if(phase>=2u&&result==INVERSE_TRIG_OK)result=inverse_trig_polynomial(&image,mission);if(phase>=3u&&result==INVERSE_TRIG_OK)result=inverse_trig_finish(&image,mission);
if(result!=(enum inverse_trig_status)expected_status||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
{unsigned k;uint16_t value=0;for(k=0;k<36u;++k)if(!inverse_trig_constant_read((enum inverse_trig_mission)inverse_trig_constants[k][0],inverse_trig_constants[k][1],&value)||value!=inverse_trig_constants[k][2])++fail;}
return fail;}
#ifndef INVERSE_TRIG_CHECK_ONLY
static unsigned rejected(void){static const unsigned field[]={4,5,7};agc_word irq;unsigned k,f=0;for(k=0;k<3u;++k){inverse_trig_input(&image,&irq,INVERSE_TRIG_LM,4);image.words[field[k]]^=1;expected=image;if(inverse_trig_prepare(&image,INVERSE_TRIG_LM,0)!=INVERSE_TRIG_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}inverse_trig_input(&image,&irq,INVERSE_TRIG_LM,4);if(inverse_trig_prepare(&image,INVERSE_TRIG_LM,0))++f;image.words[0136]=0;expected=image;if(inverse_trig_root(&image,INVERSE_TRIG_LM)!=INVERSE_TRIG_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;return f;}
int main(void){unsigned f=inverse_trig_check()+rejected();printf("256 complete original LM/CM inverse-trig states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
