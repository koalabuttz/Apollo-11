/* SPDX-License-Identifier: MIT */
/* Compiled Luminary099 interpretive dispatch paths and simple order bodies.
 * Source: Luminary099/INTERPRETER.agc (DANZIG, NEWOPS, OPJUMP, ADDRESS,
 * DIRADRES, INDEX, OPJUMP2/3, 15BITADR, DOSTORE/STORE, load/store, index,
 * switch and branch orders). The static helpers below give the register
 * effects of the original basic instructions that these fixed sequences
 * use; they are called only from the compiled sequences in this file. */
#include "lm_interp_steps.h"
#include "interp_mission.h"
#define W s->words

/* Erasable address in the canonical image: 01400-01777 through EBANK. */
static unsigned ea(const apollo_overlay_state *s,unsigned a){return (a>=01400u&&a<02000u)?(unsigned)(((W[3]>>8)&7u)*0400u+(a-01400u)):a;}
/* Erasable stores with the original editing and bank-register coupling. */
static void put(apollo_overlay_state *s,unsigned a,agc_word v){
 v&=AGC_MASK;
 switch(a){
 case 3:W[3]=(agc_word)(v&03400u);W[6]=(agc_word)((W[4]&076000u)|(W[3]>>8));return;
 case 4:W[4]=(agc_word)(v&076000u);W[6]=(agc_word)((W[4]&076000u)|(W[3]>>8));return;
 case 6:W[4]=(agc_word)(v&076000u);W[3]=(agc_word)((v&7u)<<8);W[6]=(agc_word)(W[4]|(v&7u));return;
 case 7:return;
 case 020:W[020]=(agc_word)((v>>1)|((v&1u)<<14));return;
 case 021:W[021]=(agc_word)((v>>1)|(v&AGC_SIGN));return;
 case 022:W[022]=(agc_word)(((v<<1)|(v>>14))&AGC_MASK);return;
 case 023:W[023]=(agc_word)((v>>7)&0177u);return;
 default:W[ea(s,a)]=v;return;
 }
}
/* Memory-reading orders write the word back, re-editing 020-023. */
static agc_word fetch(apollo_overlay_state *s,unsigned a){agc_word v=W[ea(s,a)];if(a>=020u&&a<=023u)put(s,a,v);return v;}
static agc_register acc(apollo_overlay_state *s){return W[0];}
static void set_a(apollo_overlay_state *s,agc_register v){W[0]=v;}
static void ca(apollo_overlay_state *s,unsigned a){set_a(s,agc_extend(fetch(s,a)));}
static void caf(apollo_overlay_state *s,agc_word k){set_a(s,agc_extend(k));}
static void cs(apollo_overlay_state *s,unsigned a){set_a(s,agc_extend(agc_negate(fetch(s,a))));}
static void csf(apollo_overlay_state *s,agc_word k){set_a(s,agc_extend(agc_negate(k)));}
static void ad(apollo_overlay_state *s,unsigned a){set_a(s,agc_add_register(acc(s),agc_extend(fetch(s,a))));}
static void adf(apollo_overlay_state *s,agc_word k){set_a(s,agc_add_register(acc(s),agc_extend(k)));}
/* MASK reads its operand without the editing write-back. */
static void maskf(apollo_overlay_state *s,agc_word k){set_a(s,agc_extend((agc_word)(agc_correct(acc(s))&k)));}
static void mask(apollo_overlay_state *s,unsigned a){maskf(s,W[ea(s,a)]);}
/* TS: returns 1 when an overflowing A skips the next instruction. */
static uint8_t ts(apollo_overlay_state *s,unsigned a){int o=agc_overflow(acc(s));put(s,a,agc_correct(acc(s)));if(o){set_a(s,o>0?1u:0177776u);return 1;}return 0;}
static void ads(apollo_overlay_state *s,unsigned a){set_a(s,agc_add_register(acc(s),agc_extend(W[ea(s,a)])));put(s,a,agc_correct(acc(s)));}
static void incr(apollo_overlay_state *s,unsigned a){put(s,a,agc_add_word(W[ea(s,a)],1));}
static void xch(apollo_overlay_state *s,unsigned a){agc_word v=W[ea(s,a)];put(s,a,agc_correct(acc(s)));set_a(s,agc_extend(v));}
/* CCS: diminished absolute value in A; returns 0 +,1 +0,2 -,3 -0. */
static uint8_t ccs_value(apollo_overlay_state *s,agc_word v){
 if(v==0){set_a(s,0);return 1;}
 if(v==AGC_MASK){set_a(s,0);return 3;}
 if(v&AGC_SIGN){agc_word m=agc_negate(v);set_a(s,(agc_register)(m>1u?m-1u:0u));return 2;}
 set_a(s,(agc_register)(v>1u?v-1u:0u));return 0;
}
static uint8_t ccs(apollo_overlay_state *s,unsigned a){return ccs_value(s,fetch(s,a));}
static uint8_t ccs_a(apollo_overlay_state *s){
 agc_register v=acc(s);
 if(v==0){return 1;}
 if(v==0177777u){set_a(s,0);return 3;}
 if(v&0100000u){agc_register m=(agc_register)(~v&0177777u);set_a(s,(agc_register)(m>1u?m-1u:0u));return 2;}
 set_a(s,(agc_register)(v>1u?v-1u:0u));return 0;
}
static void dca(apollo_overlay_state *s,unsigned a){W[1]=agc_extend(fetch(s,a+1u));set_a(s,agc_extend(fetch(s,a)));}
static void dcaf(apollo_overlay_state *s,const agc_word k[2]){W[1]=agc_extend(k[1]);set_a(s,agc_extend(k[0]));}
static void dxch(apollo_overlay_state *s,unsigned a){agc_word hi=W[ea(s,a)],lo=W[ea(s,a+1u)];put(s,a,agc_correct(acc(s)));put(s,a+1u,agc_correct(W[1]));set_a(s,agc_extend(hi));W[1]=agc_extend(lo);}
/* EXTEND MP: signed product of corrected A and K in A,L. */
static void mpf(apollo_overlay_state *s,agc_word k){agc_register_pair p=agc_multiply(acc(s),k);set_a(s,p.a);W[1]=p.l;}

