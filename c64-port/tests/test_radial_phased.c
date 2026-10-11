/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "radial_phased_cases.h"
#define RADIAL_PHASE0_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE1_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE2_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE3_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE4_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE5_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE6_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE7_FROZEN_IMPLEMENTATION
#define RADIAL_PHASE8_FROZEN_IMPLEMENTATION
#include "radial_phased_vectors.h"
static radial_case c;
static uint16_t w[RA_WORDS];
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==0
case 0:return (uint8_t)radial_phase0_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==1
case 1:return (uint8_t)radial_phase1_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==2
case 2:return (uint8_t)radial_phase2_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==3
case 3:return (uint8_t)radial_phase3_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==4
case 4:return (uint8_t)radial_phase4_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==5
case 5:return (uint8_t)radial_phase5_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==6
case 6:return (uint8_t)radial_phase6_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==7
case 7:return (uint8_t)radial_phase7_reference_begin(r,row);
#endif
#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED==8
case 8:return (uint8_t)radial_phase8_reference_begin(r,row);
#endif
default:return 0;}}
unsigned radial_phased_check(void){uint8_t phase;unsigned row,i,failures=0;
#ifdef GUIDANCE_PHASE_SELECTED
for(phase=GUIDANCE_PHASE_SELECTED;phase<=GUIDANCE_PHASE_SELECTED;++phase){
#else
for(phase=0;phase<9u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t expected;unsigned status;
if(!begin(&reader,phase,row)){++failures;continue;}
for(i=0;i<RA_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++failures;
ra_from_words(&c,w);status=rp_apply(&c,phase);
if(!frozen_words_next(&reader,&expected)||status!=expected)++failures;
ra_words(&c,w);for(i=0;i<RA_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected)++failures;
if(!frozen_words_finished(&reader))++failures;}
}return failures;}
#ifndef GUIDANCE_PHASE_CHECK_ONLY
int main(void){unsigned f=radial_phased_check();printf("288 complete source radial phase entry/exit states: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
