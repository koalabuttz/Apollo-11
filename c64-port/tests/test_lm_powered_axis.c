/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_powered_axis_cases.h"
#include "lm_powered_axis.h"
#define LM_POWERED_AXIS0_FROZEN_IMPLEMENTATION
#define LM_POWERED_AXIS1_FROZEN_IMPLEMENTATION

#include "lm_powered_axis_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(LM_POWERED_AXIS_SELECTED) || LM_POWERED_AXIS_SELECTED==0
case 0:return lm_powered_axis0_reference_begin(r,row);
#endif
#if !defined(LM_POWERED_AXIS_SELECTED) || LM_POWERED_AXIS_SELECTED==1
case 1:return lm_powered_axis1_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned lm_powered_axis_check(void){uint8_t phase;unsigned row,fail=0;
#ifdef LM_POWERED_AXIS_SELECTED
for(phase=LM_POWERED_AXIS_SELECTED;phase<=LM_POWERED_AXIS_SELECTED;++phase){
#else
for(phase=0;phase<2u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t nlo,nhi,expected_irq;uint32_t n;agc_word irq;enum lm_powered_axis_status result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)){++fail;continue;}
n=(uint32_t)nlo+((uint32_t)nhi<<16);lm_powered_axis_input(&image,&irq,n);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=lm_powered_axis(&image);if(phase&&result==LM_POWERED_AXIS_OK)result=lm_powered_axis_return(&image);
if(result!=LM_POWERED_AXIS_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef LM_POWERED_AXIS_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned op,field,f=0;static const unsigned badword[3]={4,5,7};for(op=0;op<2u;++op){for(field=0;field<3u;++field){enum lm_powered_axis_status result;lm_powered_axis_input(&image,&irq,7);
if(op&&lm_powered_axis(&image)!=LM_POWERED_AXIS_OK)return 1;
image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;
result=op?lm_powered_axis_return(&image):lm_powered_axis(&image);
if(result!=LM_POWERED_AXIS_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}}return f;}
int main(void){unsigned f=lm_powered_axis_check()+rejected_unchanged();printf("64 complete2048-word original LM axis numeric/return states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
