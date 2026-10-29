/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "throttle_feedback_phased_cases.h"
#define FEEDBACK_PHASE0_FROZEN_IMPLEMENTATION
#define FEEDBACK_PHASE1_FROZEN_IMPLEMENTATION
#define FEEDBACK_PHASE2_FROZEN_IMPLEMENTATION
#define FEEDBACK_PHASE3_FROZEN_IMPLEMENTATION
#define FEEDBACK_PHASE4_FROZEN_IMPLEMENTATION
#define FEEDBACK_PHASE5_FROZEN_IMPLEMENTATION
#include "throttle_feedback_phased_vectors.h"
static throttle_feedback_case c;
#ifdef FEEDBACK_PHASE_DEBUG
uint16_t feedback_mismatches[24],feedback_trace[8];uint8_t feedback_trace_active;static unsigned mismatch_count;
static void mismatch(unsigned row,unsigned i,uint16_t actual,uint16_t expected){if(mismatch_count<6u){unsigned k=mismatch_count++*4u;feedback_mismatches[k]=(uint16_t)row;feedback_mismatches[k+1u]=(uint16_t)i;feedback_mismatches[k+2u]=actual;feedback_mismatches[k+3u]=expected;}}
#else
#define mismatch(row,i,actual,expected) ((void)0)
#endif
static void from_words(const uint16_t w[TF_WORDS]){uint8_t i;dv_from_words(&c.previous.base,w);c.previous.norm.sr=w[93];c.previous.norm.cyl=w[94];c.previous.norm.addrwd=w[95];c.previous.norm.buf_third=w[96];c.previous.norm.tem1=w[97];for(i=0;i<6u;++i)c.previous.norm.vbuf[i]=w[98+i];c.input.mass.high=w[104];c.input.mass.low=w[105];c.input.lag_tau.high=w[106];c.input.lag_tau.low=w[107];c.input.maxforce.high=w[108];c.input.maxforce.low=w[109];c.input.minforce.high=w[110];c.input.minforce.low=w[111];c.feedback.afc.high=w[112];c.feedback.afc.low=w[113];c.feedback.fweight.high=w[114];c.feedback.fweight.low=w[115];c.feedback.lasttpip.high=w[116];c.feedback.lasttpip.low=w[117];for(i=0;i<6u;++i)c.bias[i]=w[118+i];}
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==0
case 0:return (uint8_t)feedback_phase0_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==1
case 1:return (uint8_t)feedback_phase1_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==2
case 2:return (uint8_t)feedback_phase2_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==3
case 3:return (uint8_t)feedback_phase3_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==4
case 4:return (uint8_t)feedback_phase4_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==5
case 5:return (uint8_t)feedback_phase5_reference_begin(r,row);
#endif
default:return 0;}}
unsigned throttle_feedback_phased_check(void){uint8_t phase;unsigned row,i,failures=0;
#if defined(FEEDBACK_PHASE_SELECTED)
for(phase=FEEDBACK_PHASE_SELECTED;phase<=FEEDBACK_PHASE_SELECTED;++phase){
#else
for(phase=0;phase<6u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t w[TF_WORDS],expected;unsigned status;if(!begin(&reader,phase,row)){++failures;continue;}for(i=0;i<TF_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++failures;from_words(w);
#ifdef FEEDBACK_PHASE_DEBUG
feedback_trace_active=(uint8_t)(row==8u);
#endif
status=tf_phase(&c,phase);if(!frozen_words_next(&reader,&expected)||status!=expected){++failures;mismatch(row,999,(uint16_t)status,expected);}tf_words(&c,w);for(i=0;i<TF_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected){++failures;mismatch(row,i,w[i],expected);}if(!frozen_words_finished(&reader))++failures;}}
return failures;}
#ifndef FEEDBACK_PHASE_CHECK_ONLY
int main(void){unsigned failures=throttle_feedback_phased_check();uint8_t phase;uint32_t mode;throttle_feedback_case before;for(phase=0;phase<6u;++phase){for(mode=0;mode<=AGC_MASK;++mode){agc_word wanted=phase<2u?077776u:0;static const uint8_t offsets[6]={0,0,0,0,2,6};if(mode==wanted)continue;tf_input(&c,4100);c.previous.base.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;c.previous.base.previous.previous.push.pushloc=(agc_word)(c.previous.base.previous.previous.push.base+offsets[phase]);before=c;if(tf_phase(&c,phase)!=P66_FEEDBACK_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}}printf("192 original feedback phase entry/exit states;196602 bad MODE states unchanged: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
#endif