/* Fixed constants named in the source. */
#define LOW7 0177u
#define LOW8 0377u
#define LOW10 01777u
#define LOW11 03777u
#define HIGH4 074000u
#define HIGH9 077700u
#define BIT1 1u
#define BIT5 020u
#define BIT15 040000u
#define B12T14 034000u
#define OCT1400 01400u
#define TWO_K 02000u
#define ENDVAC_NEG 077722u /* -ENDVAC DEC -45 */
#define ENDERAS_NEG 074056u /* -ENDERAS DEC -2001 */
#define DEC45 055u

void lmi_danzig_next(apollo_overlay_state *s){
 /* DANZIG CA BANKSET, TS BBANK; NOIBNKSW CCS EDOP (+0); CCS NEWJOB (+0);
  * INCR LOC; NEWOPS. */
 ca(s,0165);(void)ts(s,6);(void)ccs(s,023);(void)ccs(s,067);incr(s,0164);W[5]=IF_(06070,06037);
}
static void opjump(apollo_overlay_state *s){
 /* OPJUMP TS CYR; CCS CYR. */
 (void)ts(s,020);(void)ccs(s,020);
}
void lmi_newops(apollo_overlay_state *s,agc_word pair){
 /* NEWOPS INDEX LOC, CA 0; CCS A (negative pair); TS EDOP; MASK LOW7.
  * Every instruction ends with L overflow-corrected and sign-extended. */
 W[1]=agc_extend(agc_correct(W[1]));caf(s,pair);(void)ccs_a(s);(void)ts(s,023);maskf(s,LOW7);opjump(s);
}
void lmi_danzig_right(apollo_overlay_state *s){
 /* DANZIG CA BANKSET, TS BBANK; CCS EDOP (positive) TCF OPJUMP. */
 ca(s,0165);(void)ts(s,6);(void)ccs(s,023);opjump(s);
}
static void indjump(apollo_overlay_state *s,agc_word target){
 /* ITRnn INDEX CYR; 7 (or 3) INDJUMP -1: the transfer reads CYR without
  * rewriting it. */
 W[5]=target;
}
void lmi_direct(apollo_overlay_state *s,agc_word operand,agc_word target){
 /* ADDRESS MASK BIT1; CCS A (+0); DIRADRES INDEX LOC, CS 1; CCS A
  * (negative, an address is given); INCR LOC; TS ADDRWD; AD -ENDVAC;
  * CCS A. */
 maskf(s,BIT1);(void)ccs_a(s);csf(s,operand);(void)ccs_a(s);incr(s,0164);(void)ts(s,0116);adf(s,ENDVAC_NEG);
 if(ccs_a(s)==2u){
  /* NETZERO CA FIXLOC; ADS ADDRWD. */
  ca(s,0120);ads(s,0116);indjump(s,target);return;
 }
 /* AD -ENDERAS; IERASTST EXTEND BZMF. */
 adf(s,ENDERAS_NEG);
 if(acc(s)==0||(acc(s)&0100000u)){
  /* GEADDR MASK LOW8; AD OCT1400; XCH ADDRWD; TS EBANK. */
  maskf(s,LOW8);adf(s,OCT1400);xch(s,0116);(void)ts(s,3);indjump(s,target);return;
 }
 /* MASK LOW10; AD LOW10; XCH ADDRWD; AD INTBIT15; TS FBANK. */
 maskf(s,LOW10);adf(s,LOW10);xch(s,0116);ad(s,0115);(void)ts(s,4);indjump(s,target);
}
void lmi_indexed(apollo_overlay_state *s,agc_word operand,agc_word target){
 /* ADDRESS MASK BIT1; CCS A (positive) TCF INDEX. INDEX CA FIXLOC;
  * TS INDEXLOC; INCR LOC; INDEX LOC, CS 0; CCS A; [INCR INDEXLOC; NOOP;]
  * TS ADDRWD; MASK HIGH4; BZF INDEX2; [CA INTBIT15; ADS ADDRWD;] INDEX2
  * INDEX INDEXLOC, CS X1; ADS ADDRWD; MASK HIGH9; BZF INDWORK; MASK
  * HIGH4; BZF INDERASE; fixed bank otherwise. */
 maskf(s,BIT1);(void)ccs_a(s);ca(s,0120);(void)ts(s,0130);incr(s,0164);csf(s,operand);
 if(ccs_a(s)==0u)incr(s,0130);
 (void)ts(s,0116);maskf(s,HIGH4);
 if(acc(s)){ca(s,0115);ads(s,0116);}
 cs(s,(unsigned)(W[0130]+046u));ads(s,0116);maskf(s,HIGH9);
 if(!acc(s)){ca(s,0120);ads(s,0116);indjump(s,target);return;}
 maskf(s,HIGH4);
 if(!acc(s)){caf(s,OCT1400);xch(s,0116);(void)ts(s,3);maskf(s,LOW8);ads(s,0116);indjump(s,target);return;}
 ca(s,0116);(void)ts(s,4);maskf(s,LOW10);adf(s,TWO_K);(void)ts(s,0116);indjump(s,target);
}
void lmi_misc(apollo_overlay_state *s,agc_word operand,agc_word target){
 /* OPJUMP2 CCS CYR (negative); 15BITADR INCR LOC; INDEX LOC, CA 0;
  * TS POLISH; CAF LOW7+2K; TS FBANK; MASK CYR; INDEX A; TCF MISCJUMP. */
 (void)ccs(s,020);incr(s,0164);caf(s,operand);(void)ts(s,0117);caf(s,02177u);(void)ts(s,4);mask(s,020);W[5]=target;
}
void lmi_unary(apollo_overlay_state *s,agc_word target){
 /* OPJUMP2 CCS CYR (positive); OPJUMP3 TS FBANK; CCS CYR (positive);
  * INDEX A; TCF UNAJUMP. */
 (void)ccs(s,020);(void)ts(s,4);(void)ccs(s,020);W[5]=target;
}
static void danzig_entry(apollo_overlay_state *s){W[5]=IF_(06061,06030);}

