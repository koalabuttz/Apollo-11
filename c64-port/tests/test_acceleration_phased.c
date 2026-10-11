/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "acceleration_phased_cases.h"
#define ACCELERATION_PHASE0_FROZEN_IMPLEMENTATION
#define ACCELERATION_PHASE1_FROZEN_IMPLEMENTATION
#define ACCELERATION_PHASE2_FROZEN_IMPLEMENTATION
#define ACCELERATION_PHASE3_FROZEN_IMPLEMENTATION
#include "acceleration_phased_vectors.h"
static acceleration_case c;
static void from_words(const uint16_t w[ACC_WORDS]){uint8_t i;dv_from_words(&c.previous.base,w);c.previous.norm.sr=w[93];c.previous.norm.cyl=w[94];c.previous.norm.addrwd=w[95];c.previous.norm.buf_third=w[96];c.previous.norm.tem1=w[97];for(i=0;i<6u;++i)c.previous.norm.vbuf[i]=w[98+i];c.taurod.high=w[104];c.taurod.low=w[105];for(i=0;i<6u;++i)c.gravity[i]=w[106+i];}
static uint8_t begin(frozen_words *r,uint8_t phase,unsigned row){switch(phase){
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==0
case 0:return (uint8_t)acceleration_phase0_reference_begin(r,row);
#endif
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==1
case 1:return (uint8_t)acceleration_phase1_reference_begin(r,row);
#endif
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==2
case 2:return (uint8_t)acceleration_phase2_reference_begin(r,row);
#endif
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==3
case 3:return (uint8_t)acceleration_phase3_reference_begin(r,row);
#endif
default:return 0;}}
unsigned acceleration_phased_check(void){uint8_t phase;unsigned row,i,failures=0;
#if defined(ACCELERATION_PHASE_SELECTED)
for(phase=ACCELERATION_PHASE_SELECTED;phase<=ACCELERATION_PHASE_SELECTED;++phase){
#else
for(phase=0;phase<4u;++phase){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t w[ACC_WORDS],expected;unsigned status;if(!begin(&reader,phase,row)){++failures;continue;}for(i=0;i<ACC_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++failures;from_words(w);status=acc_phase(&c,phase);if(!frozen_words_next(&reader,&expected)||status!=expected)++failures;acc_words(&c,w);for(i=0;i<ACC_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected)++failures;if(!frozen_words_finished(&reader))++failures;}}
return failures;}
#ifndef ACCELERATION_PHASE_CHECK_ONLY
int main(void){unsigned failures=acceleration_phased_check();uint8_t phase;uint32_t mode;acceleration_case before;for(phase=0;phase<4u;++phase){for(mode=1;mode<=AGC_MASK;++mode){acc_input(&c,4100);c.previous.base.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;if(phase>=2u)c.previous.base.previous.previous.push.pushloc=(agc_word)(c.previous.base.previous.previous.push.base+2u);before=c;if(acc_phase(&c,phase)!=P66_ACCELERATION_OUT_OF_SCOPE||memcmp(&c,&before,sizeof c))++failures;}}printf("128 original acceleration phase entry/exit states;131068 bad MODE states unchanged: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
#endif
