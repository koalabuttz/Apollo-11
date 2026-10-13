/* SPDX-License-Identifier: MIT; compiled original public-domain source controls. */
#include "lm_interbank.h"
static const agc_word pc[14]={04616,04622,04631,04635,04640,04645,04651,04674,04700,04707,04713,04727,04643,04644};
bool lm_interbank_literal(uint16_t physical,agc_word *out){if(physical!=04644u||!out)return false;*out=LM_INTERBANK_PRIO12_WORD;return true;}
/* q: the entry reads or returns through Q. SWRETURN and ISWRETRN do not
 * (a callee's TC Q reaches them with Q = 3), and BANKJUMP only XCHes it into
 * A as debris (DISPLAY_INTERFACE's SAVELOCS reaches it by TC L, Q = 2). */
static bool context(const apollo_overlay_state *s,const p66_bias_io *io,bool q){return s&&io&&s->words[7]==0&&(!q||(s->words[2]>=0100u&&s->words[2]<=07775u))&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&(io->superbank&~0160u)==0&&io->interrupt_enabled<=1u;}
static void fb(apollo_overlay_state *s,agc_word value){s->words[4]=(agc_word)(value&076000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));}
static agc_word inline_word(const apollo_overlay_state *s,agc_word logical,agc_word fixed){if(logical<01400u)return s->words[logical];if(logical<02000u)return s->words[s->words[3]+(logical&0377u)];return fixed;}
bool lm_interbank_data_address(const apollo_overlay_state *s,const p66_bias_io *io,uint16_t *out){agc_word cadr,bank;
 if(!context(s,io,true)||!out||s->words[5]!=04651u)return false;
 cadr=agc_correct(s->words[0]);bank=agc_overflow(s->words[0])?(agc_word)(s->words[4]>>10):(agc_word)(cadr>>10);if(bank>=030u&&(s->words[1]&0100u))bank=(agc_word)(bank+8u);
 *out=(uint16_t)(bank*1024u+(cadr&01777u));return true;
}
/* The entry's checks common to every routine, and the registers it reads. */
typedef struct { agc_word oldfb,q; agc_register olda,oldl; int overflow; } ib_regs;
static bool begin(const apollo_overlay_state *s,const p66_bias_io *io,enum lm_interbank_entry entry,ib_regs *r){
 if(!context(s,io,entry!=LM_IB_SWRETURN&&entry!=LM_IB_ISWRETRN&&entry!=LM_IB_BANKJUMP)||(unsigned)entry>=14u||s->words[5]!=pc[entry])return false;
 r->q=s->words[2];r->oldfb=s->words[4];r->olda=s->words[0];r->oldl=s->words[1];r->overflow=agc_overflow(r->olda);return true;
}
/* A fixed inline CADR fits 15 bits; an erasable inline word is ordinary storage. */
static bool inline_ok(const apollo_overlay_state *s,const ib_regs *r,agc_word literal){
 return literal<=AGC_MASK&&!(r->q>=01400u&&r->q<02000u&&s->words[3]+(r->q&0377u)<0100u);
}
static void set_l(apollo_overlay_state *s,const ib_regs *r){s->words[1]=agc_extend(agc_correct(r->oldl));}
/* One function per group of entries, so a routine table that registers only
 * some entries (the flight store's idle-loop chunk) links only their code. */