/* STORE CS ADDRWD; AD DEC45; CCS A; work area CA FIXLOC, otherwise CA
 * OCT1400; XCH ADDRWD; TS EBANK; MASK LOW8; AHEAD5 ADS ADDRWD; STARTSTO
 * EXTEND, DCA MPAC; INDEX ADDRWD, DXCH 0; CCS MODE; TSTORE/VSTORE; TC Q. */
static void store_body(apollo_overlay_state *s){
 agc_word mode,at;
 cs(s,0116);adf(s,DEC45);
 if(ccs_a(s)==0u)ca(s,0120);
 else{caf(s,OCT1400);xch(s,0116);(void)ts(s,3);maskf(s,LOW8);}
 ads(s,0116);at=W[0116];
 dca(s,0154);dxch(s,at);mode=W[0163];(void)ccs(s,0163);
 if(mode!=0&&!(mode&AGC_SIGN)){ca(s,0156);(void)ts(s,at+2u);}
 else if(mode&AGC_SIGN){dca(s,0157);dxch(s,at+2u);dca(s,0161);dxch(s,at+4u);}
 /* TC Q executes the TC in Q (TC STORJUMP +1), leaving Q = 3. */
 W[2]=3;
}
void lmi_store(apollo_overlay_state *s,agc_word code){
 /* NEWOPS INDEX LOC, CA 0; CCS A (positive) TCF DOSTORE; TS ADDRWD;
  * MASK LOW11; XCH ADDRWD; MASK B12T14; EXTEND MP BIT5; INDEX A;
  * TCF STORJUMP; TC STORE (or STORE,1/STORE,2). */
 unsigned kind;
 caf(s,code);(void)ccs_a(s);(void)ts(s,0116);maskf(s,LOW11);xch(s,0116);maskf(s,B12T14);mpf(s,BIT5);
 kind=(unsigned)acc(s);
 if(kind==2u){cs(s,(unsigned)(W[0120]+046u));ads(s,0116);}       /* STORE,1 */
 else if(kind==4u){cs(s,(unsigned)(W[0120]+047u));ads(s,0116);}  /* STORE,2 */
 store_body(s);danzig_entry(s);
}
void lmi_dload(apollo_overlay_state *s){
 /* DLOAD EXTEND, INDEX ADDRWD, DCA 0; SLOAD2 DXCH MPAC; CAF ZERO;
  * TS MPAC +2; NEWMODE TS MODE. */
 dca(s,W[0116]);dxch(s,0154);caf(s,0);(void)ts(s,0156);(void)ts(s,0163);danzig_entry(s);
}
void lmi_sload(apollo_overlay_state *s){
 /* SLOAD ZL; INDEX ADDRWD, CA 0; TCF SLOAD2. */
 W[1]=0;ca(s,W[0116]);dxch(s,0154);caf(s,0);(void)ts(s,0156);(void)ts(s,0163);danzig_entry(s);
}
void lmi_pddl(apollo_overlay_state *s){
 /* PDDL EXTEND, INDEX ADDRWD, DCA 0; DXCH MPAC; INDEX PUSHLOC, DXCH 0;
  * INDEX MODE, CAF NO.WDS; ADS PUSHLOC; CCS MODE: ENDTPUSH, ENDDPUSH
  * or vector (TS MODE; ENDVPUSH). */
 static const agc_word nowds_minus[3]={6u,2u,3u}; /* NO.WDS -1, 0, +1 */
 agc_word mode=W[0163];unsigned idx=mode==0?1u:(mode&AGC_SIGN)?0u:2u;uint8_t c;
 dca(s,W[0116]);dxch(s,0154);dxch(s,W[0166]);caf(s,nowds_minus[idx]);ads(s,0166);c=ccs(s,0163);
 if(c==0u){(void)ts(s,0163);xch(s,0156);(void)ts(s,(unsigned)(W[0166]-1u));}
 else if(c==1u){(void)ts(s,0156);}
 else{(void)ts(s,0163);(void)ts(s,0156);dxch(s,0157);dxch(s,(unsigned)(W[0166]-4u));dxch(s,0161);dxch(s,(unsigned)(W[0166]-2u));}
 danzig_entry(s);
}
void lmi_setpd(apollo_overlay_state *s){
 /* SETPD CA ADDRWD; TS PUSHLOC; TCF NOIBNKSW (DANZIG without BBANK). */
 ca(s,0116);(void)ts(s,0166);W[5]=IF_(06063,06032);
}
void lmi_vdef(apollo_overlay_state *s){
 /* VDEF CS FOUR; ADS PUSHLOC; EXTEND, INDEX A, DCA 2; DXCH MPAC +3;
  * EXTEND, INDEX PUSHLOC, DCA 0; DXCH MPAC +5; VMODE CS ONE; NEWMODE. */
 unsigned at;
 csf(s,4);ads(s,0166);at=(unsigned)agc_correct(acc(s));dca(s,at+2u);dxch(s,0157);dca(s,W[0166]);dxch(s,0161);csf(s,1);(void)ts(s,0163);danzig_entry(s);
}
/* TAGSUB CA FIXLOC; TS INDEXLOC; CCS CYR; [INCR INDEXLOC]; TC Q. The
 * TC Q return executes the TC held in Q, leaving Q = 3. */
