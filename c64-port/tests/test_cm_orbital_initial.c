/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_orbital_initial_cases.h"
#define CM_ORBITAL_INITIAL_PHASE0_FROZEN_IMPLEMENTATION
#define CM_ORBITAL_INITIAL_PHASE1_FROZEN_IMPLEMENTATION
#define CM_ORBITAL_INITIAL_PHASE2_FROZEN_IMPLEMENTATION
#define CM_ORBITAL_INITIAL_PHASE3_FROZEN_IMPLEMENTATION
#include "cm_orbital_initial_vectors.h"
typedef struct {apollo_overlay_state image;agc_word irq;} complete;
static complete image,expected;
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(CM_ORBITAL_INITIAL_PHASE_SELECTED) || CM_ORBITAL_INITIAL_PHASE_SELECTED==0
case 0:return (uint8_t)cm_orbital_initial_phase0_reference_begin(r,row);
#endif
#if !defined(CM_ORBITAL_INITIAL_PHASE_SELECTED) || CM_ORBITAL_INITIAL_PHASE_SELECTED==1
case 1:return (uint8_t)cm_orbital_initial_phase1_reference_begin(r,row);
#endif
#if !defined(CM_ORBITAL_INITIAL_PHASE_SELECTED) || CM_ORBITAL_INITIAL_PHASE_SELECTED==2
case 2:return (uint8_t)cm_orbital_initial_phase2_reference_begin(r,row);
#endif
#if !defined(CM_ORBITAL_INITIAL_PHASE_SELECTED) || CM_ORBITAL_INITIAL_PHASE_SELECTED==3
case 3:return (uint8_t)cm_orbital_initial_phase3_reference_begin(r,row);
#endif
default:return 0;}}
static uint8_t patch(frozen_words *r,complete *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2049u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2049u)return 0;if(address==2048u)m->irq=value;else m->image.words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
unsigned cm_orbital_initial_check(void){uint8_t phase;unsigned row,failures=0;
#ifdef CM_ORBITAL_INITIAL_PHASE_SELECTED
for(phase=CM_ORBITAL_INITIAL_PHASE_SELECTED;phase<=CM_ORBITAL_INITIAL_PHASE_SELECTED;++phase){
#else
for(phase=0;phase<4u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t n;if(!begin(&reader,phase,row)||!frozen_words_next(&reader,&n)){++failures;continue;}cm_orbital_initial_input(&image.image,&image.irq,n);if(!patch(&reader,&image)){++failures;continue;}expected=image;if(!patch(&reader,&expected)||!finish(&reader)){++failures;continue;}if(cm_orbital_initial_phase(&image.image,&image.irq,phase)!=CM_ORBITAL_INITIAL_OK||memcmp(&image,&expected,sizeof image))++failures;}}
return failures;}
#ifndef CM_ORBITAL_INITIAL_CHECK_ONLY
int main(void){unsigned f=cm_orbital_initial_check();printf("128 complete2048word+IRQ source DIFEQ0 phase entry/exit pairs: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