bool lm_interbank_call(apollo_overlay_state *s,p66_bias_io *io,enum lm_interbank_entry entry,agc_word literal){ib_regs r;agc_word base,cadr=literal,relative,savedbank;
 if((entry!=LM_IB_BANKCALL&&entry!=LM_IB_SWCALL&&entry!=LM_IB_IBNKCALL&&entry!=LM_IB_ISWCALLL)||!begin(s,io,entry,&r))return false;
 base=(agc_word)(entry>=LM_IB_IBNKCALL?072:0133);
 if((entry==LM_IB_BANKCALL||entry==LM_IB_IBNKCALL)&&!inline_ok(s,&r,literal))return false;
 set_l(s,&r);
 if(entry==LM_IB_SWCALL||entry==LM_IB_ISWCALLL)cadr=agc_correct(r.olda);
 savedbank=r.oldfb;
 if(entry==LM_IB_BANKCALL||entry==LM_IB_IBNKCALL){s->words[base]=agc_correct(r.olda);s->words[base+1u]=agc_correct(r.oldl);cadr=inline_word(s,r.q,literal);r.q=agc_add_word(r.q,1);fb(s,cadr);}
 else if(r.overflow){savedbank=agc_correct(r.olda);cadr=(agc_word)(r.overflow>0?1u:01776u);}
 else fb(s,cadr);
 relative=(agc_word)(cadr&01777u);s->words[0]=agc_extend(s->words[base]);s->words[1]=agc_extend(s->words[base+1u]);s->words[base]=r.q;s->words[base+1u]=savedbank;s->words[2]=entry==LM_IB_BANKCALL||entry==LM_IB_SWCALL?04631:04707;s->words[5]=(agc_word)(02000u+relative);return true;
}
bool lm_interbank_return(apollo_overlay_state *s,p66_bias_io *io,enum lm_interbank_entry entry){ib_regs r;agc_word base;
 if((entry!=LM_IB_SWRETURN&&entry!=LM_IB_ISWRETRN)||!begin(s,io,entry,&r))return false;
 base=(agc_word)(entry==LM_IB_ISWRETRN?072:0133);
 if(s->words[base]<0100u||s->words[base]>07777u)return false;
 set_l(s,&r);
 fb(s,s->words[base+1u]);s->words[base+1u]=r.oldfb;s->words[0]=agc_extend(agc_correct(r.olda));s->words[2]=(agc_word)(base+1u);s->words[5]=s->words[base];return true;
}
bool lm_interbank_jump(apollo_overlay_state *s,p66_bias_io *io,enum lm_interbank_entry entry,agc_word literal){ib_regs r;agc_word cadr=literal;
 if((entry!=LM_IB_POSTJUMP&&entry!=LM_IB_BANKJUMP)||!begin(s,io,entry,&r))return false;
 if(entry==LM_IB_BANKJUMP&&r.q>07775u)return false;
 if(entry==LM_IB_POSTJUMP&&!inline_ok(s,&r,literal))return false;
 set_l(s,&r);
 if(entry==LM_IB_BANKJUMP)cadr=agc_correct(r.olda);
 if(entry==LM_IB_POSTJUMP){s->words[2]=r.olda;cadr=inline_word(s,r.q,literal);}fb(s,cadr);s->words[0]=entry==LM_IB_POSTJUMP?r.olda:agc_extend(r.q);
 if(entry==LM_IB_BANKJUMP&&r.overflow<0){agc_word operand=s->words[s->words[3]+0377u];enum agc_ccs_class cl=agc_classify(operand);
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
 if(entry==LM_IB_BANKJUMP&&r.overflow){s->words[2]=1;s->words[5]=02001;}
 else{s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);}return true;
}
bool lm_interbank(apollo_overlay_state *s,p66_bias_io *io,enum lm_interbank_entry entry,agc_word literal,agc_word word){ib_regs r;agc_word cadr;
 switch(entry){
 case LM_IB_BANKCALL:case LM_IB_SWCALL:case LM_IB_IBNKCALL:case LM_IB_ISWCALLL:return lm_interbank_call(s,io,entry,literal);
 case LM_IB_SWRETURN:case LM_IB_ISWRETRN:return lm_interbank_return(s,io,entry);
 case LM_IB_POSTJUMP:case LM_IB_BANKJUMP:return lm_interbank_jump(s,io,entry,literal);
 default:break;}
 if(!begin(s,io,entry,&r))return false;
 if(entry==LM_IB_USPRCADR&&!inline_ok(s,&r,literal))return false;
 if(entry==LM_IB_SUPDACAL&&word>AGC_MASK)return false;
 if(entry==LM_IB_QJUMP&&r.q>01777u)return false;
 set_l(s,&r);
 if(entry==LM_IB_MAKECADR){s->words[0]=agc_add_register(agc_extend((agc_word)(s->words[0133]&01777u)),agc_extend(s->words[0134]));s->words[5]=r.q;s->words[2]=3;return true;}
 if(entry==LM_IB_SUPDACAL){s->words[0135]=word;s->words[1]=(agc_word)(agc_correct(r.olda)&01777u);s->words[0]=agc_extend(word);
  if(r.overflow){fb(s,r.overflow>0?0:076000);if(r.overflow<0)io->superbank=0160;}
  s->words[5]=r.q;s->words[2]=3;io->interrupt_enabled=1;return true;}
 if(entry==LM_IB_USPRCADR){s->words[0164]=agc_correct(r.olda);s->words[0165]=s->words[6];s->words[023]=(agc_word)(r.overflow>0?0u:r.overflow<0?0177u:1u);cadr=inline_word(s,r.q,literal);fb(s,cadr);s->words[0164]=r.q;s->words[0]=agc_extend(agc_correct(r.olda));s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);return true;}
 if(entry==LM_IB_SUPERSW){io->superbank=(agc_word)(agc_correct(r.olda)&0160u);s->words[5]=r.q;s->words[2]=3;return true;}
 if(entry==LM_IB_QJUMP){s->words[5]=(agc_word)(02000u+r.q);return true;}
 s->words[5]=02000;return true;
}