static void tagsub(apollo_overlay_state *s){
 ca(s,0120);(void)ts(s,0130);if(ccs(s,020)==0u)incr(s,0130);W[2]=3;
}
void lmi_axt(apollo_overlay_state *s){
 /* AXT TC TAGSUB; CA POLISH; XSTORE INDEX INDEXLOC, TS X1. TAGSUB's
  */
 tagsub(s);ca(s,0117);(void)ts(s,(unsigned)(W[0130]+046u));danzig_entry(s);
}
void lmi_incr(apollo_overlay_state *s){
 /* INCR TC TAGSUB; CA POLISH; XAD2 INDEX INDEXLOC, ADS X1. */
 tagsub(s);ca(s,0117);ads(s,(unsigned)(W[0130]+046u));danzig_entry(s);
}

/* GOTO +4: CA BANKSET; TS BBANK; CA POLISH; TS FBANK; MASK LOW10; AD 2K;
 * TS LOC; TCF INTPRET +3 (CA BBANK; TS BANKSET; MASK BIT15; TS INTBIT15;
 * TS EDOP; TCF NEWOPS). Only fixed-bank targets are compiled here. */
static void goto_fixed(apollo_overlay_state *s){
 ca(s,0165);(void)ts(s,6);ca(s,0117);(void)ts(s,4);maskf(s,LOW10);adf(s,TWO_K);(void)ts(s,0164);
 ca(s,6);(void)ts(s,0165);maskf(s,BIT15);(void)ts(s,0115);(void)ts(s,023);W[5]=IF_(06070,06037);
}
void lmi_goto(apollo_overlay_state *s){
 /* GOTO CA POLISH; +1 MASK HIGH4; EXTEND BZF GOTOERS (fixed: nonzero). */
 ca(s,0117);maskf(s,HIGH4);goto_fixed(s);
}
void lmi_rvq(apollo_overlay_state *s){
 /* RVQ INDEX FIXLOC, CA QPRET; TS POLISH; TCF GOTO +4. */
 ca(s,(unsigned)(W[0120]+052u));(void)ts(s,0117);goto_fixed(s);
}
/* BRANCH (06723): CCS MPAC, MPAC +1, MPAC +2 in turn. A positive register
 * returns through TC Q (Q = 3), all zero through INDEX Q, TC 1 (Q 06741)
 * and a negative one through INDEX Q, TC 2 (Q 06743). 0 +,1 zero,2 -. */
