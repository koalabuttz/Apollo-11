/* SPDX-License-Identifier: MIT; explicit compiled original CM control. */
#include "cm_interbank_extended.h"
static const agc_word pc[14]={04662,04666,04675,04701,04704,04711,04715,04740,04744,04753,04757,04773,04707,04710};
static int context(const apollo_overlay_state *s,const cm_native_io *io){return s&&io&&s->words[7]==0&&s->words[2]<=07775u&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&(io->superbank&~0160u)==0&&io->interrupt_enabled<=1u;}
static void fb(apollo_overlay_state *s,agc_word value){s->words[4]=(agc_word)(value&076000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));}
enum exec_status cm_interbank_extended_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){agc_word cadr,bank;
 if(!context(s,io)||!out||s->words[5]!=04715u)return EXEC_OUT_OF_SCOPE;
 cadr=agc_correct(s->words[0]);bank=agc_overflow(s->words[0])?(agc_word)(s->words[4]>>10):(agc_word)(cadr>>10);
 if(bank>=030u&&(s->words[1]&0100u))bank=(agc_word)(bank+8u);
 *out=(uint16_t)(bank*1024u+(cadr&01777u));return EXEC_OK;
}
enum exec_status cm_interbank_extended(apollo_overlay_state *s,cm_native_io *io,enum cm_interbank_entry entry,agc_word literal,agc_word word){agc_word oldfb,cadr,q,base,relative,savedbank;agc_register olda,oldl;int overflow;
 if(!context(s,io)||(unsigned)entry>=14u||s->words[5]!=pc[entry])return EXEC_OUT_OF_SCOPE;
 q=s->words[2];oldfb=s->words[4];olda=s->words[0];oldl=s->words[1];overflow=agc_overflow(olda);base=(agc_word)(entry>=CM_IB_IBNKCALL&&entry<=CM_IB_ISWRETURN?072:0133);cadr=literal;
 if((entry==CM_IB_BANKCALL||entry==CM_IB_IBNKCALL||entry==CM_IB_POSTJUMP||entry==CM_IB_USPRCADR)&&literal>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 if((entry==CM_IB_SWRETURN||entry==CM_IB_ISWRETURN)&&s->words[base]>07777u)return EXEC_OUT_OF_SCOPE;
 if(entry==CM_IB_SUPDACAL&&word>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 if(entry==CM_IB_QJUMP&&q>01777u)return EXEC_OUT_OF_SCOPE;
 /* Negative-overflow BANKJUMP is not a compiled CADR transfer: INDEX Q
  * changes TCF 10000 into CCS 1777 and may execute arbitrary caller words. */
 if(entry==CM_IB_BANKJUMP&&overflow<0)return EXEC_OUT_OF_SCOPE;
 /* The physical L register has no retained overflow bits: every source
  * instruction normalizes its register input before this return boundary. */
 s->words[1]=agc_extend(agc_correct(oldl));
 if(entry==CM_IB_SWCALL||entry==CM_IB_BANKJUMP||entry==CM_IB_ISWCALL)cadr=agc_correct(olda);
 if(entry==CM_IB_SWRETURN||entry==CM_IB_ISWRETURN){if(s->words[base]>07777u)return EXEC_OUT_OF_SCOPE;fb(s,s->words[base+1u]);s->words[base+1u]=oldfb;s->words[0]=agc_extend(agc_correct(olda));s->words[2]=(agc_word)(base+1u);s->words[5]=s->words[base];return EXEC_OK;}
 if(entry==CM_IB_BANKCALL||entry==CM_IB_SWCALL||entry==CM_IB_IBNKCALL||entry==CM_IB_ISWCALL){savedbank=oldfb;relative=(agc_word)(cadr&01777u);
  if(entry==CM_IB_BANKCALL||entry==CM_IB_IBNKCALL){s->words[base]=agc_correct(olda);s->words[base+1u]=agc_correct(oldl);q=agc_add_word(q,1);fb(s,cadr);}
  else if(overflow){savedbank=agc_correct(olda);relative=(agc_word)(overflow>0?1u:01776u);}
  else fb(s,cadr);
  s->words[0]=agc_extend(s->words[base]);s->words[1]=agc_extend(s->words[base+1u]);s->words[base]=q;s->words[base+1u]=savedbank;s->words[2]=entry==CM_IB_BANKCALL||entry==CM_IB_SWCALL?04675:04753;s->words[5]=(agc_word)(02000u+relative);return EXEC_OK;
 }
 if(entry==CM_IB_POSTJUMP||entry==CM_IB_BANKJUMP){fb(s,cadr);s->words[0]=entry==CM_IB_POSTJUMP?olda:agc_extend(q);
  if(entry==CM_IB_BANKJUMP&&overflow){s->words[2]=(agc_register)(overflow>0?1u:0177776u);s->words[5]=(agc_word)(agc_add_word(012000,agc_correct(s->words[2]))&07777u);}
  else{s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);}return EXEC_OK;
 }
 if(entry==CM_IB_MAKECADR){s->words[0]=agc_add_register(agc_extend((agc_word)(s->words[0133]&01777u)),agc_extend(s->words[0134]));s->words[5]=q;s->words[2]=3;return EXEC_OK;}
 if(entry==CM_IB_SUPDACAL){if(word>AGC_MASK)return EXEC_OUT_OF_SCOPE;s->words[0135]=word;s->words[1]=(agc_word)(agc_correct(olda)&01777u);s->words[0]=agc_extend(word);
  if(overflow){fb(s,overflow>0?0:076000);if(overflow<0)io->superbank=0160;}
  s->words[5]=q;s->words[2]=3;io->interrupt_enabled=1;return EXEC_OK;}
 if(entry==CM_IB_USPRCADR){s->words[0164]=q;s->words[0165]=s->words[6];s->words[023]=(agc_word)(overflow>0?0u:overflow<0?0177u:1u);fb(s,cadr);s->words[0]=agc_extend(agc_correct(olda));s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);return EXEC_OK;}
 if(entry==CM_IB_SUPERSW){io->superbank=(agc_word)(agc_correct(olda)&0160u);s->words[5]=q;s->words[2]=3;return EXEC_OK;}
 if(entry==CM_IB_QJUMP){if(q>01777u)return EXEC_OUT_OF_SCOPE;s->words[5]=(agc_word)(02000u+q);return EXEC_OK;}
 s->words[5]=02000;return EXEC_OK;
}
