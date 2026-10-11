/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "horizontal_phased_cases.h"
#define HORIZONTAL_PHASE0_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE1_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE2_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE3_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE4_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE5_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE6_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE7_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE8_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE9_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE10_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE11_FROZEN_IMPLEMENTATION
#define HORIZONTAL_PHASE12_FROZEN_IMPLEMENTATION
#include "horizontal_phased_vectors.h"
static horizontal_phased_case c;
static uint8_t begin(frozen_words *r,uint8_t step,unsigned row){switch(step){
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==0
case 0:return (uint8_t)horizontal_phase0_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==1
case 1:return (uint8_t)horizontal_phase1_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==2
case 2:return (uint8_t)horizontal_phase2_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==3
case 3:return (uint8_t)horizontal_phase3_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==4
case 4:return (uint8_t)horizontal_phase4_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==5
case 5:return (uint8_t)horizontal_phase5_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==6
case 6:return (uint8_t)horizontal_phase6_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==7
case 7:return (uint8_t)horizontal_phase7_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==8
case 8:return (uint8_t)horizontal_phase8_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==9
case 9:return (uint8_t)horizontal_phase9_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==10
case 10:return (uint8_t)horizontal_phase10_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==11
case 11:return (uint8_t)horizontal_phase11_reference_begin(r,row);
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==12
case 12:return (uint8_t)horizontal_phase12_reference_begin(r,row);
#endif
default:return 0;}}
unsigned horizontal_phased_check(void){unsigned fail=0,row,i;uint8_t step;
#if defined(HORIZONTAL_PHASE_SELECTED)
for(step=HORIZONTAL_PHASE_SELECTED;step<=HORIZONTAL_PHASE_SELECTED;++step){
#else
for(step=0;step<13u;++step){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t w[HHP_WORDS],expected,operation,prior_status;uint8_t stage;unsigned result;if(!begin(&reader,step,row)){++fail;continue;}if(!frozen_words_next(&reader,&operation)||!frozen_words_next(&reader,&prior_status))++fail;for(i=0;i<HHP_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++fail;hhp_from_words(&c,w);c.next=(uint8_t)operation;c.unit_status=prior_status;result=hhp_step(&c,&stage);if(!frozen_words_next(&reader,&expected)||stage!=expected)++fail;if(!frozen_words_next(&reader,&expected)||result!=expected)++fail;hhp_words(&c,w);for(i=0;i<HHP_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected)++fail;if(!frozen_words_finished(&reader))++fail;}}return fail;}
#ifndef HORIZONTAL_PHASE_CHECK_ONLY
int main(void){unsigned f=horizontal_phased_check();printf("416 complete original horizontal phase entries/exits: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
