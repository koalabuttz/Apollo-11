/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_smcdure_cases.h"
#include "cm_smcdure.h"
#define CM_SMCDURE0_FROZEN_IMPLEMENTATION
#define CM_SMCDURE1_FROZEN_IMPLEMENTATION
#define CM_SMCDURE2_FROZEN_IMPLEMENTATION
#define CM_SMCDURE3_FROZEN_IMPLEMENTATION
#define CM_SMCDURE4_FROZEN_IMPLEMENTATION
#define CM_SMCDURE5_FROZEN_IMPLEMENTATION
#define CM_SMCDURE6_FROZEN_IMPLEMENTATION
#define CM_SMCDURE7_FROZEN_IMPLEMENTATION
#define CM_SMCDURE8_FROZEN_IMPLEMENTATION
#define CM_SMCDURE9_FROZEN_IMPLEMENTATION
#define CM_SMCDURE10_FROZEN_IMPLEMENTATION
#define CM_SMCDURE11_FROZEN_IMPLEMENTATION
#define CM_SMCDURE12_FROZEN_IMPLEMENTATION
#define CM_SMCDURE13_FROZEN_IMPLEMENTATION
#define CM_SMCDURE14_FROZEN_IMPLEMENTATION
#define CM_SMCDURE15_FROZEN_IMPLEMENTATION
#define CM_SMCDURE16_FROZEN_IMPLEMENTATION
#define CM_SMCDURE17_FROZEN_IMPLEMENTATION
#include "cm_smcdure_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==0
case 0:return cm_smcdure0_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==1
case 1:return cm_smcdure1_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==2
case 2:return cm_smcdure2_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==3
case 3:return cm_smcdure3_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==4
case 4:return cm_smcdure4_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==5
case 5:return cm_smcdure5_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==6
case 6:return cm_smcdure6_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==7
case 7:return cm_smcdure7_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==8
case 8:return cm_smcdure8_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==9
case 9:return cm_smcdure9_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==10
case 10:return cm_smcdure10_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==11
case 11:return cm_smcdure11_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==12
case 12:return cm_smcdure12_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==13
case 13:return cm_smcdure13_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==14
case 14:return cm_smcdure14_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==15
case 15:return cm_smcdure15_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==16
case 16:return cm_smcdure16_reference_begin(r,row);
#endif
#if !defined(CM_SMCDURE_SELECTED) || CM_SMCDURE_SELECTED==17
case 17:return cm_smcdure17_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_smcdure_check(void){uint8_t phase;unsigned row,fail=0;
#ifdef CM_SMCDURE_SELECTED
for(phase=CM_SMCDURE_SELECTED;phase<=CM_SMCDURE_SELECTED;++phase){
#else
for(phase=0;phase<18u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;uint8_t step;agc_word irq;enum cm_smcdure_status result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++fail;continue;}
cm_smcdure_input(&image,&irq,n);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=CM_SMCDURE_OK;for(step=0;step<=phase&&result==CM_SMCDURE_OK;++step)result=cm_smcdure(&image,(enum cm_smcdure_phase)step);
if(result!=CM_SMCDURE_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_SMCDURE_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned field,f=0;static const unsigned badword[3]={4,5,7};for(field=0;field<3u;++field){cm_smcdure_input(&image,&irq,7);image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;
if(cm_smcdure(&image,CM_SMCDURE_LOAD_FIRST)!=CM_SMCDURE_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}return f;}
int main(void){unsigned f=cm_smcdure_check()+rejected_unchanged();printf("576 complete2048-word original CM SMCDURES source operation states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
