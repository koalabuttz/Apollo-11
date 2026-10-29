/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_mysubs_cases.h"
#define CM_MYSUBS0_FROZEN_IMPLEMENTATION
#define CM_MYSUBS1_FROZEN_IMPLEMENTATION
#define CM_MYSUBS2_FROZEN_IMPLEMENTATION
#define CM_MYSUBS3_FROZEN_IMPLEMENTATION
#include "cm_mysubs_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){switch(selector){
#if !defined(CM_MYSUBS_SELECTED) || CM_MYSUBS_SELECTED==0
case 0:return (uint8_t)cm_mysubs0_reference_begin(r,row);
#endif
#if !defined(CM_MYSUBS_SELECTED) || CM_MYSUBS_SELECTED==1
case 1:return (uint8_t)cm_mysubs1_reference_begin(r,row);
#endif
#if !defined(CM_MYSUBS_SELECTED) || CM_MYSUBS_SELECTED==2
case 2:return (uint8_t)cm_mysubs2_reference_begin(r,row);
#endif
#if !defined(CM_MYSUBS_SELECTED) || CM_MYSUBS_SELECTED==3
case 3:return (uint8_t)cm_mysubs3_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_mysubs_check(void){uint8_t selector;unsigned row,fail=0;
#ifdef CM_MYSUBS_SELECTED
for(selector=CM_MYSUBS_SELECTED;selector<=CM_MYSUBS_SELECTED;++selector){
#else
for(selector=0;selector<4u;++selector){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t nlo,nhi,expected_irq;uint32_t n;agc_word irq;enum cm_mysubs_status result;if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)){++fail;continue;}n=(uint32_t)nlo+((uint32_t)nhi<<16);cm_mysubs_input(&image,&irq,n,(uint8_t)(selector%2u));expected=image;if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){++fail;continue;}result=(selector%2u)?cm_dpadd(&image):cm_smallmp(&image);if(result==CM_MYSUBS_OK&&selector>=2u)result=cm_mysubs_return(&image);if(result!=CM_MYSUBS_OK||irq!=expected_irq||memcmp(&image,&expected,sizeof image))++fail;}}
return fail;}
#ifndef CM_MYSUBS_CHECK_ONLY
static unsigned rejected_unchanged(void){agc_word irq;unsigned op,field,f=0;static const unsigned badword[3]={4,5,7};for(op=0;op<3u;++op)for(field=0;field<3u;++field){enum cm_mysubs_status result;cm_mysubs_input(&image,&irq,7,(uint8_t)(op==1u));if(op==2u&&cm_smallmp(&image)!=CM_MYSUBS_OK)return 1;image.words[badword[field]]=(agc_word)(field==2u?1u:0u);expected=image;result=op==0u?cm_smallmp(&image):op==1u?cm_dpadd(&image):cm_mysubs_return(&image);if(result!=CM_MYSUBS_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||irq!=1u)++f;}return f;}
int main(void){unsigned f=cm_mysubs_check()+rejected_unchanged();printf("128 complete2048-word original CM SMALLMP/DPADD numerical/full-return states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