/* BRANCH: -0 falls through CCS's fourth slot to the next word, as +0. */
static uint8_t branch(apollo_overlay_state *s){
 unsigned i;uint8_t c;
 for(i=0;i<3u;++i){c=ccs(s,0154u+i);if(c==0u){W[2]=3;return 0;}if(c==2u){W[2]=IF_(06743,06712);return 2;}}
 W[2]=IF_(06741,06710);return 1;
}
uint8_t lmi_bze(apollo_overlay_state *s){
 /* BZE/GOTO CCS CYR (positive); TC BRANCH; zero TCF GOTO, else DANZIG. */
 (void)ccs(s,020);if(branch(s)!=1u){danzig_entry(s);return 0;}lmi_goto(s);return 1;
}
uint8_t lmi_bpl(apollo_overlay_state *s){
 /* BPL/BMN CCS CYR (positive) TCF BPL; TC BRANCH; + or zero TCF GOTO. */
 (void)ccs(s,020);if(branch(s)==2u){danzig_entry(s);return 0;}lmi_goto(s);return 1;
}
/* SWITCHES: SWBIT/SWWORD in BUF +1/BUF, STATE word in Q, set/invert/clear
 * by POLISH bits 7-8 under INHINT, RELINT, then branch/skip/noop by bits
 * 5-6. The INHINT/RELINT pair leaves interrupts enabled. */
