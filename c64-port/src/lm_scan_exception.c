/* SPDX-License-Identifier: MIT; original Luminary099 EJSCAN exceptions. */
#include "lm_scan_exception.h"
enum exec_status lm_scan_exception(apollo_overlay_state *s,p66_bias_io *io,
 enum lm_scan_entry entry){
 static const agc_word pc[4]={05146,05133,05155,03120};
 static const agc_word calls[7]={03122,03126,03132,03136,03142,03146,03152};
 agc_word *w,packed,buf0,buf1,buf2,highest=0,q=0,l,oldbank,base=0;
 uint8_t i,hole=0;
 if(!s||!io||(unsigned)entry>3u)return EXEC_OUT_OF_SCOPE;
 w=s->words;oldbank=w[6];l=agc_correct(w[1]);
 if(w[5]!=pc[entry]||w[7]||w[3]>03400u||(w[3]&0377u)||
    (w[4]&~076000u)||oldbank!=(agc_word)(w[4]|(w[3]>>8))||
    (io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(entry==LM_SCAN_SLEEP&&agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;
 packed=w[0167];buf0=w[0130];buf1=AGC_NEG_ZERO;buf2=w[0132];
 if(entry==LM_SCAN_DIRECT){
  if(w[4]!=02000u||io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;
  buf1=w[0131];
 }else{
  if(!packed||packed>=AGC_SIGN)return EXEC_OUT_OF_SCOPE;
  if(entry==LM_SCAN_PRIORITY){
   if(w[2]<0100u||w[2]>07777u||agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;
   packed=agc_add_word(agc_correct(w[0]),(agc_word)(packed&0777u));
   if(packed>=AGC_SIGN)return EXEC_OUT_OF_SCOPE;
   buf0=0;buf1=agc_negate(packed);
  }else if(entry==LM_SCAN_END){
   base=(agc_word)(packed&0777u);l=base;
   if(base>EXEC_FAKEPRET&&(base<0401u||base>0661u||(base-0401u)%054u))return EXEC_OUT_OF_SCOPE;
  }
 }
 if(agc_classify(buf1)==AGC_NEGATIVE)highest=agc_negate(buf1);
 /* +0 at the third candidate executes the -CCSPR data word as AD07610.
  * Its transient A is overwritten by the next CCS; BUF/Q are unchanged.
  * Other +0 candidate slots execute their actual TC CCSHOLE. */
 for(i=1;i<8u;++i){agc_word priority=w[0167u+12u*i];
  if(priority==0){if(i==3u)continue;q=(agc_word)(calls[i-1u]+1u);hole=1;break;}
  if(agc_classify(priority)==AGC_POSITIVE){
   if(agc_classify(buf1)==AGC_POSITIVE||buf1==0)return EXEC_OUT_OF_SCOPE;
   buf2=(agc_word)(priority-1u);
   if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];}
  }
 }
 if(!hole){enum agc_ccs_class result=agc_classify(buf1);
  if(result!=AGC_POSITIVE&&result!=AGC_PLUS_ZERO)return EXEC_OUT_OF_SCOPE;
  q=(agc_word)(result==AGC_POSITIVE?03156u:03157u);
 }
 if(entry==LM_SCAN_PRIORITY){
  w[063]=agc_correct(w[0]);w[0164]=w[2];w[0165]=oldbank;w[0167]=packed;
  w[3]=0400;w[4]=02000;w[6]=02001;
 }else if(entry==LM_SCAN_SLEEP){
  w[0164]=agc_correct(w[0]);w[0167]=agc_negate(packed);
  w[0165]=(agc_word)((oldbank&7u)|io->superbank);
  w[4]=02000;w[6]=(agc_word)(02000u|(oldbank&7u));
 }else if(entry==LM_SCAN_END){
  w[0167]=AGC_NEG_ZERO;
  if(base>EXEC_FAKEPRET)w[base-1u]=(agc_word)(base-1u);
  w[4]=02000;w[6]=(agc_word)(02000u|(oldbank&7u));
 }
 w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[1]=agc_extend(l);
 w[0]=hole?0:agc_extend(agc_diminished_abs(buf1));
 w[2]=q;w[5]=05705;io->interrupt_enabled=0;
 return EXEC_BAILOUT_PENDING;
}
