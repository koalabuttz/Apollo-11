/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_powered_quick_cases.h"
#include "cm_powered_quick_return.h"
#include "cm_quictrig.h"
#define CM_POWERED_QUICK0_FROZEN_IMPLEMENTATION
#define CM_POWERED_QUICK1_FROZEN_IMPLEMENTATION
#include "cm_powered_quick_vectors.h"
unsigned cm_powered_constants_check(void);
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_POWERED_QUICK_SELECTED) || CM_POWERED_QUICK_SELECTED==0
case 0:return cm_powered_quick0_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_QUICK_SELECTED) || CM_POWERED_QUICK_SELECTED==1
case 1:return cm_powered_quick1_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_powered_quick_check(void){uint8_t phase;unsigned row,fail=cm_powered_constants_check();
#ifdef CM_POWERED_QUICK_SELECTED
for(phase=CM_POWERED_QUICK_SELECTED;phase<=CM_POWERED_QUICK_SELECTED;++phase){
#else
for(phase=0;phase<2u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;uint8_t step;agc_word irq;unsigned result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++fail;continue;}
cm_powered_quick_input(&image,&irq,n);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
(void)step;result=(unsigned)cm_quictrig(&image,&irq);if(!result&&phase==1u)result=(unsigned)cm_powered_quick_return(&image);
if(result!=0||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_POWERED_QUICK_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned field,f=0;static const unsigned badword[3]={4,5,7};for(field=0;field<3u;++field){cm_powered_quick_input(&image,&irq,7);if(cm_quictrig(&image,&irq)!=CM_TRIG_OK)return 1;image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;if(cm_powered_quick_return(&image)!=1||memcmp(&image,&expected,sizeof image))++f;}return f;}
int main(void){unsigned f=cm_powered_quick_check()+rejected_unchanged();printf("64 complete2048-word original CM QUICTRIG+TC A source states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
