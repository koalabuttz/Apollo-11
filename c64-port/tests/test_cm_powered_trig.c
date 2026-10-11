/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_powered_trig_cases.h"
#include "cm_powered_trig.h"
#define CM_POWERED_TRIG0_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG1_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG2_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG3_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG4_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG5_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG6_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG7_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG8_FROZEN_IMPLEMENTATION
#define CM_POWERED_TRIG9_FROZEN_IMPLEMENTATION
#include "cm_powered_trig_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==0
case 0:return cm_powered_trig0_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==1
case 1:return cm_powered_trig1_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==2
case 2:return cm_powered_trig2_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==3
case 3:return cm_powered_trig3_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==4
case 4:return cm_powered_trig4_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==5
case 5:return cm_powered_trig5_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==6
case 6:return cm_powered_trig6_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==7
case 7:return cm_powered_trig7_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==8
case 8:return cm_powered_trig8_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED==9
case 9:return cm_powered_trig9_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_powered_trig_check(void){uint8_t phase;unsigned row,fail=0;
#ifdef CM_POWERED_TRIG_SELECTED
for(phase=CM_POWERED_TRIG_SELECTED;phase<=CM_POWERED_TRIG_SELECTED;++phase){
#else
for(phase=0;phase<10u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,live,expected_irq;agc_word irq;uint8_t step;enum cm_powered_trig_status result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&live)){++fail;continue;}
cm_powered_trig_input(&image,&irq,n,(uint8_t)live);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=cm_powered_cdu_start(&image,(uint8_t)live);
for(step=1;step<=phase&&result==CM_POWERED_TRIG_OK;++step)result=step%3u==1u?cm_powered_cdu_cosine(&image):step%3u==2u?cm_powered_cdu_sine(&image):cm_powered_cdu_next(&image);
if(result!=CM_POWERED_TRIG_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_POWERED_TRIG_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned op,field,f=0;static const unsigned badword[3]={4,5,7};for(op=0;op<4u;++op)for(field=0;field<3u;++field){enum cm_powered_trig_status result;cm_powered_trig_input(&image,&irq,7,1);
if(op&&cm_powered_cdu_start(&image,1)!=CM_POWERED_TRIG_OK)return 1;
if(op>=2u&&cm_powered_cdu_cosine(&image)!=CM_POWERED_TRIG_OK)return 1;
if(op>=3u&&cm_powered_cdu_sine(&image)!=CM_POWERED_TRIG_OK)return 1;
image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;
result=op==0u?cm_powered_cdu_start(&image,1):op==1u?cm_powered_cdu_cosine(&image):op==2u?cm_powered_cdu_sine(&image):cm_powered_cdu_next(&image);
if(result!=CM_POWERED_TRIG_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}return f;}
int main(void){unsigned f=cm_powered_trig_check()+rejected_unchanged();printf("320 complete2048-word original CM CDU trig phase states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
