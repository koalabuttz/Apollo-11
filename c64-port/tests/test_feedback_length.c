/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_feedback_pairs_cases.h"
#include "guidance_length_phased.h"
#define FEEDBACK_LENGTH0_FROZEN_IMPLEMENTATION
#define FEEDBACK_LENGTH1_FROZEN_IMPLEMENTATION
#define FEEDBACK_LENGTH2_FROZEN_IMPLEMENTATION
#define FEEDBACK_LENGTH3_FROZEN_IMPLEMENTATION
#include "feedback_length_vectors.h"
static throttle_feedback_pairs_case c;
static void from_words(const uint16_t w[TFP_WORDS]){uint8_t i;dv_from_words(&c.source.previous.base,w);c.source.previous.norm.sr=w[93];c.source.previous.norm.cyl=w[94];c.source.previous.norm.addrwd=w[95];c.source.previous.norm.buf_third=w[96];c.source.previous.norm.tem1=w[97];for(i=0;i<6u;++i)c.source.previous.norm.vbuf[i]=w[98+i];c.source.input.mass.high=w[104];c.source.input.mass.low=w[105];c.source.input.lag_tau.high=w[106];c.source.input.lag_tau.low=w[107];c.source.input.maxforce.high=w[108];c.source.input.maxforce.low=w[109];c.source.input.minforce.high=w[110];c.source.input.minforce.low=w[111];c.source.feedback.afc.high=w[112];c.source.feedback.afc.low=w[113];c.source.feedback.fweight.high=w[114];c.source.feedback.fweight.low=w[115];c.source.feedback.lasttpip.high=w[116];c.source.feedback.lasttpip.low=w[117];for(i=0;i<6u;++i)c.source.bias[i]=w[118+i];c.control.pc=w[124];c.control.q=w[125];c.control.fb=w[126];c.control.eb=w[127];c.control.bb=w[128];c.control.loc=w[129];c.control.bankset=w[130];c.control.cyr=w[131];c.control.edop=w[132];c.control.intbit15=w[133];c.control.interrupt_enabled=w[134];c.control.polish=w[135];c.control.next_pair=(uint8_t)w[136];}
static unsigned apply(uint8_t step){throttle_feedback_case *x=&c.source;p66_preparation *s=&x->previous.base.previous.previous.previous.preparation;p66_scale_scratch *sc=&x->previous.base.previous.previous.previous.scratch;p66_elapsed_state *el=&x->previous.base.previous.elapsed;p66_divide_scratch *d=&x->previous.base.scratch;p66_norm_scratch *n=&x->previous.norm;p66_pushdown *p=&x->previous.base.previous.previous.push;unsigned result;switch(step){
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==0
case 0:result=(unsigned)p66_length_square_phase(s,sc,el,d,n,p);c.control.pc=03202;c.control.q=0140;c.control.fb=0;c.control.eb=03400;c.control.bb=7;c.control.loc=03437;c.control.cyr=040005;c.control.edop=0052;c.control.next_pair=255;return result;
#endif
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==1
case 1:result=(unsigned)p66_length_root_phase(s,sc,el,d,n,p);c.control.pc=03210;c.control.q=3;return result;
#endif
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==2
case 2:result=(unsigned)p66_length_finish_phase(s,sc,el,d,n,p);c.control.pc=06061;return result;
#endif
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==3
case 3:return (unsigned)p66_feedback_timestamp_after_length(s,n,p,&c.control);
#endif
default:return P66_FEEDBACK_OUT_OF_SCOPE;}}
static uint8_t begin(frozen_words *r,uint8_t step,unsigned row){switch(step){
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==0
case 0:return (uint8_t)feedback_length0_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==1
case 1:return (uint8_t)feedback_length1_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==2
case 2:return (uint8_t)feedback_length2_reference_begin(r,row);
#endif
#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED==3
case 3:return (uint8_t)feedback_length3_reference_begin(r,row);
#endif
default:return 0;}}
unsigned feedback_length_check(void){unsigned fail=0,row,i;uint8_t step;
#if defined(FEEDBACK_LENGTH_SELECTED)
for(step=FEEDBACK_LENGTH_SELECTED;step<=FEEDBACK_LENGTH_SELECTED;++step){
#else
for(step=0;step<4u;++step){
#endif
for(row=0;row<32u;++row){frozen_words reader;uint16_t w[TFP_WORDS],expected;if(!begin(&reader,step,row)){++fail;continue;}for(i=0;i<TFP_WORDS;++i)if(!frozen_words_next(&reader,&w[i]))++fail;from_words(w);{unsigned status=apply(step);if(!frozen_words_next(&reader,&expected)||status!=expected)++fail;}tfp_words(&c,w);for(i=0;i<TFP_WORDS;++i)if(!frozen_words_next(&reader,&expected)||w[i]!=expected)++fail;if(!frozen_words_finished(&reader))++fail;}}return fail;}
#ifndef FEEDBACK_LENGTH_CHECK_ONLY
int main(void){unsigned f=feedback_length_check();printf("128 complete original feedback length entries/exits: %s\n",f?"FAIL":"PASS");return f?1:0;}
#endif
