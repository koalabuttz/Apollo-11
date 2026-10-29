/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_nbsm_cases.h"
#include "cm_nbsm.h"
#include "cm_powered_interface_apply.h"
#define CM_NBSM0_FROZEN_IMPLEMENTATION
#define CM_NBSM1_FROZEN_IMPLEMENTATION
#define CM_NBSM2_FROZEN_IMPLEMENTATION
#define CM_NBSM3_FROZEN_IMPLEMENTATION
#define CM_NBSM4_FROZEN_IMPLEMENTATION
#define CM_NBSM5_FROZEN_IMPLEMENTATION
#include "cm_nbsm_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_NBSM_SELECTED) || CM_NBSM_SELECTED==0
case 0:return cm_nbsm0_reference_begin(r,row);
#endif
#if !defined(CM_NBSM_SELECTED) || CM_NBSM_SELECTED==1
case 1:return cm_nbsm1_reference_begin(r,row);
#endif
#if !defined(CM_NBSM_SELECTED) || CM_NBSM_SELECTED==2
case 2:return cm_nbsm2_reference_begin(r,row);
#endif
#if !defined(CM_NBSM_SELECTED) || CM_NBSM_SELECTED==3
case 3:return cm_nbsm3_reference_begin(r,row);
#endif
#if !defined(CM_NBSM_SELECTED) || CM_NBSM_SELECTED==4
case 4:return cm_nbsm4_reference_begin(r,row);
#endif
#if !defined(CM_NBSM_SELECTED) || CM_NBSM_SELECTED==5
case 5:return cm_nbsm5_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_nbsm_check(void){uint8_t phase;unsigned row,fail=0;
#ifdef CM_NBSM_SELECTED
for(phase=CM_NBSM_SELECTED;phase<=CM_NBSM_SELECTED;++phase){
#else
for(phase=0;phase<6u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;uint8_t step;agc_word irq;unsigned result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++fail;continue;}
cm_nbsm_input(&image,&irq,n);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=CM_NBSM_OK;for(step=0;step<=phase&&step<5u&&result==CM_NBSM_OK;++step)result=(unsigned)cm_nbsm(&image,(enum cm_nbsm_phase)step);if(phase==5u&&result==CM_NBSM_OK){if(!cm_powered_interface_apply(&image,&irq,5))result=1;else result=(unsigned)cm_nbsm(&image,CM_NBSM_STORE_RETURN);}
if(result!=0||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_NBSM_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned field,f=0;static const unsigned badword[3]={4,5,7};for(field=0;field<3u;++field){cm_nbsm_input(&image,&irq,7);image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;if(cm_nbsm(&image,CM_NBSM_SAVE_RETURN)!=1||memcmp(&image,&expected,sizeof image))++f;}return f;}
int main(void){unsigned f=cm_nbsm_check()+rejected_unchanged();printf("192 complete2048-word original CM legacy NBSM source states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
