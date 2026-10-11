/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_powered_interface_cases.h"
#include "cm_powered_interface_apply.h"
#define CM_POWERED_INTERFACE0_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE1_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE2_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE3_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE4_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE5_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE6_FROZEN_IMPLEMENTATION
#define CM_POWERED_INTERFACE7_FROZEN_IMPLEMENTATION

#include "cm_powered_interface_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==0
case 0:return cm_powered_interface0_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==1
case 1:return cm_powered_interface1_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==2
case 2:return cm_powered_interface2_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==3
case 3:return cm_powered_interface3_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==4
case 4:return cm_powered_interface4_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==5
case 5:return cm_powered_interface5_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==6
case 6:return cm_powered_interface6_reference_begin(r,row);
#endif
#if !defined(CM_POWERED_INTERFACE_SELECTED) || CM_POWERED_INTERFACE_SELECTED==7
case 7:return cm_powered_interface7_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_powered_interface_check(void){uint8_t phase;unsigned row,fail=0;
#ifdef CM_POWERED_INTERFACE_SELECTED
for(phase=CM_POWERED_INTERFACE_SELECTED;phase<=CM_POWERED_INTERFACE_SELECTED;++phase){
#else
for(phase=0;phase<8u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;agc_word irq;uint8_t result;
if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++fail;continue;}
cm_powered_interface_input(&image,&irq,n,phase);expected=image;
if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}
result=cm_powered_interface_apply(&image,&irq,phase);
if(!result||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_POWERED_INTERFACE_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned op,field,f=0;static const unsigned badword[3]={4,5,7};for(op=0;op<8u;++op){for(field=0;field<3u;++field){cm_powered_interface_input(&image,&irq,7,(uint8_t)op);image.words[badword[field]]=(agc_word)(image.words[badword[field]]^1u);expected=image;
if(cm_powered_interface_start(&image,(uint8_t)op)!=CM_POWERED_INTERFACE_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++f;}}return f;}
int main(void){unsigned f=cm_powered_interface_check()+rejected_unchanged();printf("256 complete2048-word original CM interpreted interface return states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
