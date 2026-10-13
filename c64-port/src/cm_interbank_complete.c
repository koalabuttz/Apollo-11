/* SPDX-License-Identifier: MIT; compiled original public-domain source controls. */
#include "cm_interbank_complete.h"
static const agc_word pc[14]={04662,04666,04675,04701,04704,04711,04715,04740,04744,04753,04757,04773,04707,04710};
bool cm_interbank_complete_literal(uint16_t physical,agc_word *out){if(physical!=04710u||!out)return false;*out=CM_INTERBANK_COMPLETE_PRIO12_WORD;return true;}
/* q: the entry reads Q. SWRETURN and ISWRETRN do not, and a callee's TC Q
 * reaches them with Q = 3. */
static bool context(const apollo_overlay_state *s,const cm_native_io *io,bool q){return s&&io&&s->words[7]==0&&(!q||(s->words[2]>=0100u&&s->words[2]<=07775u))&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&(io->superbank&~0160u)==0&&io->interrupt_enabled<=1u;}
static void fb(apollo_overlay_state *s,agc_word value){s->words[4]=(agc_word)(value&076000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));}
static agc_word inline_word(const apollo_overlay_state *s,agc_word logical,agc_word fixed){if(logical<01400u)return s->words[logical];if(logical<02000u)return s->words[s->words[3]+(logical&0377u)];return fixed;}
bool cm_interbank_complete_data_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){agc_word cadr,bank;
 if(!context(s,io,true)||!out||s->words[5]!=04715u)return false;
 cadr=agc_correct(s->words[0]);bank=agc_overflow(s->words[0])?(agc_word)(s->words[4]>>10):(agc_word)(cadr>>10);if(bank>=030u&&(s->words[1]&0100u))bank=(agc_word)(bank+8u);
 *out=(uint16_t)(bank*1024u+(cadr&01777u));return true;
}
bool cm_interbank_complete(apollo_overlay_state *s,cm_native_io *io,enum cm_interbank_complete_entry entry,agc_word literal,agc_word word){agc_word oldfb,cadr,q,base,relative,savedbank;agc_register olda,oldl;int overflow;
 if(!context(s,io,entry!=CM_IC_SWRETURN&&entry!=CM_IC_ISWRETRN)||(unsigned)entry>=14u||s->words[5]!=pc[entry])return false;
 q=s->words[2];oldfb=s->words[4];olda=s->words[0];oldl=s->words[1];overflow=agc_overflow(olda);base=(agc_word)(entry>=CM_IC_IBNKCALL&&entry<=CM_IC_ISWRETRN?072:0133);cadr=literal;
 if((entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL||entry==CM_IC_POSTJUMP||entry==CM_IC_USPRCADR)&&literal>AGC_MASK)return false;
 if((entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL||entry==CM_IC_POSTJUMP||entry==CM_IC_USPRCADR)&&q>=01400u&&q<02000u&&s->words[3]+(q&0377u)<0100u)return false;
 if((entry==CM_IC_SWRETURN||entry==CM_IC_ISWRETRN)&&(s->words[base]<0100u||s->words[base]>07777u))return false;
 if(entry==CM_IC_SUPDACAL&&word>AGC_MASK)return false;
 if(entry==CM_IC_QJUMP&&q>01777u)return false;
 s->words[1]=agc_extend(agc_correct(oldl));
 if(entry==CM_IC_SWCALL||entry==CM_IC_BANKJUMP||entry==CM_IC_ISWCALL)cadr=agc_correct(olda);
 if(entry==CM_IC_SWRETURN||entry==CM_IC_ISWRETRN){fb(s,s->words[base+1u]);s->words[base+1u]=oldfb;s->words[0]=agc_extend(agc_correct(olda));s->words[2]=(agc_word)(base+1u);s->words[5]=s->words[base];return true;}
 if(entry==CM_IC_BANKCALL||entry==CM_IC_SWCALL||entry==CM_IC_IBNKCALL||entry==CM_IC_ISWCALL){savedbank=oldfb;
  if(entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL){s->words[base]=agc_correct(olda);s->words[base+1u]=agc_correct(oldl);cadr=inline_word(s,q,literal);q=agc_add_word(q,1);fb(s,cadr);}
  else if(overflow){savedbank=agc_correct(olda);cadr=(agc_word)(overflow>0?1u:01776u);}
  else fb(s,cadr);
  relative=(agc_word)(cadr&01777u);s->words[0]=agc_extend(s->words[base]);s->words[1]=agc_extend(s->words[base+1u]);s->words[base]=q;s->words[base+1u]=savedbank;s->words[2]=entry==CM_IC_BANKCALL||entry==CM_IC_SWCALL?04675:04753;s->words[5]=(agc_word)(02000u+relative);return true;
 }
 if(entry==CM_IC_POSTJUMP||entry==CM_IC_BANKJUMP){if(entry==CM_IC_POSTJUMP){s->words[2]=olda;cadr=inline_word(s,q,literal);}fb(s,cadr);s->words[0]=entry==CM_IC_POSTJUMP?olda:agc_extend(q);
  if(entry==CM_IC_BANKJUMP&&overflow<0){agc_word operand=s->words[s->words[3]+0377u];enum agc_ccs_class cl=agc_classify(operand);
   /* The original INDEX Q changes this one TCF into CCS1777. Its four
    * fixed successors enter MAKECADR at CAF, MASK, AD, or TC respectively. */
   s->words[0]=agc_diminished_abs(operand);
   if(cl==AGC_POSITIVE)s->words[0]=agc_extend((agc_word)(s->words[0133]&01777u));
   else if(cl==AGC_PLUS_ZERO)s->words[0]=0;
   if(cl!=AGC_MINUS_ZERO)s->words[0]=agc_add_register(s->words[0],agc_extend(s->words[0134]));
   /* TC Q reaches physical register Q. Here Q is the source's -1 rather
    * than an ordinary TC caller descriptor, so retain that explicit external
    * continuation before any fetched register word executes. */
   s->words[2]=0177776;s->words[5]=2;return true;
  }
  if(entry==CM_IC_BANKJUMP&&overflow){s->words[2]=1;s->words[5]=02001;}
  else{s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);}return true;
 }
 if(entry==CM_IC_MAKECADR){s->words[0]=agc_add_register(agc_extend((agc_word)(s->words[0133]&01777u)),agc_extend(s->words[0134]));s->words[5]=q;s->words[2]=3;return true;}
 if(entry==CM_IC_SUPDACAL){s->words[0135]=word;s->words[1]=(agc_word)(agc_correct(olda)&01777u);s->words[0]=agc_extend(word);
  if(overflow){fb(s,overflow>0?0:076000);if(overflow<0)io->superbank=0160;}
  s->words[5]=q;s->words[2]=3;io->interrupt_enabled=1;return true;}
 if(entry==CM_IC_USPRCADR){s->words[0164]=agc_correct(olda);s->words[0165]=s->words[6];s->words[023]=(agc_word)(overflow>0?0u:overflow<0?0177u:1u);cadr=inline_word(s,q,literal);fb(s,cadr);s->words[0164]=q;s->words[0]=agc_extend(agc_correct(olda));s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);return true;}
 if(entry==CM_IC_SUPERSW){io->superbank=(agc_word)(agc_correct(olda)&0160u);s->words[5]=q;s->words[2]=3;return true;}
 if(entry==CM_IC_QJUMP){s->words[5]=(agc_word)(02000u+q);return true;}
 s->words[5]=02000;return true;
}