uint8_t lmi_switch(apollo_overlay_state *s,p66_bias_io *io){
 static const agc_word bits[16]={040000,020000,010000,04000,02000,01000,0400,0200,0100,040,020,010,4,2,1,0};
 unsigned op,word;agc_word old;
 caf(s,017);mask(s,0117);caf(s,bits[acc(s)&017u]);(void)ts(s,0131);caf(s,0100);mpf(s,W[0117]);(void)ts(s,0130);
 io->interrupt_enabled=0;word=(unsigned)acc(s);ca(s,074u+word);(void)ts(s,2);old=W[2];
 caf(s,02000);mpf(s,W[0117]);maskf(s,014);op=(unsigned)acc(s);
 if(op==0u){ca(s,0131);set_a(s,(agc_register)(acc(s)|agc_extend(old)));(void)ts(s,074u+W[0130]);}
 else if(op==4u){ca(s,0131);set_a(s,(agc_register)(acc(s)^agc_extend(old)));(void)ts(s,074u+W[0130]);}
 else if(op==010u){cs(s,0131);maskf(s,old);(void)ts(s,074u+W[0130]);}
 io->interrupt_enabled=1;caf(s,010000);mpf(s,W[0117]);maskf(s,014);op=(unsigned)acc(s);
 if(op==014u){danzig_entry(s);return 0;}
 if(op==4u)return 1; /* +5 TCF SWBRANCH */
 if(op==0u)cs(s,2);else ca(s,2);
 /* TEST MASK SWBIT; CCS A: positive skips, +0 branches. */
 mask(s,0131);
 if(ccs_a(s)==1u)return 1;
 incr(s,0164);danzig_entry(s);return 0;
}
void lmi_swbranch(apollo_overlay_state *s,agc_word target_word){
 /* SWBRANCH CA BANKSET; TS FBANK; INDEX LOC, CA 1; TS POLISH; TCF GOTO +1. */
 ca(s,0165);(void)ts(s,4);caf(s,target_word);(void)ts(s,0117);maskf(s,HIGH4);goto_fixed(s);
}
void lmi_ddv_prologue(apollo_overlay_state *s,const agc_word divisor[2]){
 /* DDV EXTEND, INDEX ADDRWD, DCA 0; TCF BDDV +4: DXCH BUF; CAF ZERO;
  * TS FBANK; TCF DDV/BDDV. */
 dcaf(s,divisor);dxch(s,0130);caf(s,0);(void)ts(s,4);W[5]=02353;
}
void lmi_gshift_prologue(apollo_overlay_state *s){
 /* GSHIFT CAF LOW7; TS FBANK; TCF GENSHIFT. */
 caf(s,LOW7);(void)ts(s,4);W[5]=02214;
}

void lmi_sload_fixed(apollo_overlay_state *s,const agc_word operand[1]){
 /* SLOAD with ADDRWD naming a fixed word in the selected bank. */
 W[1]=0;caf(s,operand[0]);dxch(s,0154);caf(s,0);(void)ts(s,0156);(void)ts(s,0163);danzig_entry(s);
}
void lmi_dload_fixed(apollo_overlay_state *s,const agc_word operand[2]){
 /* DLOAD with ADDRWD naming a fixed DP operand in the selected bank. */
 dcaf(s,operand);dxch(s,0154);caf(s,0);(void)ts(s,0156);(void)ts(s,0163);danzig_entry(s);
}
void lmi_noibnksw_right(apollo_overlay_state *s){
 /* NOIBNKSW CCS EDOP (positive) TCF OPJUMP (SETPD's return). */
 (void)ccs(s,023);opjump(s);
}
void lmi_goto_order(apollo_overlay_state *s){
 /* BZE/GOTO CCS CYR (not positive) TCF GOTO. */
 (void)ccs(s,020);lmi_goto(s);
}
#undef W
