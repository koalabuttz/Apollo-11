/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_fleshpot_cases.h"
#include "lm_fleshpot.h"
#define LM_FLESHPOT0_FROZEN_IMPLEMENTATION
#define LM_FLESHPOT1_FROZEN_IMPLEMENTATION
#define LM_FLESHPOT2_FROZEN_IMPLEMENTATION
#define LM_FLESHPOT3_FROZEN_IMPLEMENTATION
#include "lm_fleshpot_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(LM_FLESHPOT_SELECTED) || LM_FLESHPOT_SELECTED==0
case 0:return lm_fleshpot0_reference_begin(r,row);
#endif
#if !defined(LM_FLESHPOT_SELECTED) || LM_FLESHPOT_SELECTED==1
case 1:return lm_fleshpot1_reference_begin(r,row);
#endif
#if !defined(LM_FLESHPOT_SELECTED) || LM_FLESHPOT_SELECTED==2
case 2:return lm_fleshpot2_reference_begin(r,row);
#endif
#if !defined(LM_FLESHPOT_SELECTED) || LM_FLESHPOT_SELECTED==3
case 3:return lm_fleshpot3_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned lm_fleshpot_check(void){uint8_t phase;unsigned row,fail=0;
#ifdef LM_FLESHPOT_SELECTED
for(phase=LM_FLESHPOT_SELECTED;phase<=LM_FLESHPOT_SELECTED;++phase){
#else
for(phase=0;phase<4u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;uint8_t step;agc_word irq;unsigned result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++fail;continue;}
lm_fleshpot_input(&image,&irq,n);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=LM_FLESHPOT_OK;for(step=0;step<=phase&&!result;++step)result=(unsigned)lm_fleshpot(&image,(enum lm_fleshpot_phase)step);
if(result!=0||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef LM_FLESHPOT_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned field,f=0;static const unsigned badword[3]={4,5,7};for(field=0;field<3u;++field){lm_fleshpot_input(&image,&irq,7);image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;
if(lm_fleshpot(&image,LM_FLESHPOT_FIRST_ROW)!=1||memcmp(&image,&expected,sizeof image))++f;}return f;}
int main(void){unsigned f=lm_fleshpot_check()+rejected_unchanged();printf("128 complete2048-word original LM FLESHPOT source states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
