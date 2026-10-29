/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_feedback_pairs_cases.h"
#define FEEDBACK_PAIR0_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR1_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR2_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR3_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR4_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR5_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR6_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR7_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR8_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR9_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR10_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR11_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR12_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR13_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR14_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR15_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR16_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR17_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR18_FROZEN_IMPLEMENTATION
#define FEEDBACK_PAIR19_FROZEN_IMPLEMENTATION
#include "throttle_feedback_pairs_vectors.h"
static throttle_feedback_pairs_case c;
static void from_words(const uint16_t w[TFP_WORDS]){uint8_t i;dv_from_words(&c.source.previous.base,w);c.source.previous.norm.sr=w[93];c.source.previous.norm.cyl=w[94];c.source.previous.norm.addrwd=w[95];c.source.previous.norm.buf_third=w[96];c.source.previous.norm.tem1=w[97];for(i=0;i<6u;++i)c.source.previous.norm.vbuf[i]=w[98+i];c.source.input.mass.high=w[104];c.source.input.mass.low=w[105];c.source.input.lag_tau.high=w[106];c.source.input.lag_tau.low=w[107];c.source.input.maxforce.high=w[108];c.source.input.maxforce.low=w[109];c.source.input.minforce.high=w[110];c.source.input.minforce.low=w[111];c.source.feedback.afc.high=w[112];c.source.feedback.afc.low=w[113];c.source.feedback.fweight.high=w[114];c.source.feedback.fweight.low=w[115];c.source.feedback.lasttpip.high=w[116];c.source.feedback.lasttpip.low=w[117];for(i=0;i<6u;++i)c.source.bias[i]=w[118+i];c.control.pc=w[124];c.control.q=w[125];c.control.fb=w[126];c.control.eb=w[127];c.control.bb=w[128];c.control.loc=w[129];c.control.bankset=w[130];c.control.cyr=w[131];c.control.edop=w[132];c.control.intbit15=w[133];c.control.interrupt_enabled=w[134];c.control.polish=w[135];c.control.next_pair=(uint8_t)w[136];}
static uint8_t begin(frozen_words *r,uint8_t step,unsigned row){switch(step){
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==0
case 0:return (uint8_t)feedback_pair0_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==1
case 1:return (uint8_t)feedback_pair1_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==2
case 2:return (uint8_t)feedback_pair2_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==3
case 3:return (uint8_t)feedback_pair3_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==4
case 4:return (uint8_t)feedback_pair4_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==5
case 5:return (uint8_t)feedback_pair5_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==6
case 6:return (uint8_t)feedback_pair6_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==7
case 7:return (uint8_t)feedback_pair7_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==8
case 8:return (uint8_t)feedback_pair8_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==9
case 9:return (uint8_t)feedback_pair9_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==10
case 10:return (uint8_t)feedback_pair10_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==11
case 11:return (uint8_t)feedback_pair11_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==12
case 12:return (uint8_t)feedback_pair12_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==13
case 13:return (uint8_t)feedback_pair13_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==14
case 14:return (uint8_t)feedback_pair14_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==15
case 15:return (uint8_t)feedback_pair15_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==16
case 16:return (uint8_t)feedback_pair16_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==17
case 17:return (uint8_t)feedback_pair17_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==18
case 18:return (uint8_t)feedback_pair18_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==19
case 19:return (uint8_t)feedback_pair19_reference_begin(r,row);
#endif
default:return 0;}}
unsigned feedback_pairs_check(void){unsigned fail=0,row,i;uint8_t step;
#if defined(FEEDBACK_PAIR_SELECTED)
for(step=FEEDBACK_PAIR_SELECTED;step<=FEEDBACK_PAIR_SELECTED;++step){
#else
for(step=0;step<20u;++step){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t w[TFP_WORDS],expected;if(!begin(&reader,step,row)){++fail;continue;}for(i=0;i<TFP_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++fail;from_words(w);{unsigned status=tfp_apply(&c);if(!frozen_words_next(&reader,&expected)||status!=expected)++fail;}tfp_words(&c,w);for(i=0;i<TFP_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected)++fail;if(!frozen_words_finished(&reader))++fail;}}return fail;}
#ifndef FEEDBACK_PAIR_CHECK_ONLY
int main(void){unsigned f=feedback_pairs_check();printf("640 complete original feedback pair entries/exits: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
