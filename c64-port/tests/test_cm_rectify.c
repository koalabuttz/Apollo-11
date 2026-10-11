/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_rectify_cases.h"
#define CM_RECTIFY_PHASE0_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE1_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE2_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE3_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE4_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE5_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE6_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE7_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE8_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE9_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE10_FROZEN_IMPLEMENTATION
#define CM_RECTIFY_PHASE11_FROZEN_IMPLEMENTATION
#include "cm_rectify_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==0
case 0:return (uint8_t)cm_rectify_phase0_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==1
case 1:return (uint8_t)cm_rectify_phase1_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==2
case 2:return (uint8_t)cm_rectify_phase2_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==3
case 3:return (uint8_t)cm_rectify_phase3_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==4
case 4:return (uint8_t)cm_rectify_phase4_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==5
case 5:return (uint8_t)cm_rectify_phase5_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==6
case 6:return (uint8_t)cm_rectify_phase6_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==7
case 7:return (uint8_t)cm_rectify_phase7_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==8
case 8:return (uint8_t)cm_rectify_phase8_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==9
case 9:return (uint8_t)cm_rectify_phase9_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==10
case 10:return (uint8_t)cm_rectify_phase10_reference_begin(r,row);
#endif
#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED==11
case 11:return (uint8_t)cm_rectify_phase11_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_rectify_check(void){uint8_t phase;unsigned row,failures=0;
#ifdef CM_RECTIFY_PHASE_SELECTED
for(phase=CM_RECTIFY_PHASE_SELECTED;phase<=CM_RECTIFY_PHASE_SELECTED;++phase){
#else
for(phase=0;phase<12u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n;if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++failures;continue;}cm_rectify_input(&image,n);if(!patch(&reader,&image)){++failures;continue;}expected=image;if(!patch(&reader,&expected)||!finish(&reader)){++failures;continue;}if(cm_rectify_phase(&image,phase)!=CM_RECTIFY_OK||memcmp(&image,&expected,sizeof image))++failures;}}
return failures;}
#ifndef CM_RECTIFY_CHECK_ONLY
int main(void){unsigned f=cm_rectify_check();printf("384 complete2048word source RECTIFY phase entry/exit pairs: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
