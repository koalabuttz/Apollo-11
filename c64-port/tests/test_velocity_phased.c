/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "velocity_phased_cases.h"
#define VELOCITY_PHASE0_FROZEN_IMPLEMENTATION
#define VELOCITY_PHASE1_FROZEN_IMPLEMENTATION
#define VELOCITY_PHASE2_FROZEN_IMPLEMENTATION
#include "velocity_phased_vectors.h"
static velocity_phased_case c;
static uint16_t w[VLP_WORDS];
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==0
case 0:return (uint8_t)velocity_phase0_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==1
case 1:return (uint8_t)velocity_phase1_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==2
case 2:return (uint8_t)velocity_phase2_reference_begin(r,row);
#endif
default:return 0;}}
unsigned velocity_phased_check(void){uint8_t phase;unsigned row,i,failures=0;
#ifdef GUIDANCE_PHASE_SELECTED
for(phase=GUIDANCE_PHASE_SELECTED;phase<=GUIDANCE_PHASE_SELECTED;++phase){
#else
for(phase=0;phase<3u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t expected;unsigned status;
if(!begin(&reader,phase,row)){++failures;continue;}
for(i=0;i<VLP_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++failures;
vlp_from_words(&c,w);status=vlp_apply(&c,phase);
if(!frozen_words_next(&reader,&expected)||status!=expected)++failures;
vlp_words(&c,w);for(i=0;i<VLP_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected)++failures;
if(!frozen_words_finished(&reader))++failures;}
}return failures;}
#ifndef GUIDANCE_PHASE_CHECK_ONLY
int main(void){unsigned f=velocity_phased_check();printf("96 complete source velocity phase entry/exit states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
