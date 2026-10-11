/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_difeq_cases.h"
#define CM_DIFEQ_STAGE0_PHASE0_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE1_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE2_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE3_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE4_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE5_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE6_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE7_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE0_PHASE8_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE0_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE1_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE2_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE3_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE4_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE5_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE6_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE7_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE8_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE9_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE10_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE1_PHASE11_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE0_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE1_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE2_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE3_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE4_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE5_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE6_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE7_FROZEN_IMPLEMENTATION
#define CM_DIFEQ_STAGE2_PHASE8_FROZEN_IMPLEMENTATION
#include "cm_difeq_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){switch(selector){
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==0
case 0:return (uint8_t)cm_difeq_stage0_phase0_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==1
case 1:return (uint8_t)cm_difeq_stage0_phase1_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==2
case 2:return (uint8_t)cm_difeq_stage0_phase2_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==3
case 3:return (uint8_t)cm_difeq_stage0_phase3_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==4
case 4:return (uint8_t)cm_difeq_stage0_phase4_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==5
case 5:return (uint8_t)cm_difeq_stage0_phase5_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==6
case 6:return (uint8_t)cm_difeq_stage0_phase6_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==7
case 7:return (uint8_t)cm_difeq_stage0_phase7_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==8
case 8:return (uint8_t)cm_difeq_stage0_phase8_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==16
case 16:return (uint8_t)cm_difeq_stage1_phase0_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==17
case 17:return (uint8_t)cm_difeq_stage1_phase1_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==18
case 18:return (uint8_t)cm_difeq_stage1_phase2_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==19
case 19:return (uint8_t)cm_difeq_stage1_phase3_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==20
case 20:return (uint8_t)cm_difeq_stage1_phase4_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==21
case 21:return (uint8_t)cm_difeq_stage1_phase5_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==22
case 22:return (uint8_t)cm_difeq_stage1_phase6_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==23
case 23:return (uint8_t)cm_difeq_stage1_phase7_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==24
case 24:return (uint8_t)cm_difeq_stage1_phase8_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==25
case 25:return (uint8_t)cm_difeq_stage1_phase9_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==26
case 26:return (uint8_t)cm_difeq_stage1_phase10_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==27
case 27:return (uint8_t)cm_difeq_stage1_phase11_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==32
case 32:return (uint8_t)cm_difeq_stage2_phase0_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==33
case 33:return (uint8_t)cm_difeq_stage2_phase1_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==34
case 34:return (uint8_t)cm_difeq_stage2_phase2_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==35
case 35:return (uint8_t)cm_difeq_stage2_phase3_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==36
case 36:return (uint8_t)cm_difeq_stage2_phase4_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==37
case 37:return (uint8_t)cm_difeq_stage2_phase5_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==38
case 38:return (uint8_t)cm_difeq_stage2_phase6_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==39
case 39:return (uint8_t)cm_difeq_stage2_phase7_reference_begin(r,row);
#endif
#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR==40
case 40:return (uint8_t)cm_difeq_stage2_phase8_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_difeq_check(void){uint8_t selector;unsigned row,failures=0;
#ifdef CM_DIFEQ_SELECTOR
for(selector=CM_DIFEQ_SELECTOR;selector<=CM_DIFEQ_SELECTOR;++selector){
#else
for(selector=0;selector<41u;++selector){if(selector%16u>=cm_difeq_phase_count((uint8_t)(selector/16u)))continue;
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n;uint8_t stage=(uint8_t)(selector/16u),phase=(uint8_t)(selector%16u);if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&n)){++failures;continue;}cm_difeq_input(&image,n);if(!patch(&reader,&image)){++failures;continue;}expected=image;if(!patch(&reader,&expected)||!finish(&reader)){++failures;continue;}if(cm_difeq_phase(&image,stage,phase)!=CM_DIFEQ_OK||memcmp(&image,&expected,sizeof image))++failures;}}
return failures;}
#ifndef CM_DIFEQ_CHECK_ONLY
static unsigned rejection(void){unsigned errors=0;uint8_t stage,phase;for(stage=0;stage<3u;++stage)for(phase=0;phase<cm_difeq_phase_count(stage);++phase){cm_difeq_input(&image,(uint16_t)(stage*512u));image.words[0120]=0400;expected=image;if(cm_difeq_phase(&image,stage,phase)!=CM_DIFEQ_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image))++errors;}return errors;}
int main(void){unsigned f=cm_difeq_check()+rejection();printf("960 complete2048word original DIFEQ phase entry/exit pairs and unchanged scope rejections: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
