/* SPDX-License-Identifier: MIT
 * Luminary099 THROTTLE_CONTROL_ROUTINES.agc, all of it (bank 31,
 * 02216-02417), and LUNAR_LANDING_GUIDANCE_EQUATIONS FASTCHNG (31,3740),
 * as routines of the control-transfer ABI. Each source instruction is one
 * agc_image_ops call (yaAGC-qualified register effects), named with its
 * erasable address or, for a fixed operand, the source constant. TS
 * overflow skips are followed as the original does. */
#include "lm_throttle.h"
#include "agc_image_ops.h"
#define W s->words
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_THR_COUNT_PATHS
extern unsigned lm_thr_paths[];
#define LM_THR_PATH(n) ((void)++lm_thr_paths[n])
#else
#define LM_THR_PATH(n) ((void)0)
#endif
#define P(z) (uint16_t)(031u*1024u+((z)-02000u))
/* Erasable (EBANK=PIF is EBANK7; LOWCRIT is in EBANK5, PHSNAME3 in EBANK3). */
#define ABDELV 01246u
#define RTNHOLD 01607u
#define PIFPSET 01606u
#define FWEIGHT 01610u
#define PIF 01612u
#define PSEUDO55 01614u
#define FC 01615u
#define TTHROT 01617u
#define FCOLD 01620u
#define AFC 01463u      /* /AFC/ */
#define FCODD 01465u
#define FP 01467u
#define LOWCRIT 01506u
#define LWCRIT 0130u    /* L*WCR*T = BUF */
#define HGHCRIT 0131u   /* H*GHCR*T = BUF +1 */
#define BUF 0130u
#define BUF1 0131u
#define THISTPIP1 01762u /* THISTPIP +1 */
#define PIPTIME1 01235u  /* PIPTIME +1 */
#define MODREG 01011u
#define TIME1 025u
#define THRUST 055u
#define MPAC 0154u
#define MPAC1 0155u
#define MPAC2 0156u
#define ADDRWD 0116u
#define MPTEMP 0135u
#define MASS 01244u
#define PHSNAME3 01442u
#define EBANK 3u
/* Fixed constants: /AF/CNST 31,2417 (DEC .13107), POSMAX 04733, 3SECS 05002,
 * EBANK5 05014, EBANK7 05016, EBANK3 05007, ZERO 04755, FMAXODD 31,2002,
 * FMAXPOS 31,2003, THROTLAG 31,2004, SCALEFAC 31,2005-2006, DEC66 31,2501,
 * BIT4 04750, BIT6 04746, BIT13 04737 (FEXTRA = BIT13), 2SECS 05000,
 * 4SECS 05003, LOW8 04357, ONE 04753. */
#define AFCNST 04143u
#define POSMAX 037777u
#define SECS3 0454u
#define EBANK5 02400u
#define EBANK7 03400u
#define EBANK3 01400u
#define ZERO 0u
#define FMAXODD 07401u
#define FMAXPOS 06613u
#define THROTLAG 024u
#define DEC66 0102u
#define BIT4 010u
#define BIT6 040u
#define BIT13 010000u
#define SECS2 0310u
#define SECS4 0620u
#define LOW8 0377u
#define ONE 1u
static const agc_word scalefac[2]={00307u,017534u};
#define CHAN14_RETURN 02303u

/* Callers run under EBANK7 (EBANK= PIF / E2DPS) in bank 31. */
static int entry(const agc_image *s){return s&&(W[4]&076000u)==LM_THR_FB&&W[3]==EBANK7;}
/* A plain TC into fixed memory, as every return word is. */
static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
/* TC K, K holding the return word: Z = the word, Q = K+1. */
static enum exec_status tc_indirect(agc_image *s,agc_register word,agc_word k){
 if(!tc_word(word))return EXEC_OUT_OF_SCOPE;
 W[5]=word;ai_tc(s,(agc_word)(k+1u));return EXEC_OK;
}
static enum exec_status transfer(agc_image *s,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;return EXEC_OK;}

/* INTERPRETER DMP/DMPSUB (07103) entered by TC DMP with the ADRES after it:
 * INDEX Q; CAF 0; INCR Q; TS ADDRWD, then MPAC (TP) = MPAC x C(ADRES) and
 * TC Q back past the ADRES (Q = 3 after the TC Q). k names a fixed operand's
 * two words; NULL means the erasable pair at adres. */
static void dmp(agc_image *s,agc_word adres,const agc_word *k,agc_word ret){
 ai_caf(s,adres);ai_incr(s,2u);(void)ai_ts(s,ADDRWD);
 if(k)ai_caf(s,k[1]);else ai_ca(s,(unsigned)adres+1u);
 (void)ai_ts(s,MPAC2);ai_caf(s,ZERO);ai_xch(s,MPAC1);(void)ai_ts(s,MPTEMP);ai_mp(s,MPAC2);
 ai_xch(s,MPAC2);ai_mp(s,MPAC);ai_das(s,MPAC1);
 if(k)ai_caf(s,k[0]);else ai_ca(s,adres);
 ai_xch(s,MPTEMP);ai_mp(s,MPTEMP);ai_das(s,MPAC1);
 ai_xch(s,MPAC);ai_mp(s,MPTEMP);ai_das(s,MPAC);
 (void)ret;ai_tc(s,3u); /* TC Q: Z = ret, Q = 3 */
}
/* INTERPRETER TPAGREE (07257) with BRANCH (06723), entered by TC TPAGREE:
 * LXCH Q; TC BRANCH; the sign of MPAC selects ARG+, the negative entry or
 * ARGZERO; the triple is forced to agree in sign; TC L returns (Q = 2). */
static void tpagree(agc_image *s,agc_word ret){uint8_t c,k=1;unsigned i;static const unsigned m[3]={MPAC,MPAC1,MPAC2};
 ai_tc(s,ret);ai_lxch(s,2u);ai_tc(s,07261u);
 /* BRANCH: CCS MPAC, MPAC+1, MPAC+2 until one is nonzero. */
 for(i=0;i<3u;++i){c=ai_ccs(s,m[i]);if(c==0u||c==2u){k=c;break;}}
 if(i==3u){LM_THR_PATH(18); /* Q+1: INDEX Q; TC 1 -> TCF ARGZERO; TS MPAC+2; TCF ARGZERO2 */
  ai_tc(s,06741u);(void)ai_ts(s,MPAC2);(void)ai_ts(s,MPAC);(void)ai_ts(s,MPAC1);
 }else{
  LM_THR_PATH(16u+k/2u);
  if(k==0u){ai_tc(s,3u);ai_caf(s,POSMAX);}    /* TC Q; TCF ARG+ */
  else{ai_tc(s,06743u);ai_csf(s,POSMAX);}      /* NEG: INDEX Q; TC 2; CS POSMAX; TCF +2 */
  (void)ai_ts(s,2u);ai_aug(s,0u);ai_ad(s,MPAC2);
  if(!ai_ts(s,MPAC2))ai_caf(s,ZERO);
  ai_ad(s,2u);ai_ad(s,MPAC1);
  if(!ai_ts(s,MPAC1))ai_caf(s,ZERO);
  ai_ad(s,2u);ai_ad(s,MPAC);
  if(!ai_ts(s,MPAC))(void)ai_ts(s,MPAC1);      /* ARGZERO2: always skipping unless argzero */
 }
 W[5]=(agc_word)(W[1]&07777u);ai_tc(s,2u);      /* TC L */
}

/* THROTTLE: CA ABDELV; EXTEND; MP /AF/CNST; then THROTTLE +3. */
static enum exec_status throttle3_body(agc_image *s){ai_qxch(s,RTNHOLD);return transfer(s,02224u,LM_THR_MASSMULT);}
static enum exec_status throttle(const agc_native_program *p,agc_image *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,ABDELV);ai_mpf(s,AFCNST);return throttle3_body(s);}
/* THROTTLE +3: EXTEND; QXCH RTNHOLD; AFDUMP TC MASSMULT. */
static enum exec_status throttle3(const agc_native_program *p,agc_image *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 return throttle3_body(s);}
/* 31,2224: DXCH FP; EXTEND; DCA /AFC/; TC MASSMULT. */
static enum exec_status present(const agc_native_program *p,agc_image *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,FP);ai_dca(s,AFC);return transfer(s,02230u,LM_THR_MASSMULT);}
/* 31,2230: TS FC; DXCH FCODD; the 3-second FWEIGHT test; WHERETO's
 * critical-thrust bands; TC FASTCHNG at DOPIF (skipped when TS PIFPSET
 * overflows at FLATOUT2). */
static enum exec_status desired(const agc_native_program *p,agc_image *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 if(!ai_ts(s,FC))ai_dxch(s,FCODD);else LM_THR_PATH(21);
 ai_cs(s,TTHROT);ai_ad(s,TIME1);ai_maskf(s,POSMAX);ai_com(s);ai_adf(s,SECS3);
 if(!ai_bzmf(s)){LM_THR_PATH(0);ai_dca(s,FWEIGHT);ai_das(s,FP);}else LM_THR_PATH(1);
 /* WHERETO */
 ai_caf(s,EBANK5);(void)ai_ts(s,EBANK);ai_dca(s,LOWCRIT);ai_dxch(s,LWCRIT);
 ai_caf(s,EBANK7);(void)ai_ts(s,EBANK);
 ai_csf(s,ZERO);(void)ai_ts(s,PIFPSET);
 ai_cs(s,HGHCRIT);ai_ad(s,FCOLD);
 if(ai_bzmf(s)){
  /* LOWFCOLD */
  LM_THR_PATH(2);ai_cs(s,HGHCRIT);ai_ad(s,FCODD);
  if(ai_bzmf(s)){LM_THR_PATH(3);return transfer(s,CHAN14_RETURN,LM_THR_FASTCHNG);}
  LM_THR_PATH(4);ai_caf(s,FMAXPOS);
 }else{
  ai_cs(s,LWCRIT);ai_ad(s,FCODD);
  if(ai_bzmf(s)){ /* FCOMPSET */
   LM_THR_PATH(5);ai_csf(s,FMAXODD);ai_ad(s,FP);goto flatout2;
  }
  LM_THR_PATH(6);ai_ca(s,FP);
 }
 /* FLATOUT1 */
 ai_dxch(s,FCODD);ai_caf(s,BIT13);
flatout2:
 if(ai_ts(s,PIFPSET)){LM_THR_PATH(7);W[5]=CHAN14_RETURN;return EXEC_OK;} /* skips TC FASTCHNG */
 return transfer(s,CHAN14_RETURN,LM_THR_FASTCHNG);
}
/* DOIT .. THDUMP: THRUST and PSEUDO55, channel 14 bit 4, TTHROT, the
 * FWEIGHT compensation, TC RTNHOLD. */
static enum exec_status doit(agc_image *s,lm_flight_io *io){agc_word v;uint8_t c;
 ai_ca(s,PIF);ai_ad(s,PIFPSET);
 if(ai_ts(s,PSEUDO55)){LM_THR_PATH(8);ai_caf(s,BIT4);}
 else if(!ai_ts(s,THRUST))ai_caf(s,BIT4);
 else LM_THR_PATH(9);
 /* EXTEND; WOR CHAN14 */
 v=(agc_word)(((W[0]&037777u)|((W[0]>>1)&040000u))|io->chan14);io->chan14=v;W[0]=agc_extend(v);
 ai_ca(s,TIME1);(void)ai_ts(s,TTHROT);ai_ca(s,THISTPIP1);(void)ai_ts(s,BUF);
 ai_cs(s,MODREG);ai_adf(s,DEC66);
 if(ai_bzf(s)){LM_THR_PATH(10);ai_caf(s,SECS2);} /* FWCOMP */
 else{LM_THR_PATH(11);ai_ca(s,PIPTIME1);(void)ai_ts(s,BUF);ai_caf(s,SECS4);}
 (void)ai_ts(s,2u);ai_mpf(s,BIT6);ai_lxch(s,BUF1);
 ai_cs(s,BUF);ai_ad(s,TIME1);ai_adf(s,THROTLAG);ai_maskf(s,LOW8);ai_zl(s);ai_dv(s,2u);
 ai_mp(s,PIF);ai_double(s);ai_dxch(s,FWEIGHT);
 c=ai_ccs(s,PIF);LM_THR_PATH(12u+c);if(c==0u||c==2u)ai_adf(s,ONE);
 ai_mp(s,PIF);ai_dv(s,BUF1);ai_zl(s);ai_das(s,FWEIGHT);
 /* THDUMP: TC RTNHOLD */
 return tc_indirect(s,ai_word(s,RTNHOLD),RTNHOLD);
}
/* 31,2303 (FASTCHNG's return): EXTEND; DCA FCODD; TS FCOLD; DXCH PIF;
 * EXTEND; DCS FP; DAS PIF; DOIT. */
static enum exec_status pif(const agc_native_program *p,agc_image *s,void *io){(void)p;
 if(!entry(s)||!io||!tc_word(ai_word(s,RTNHOLD)))return EXEC_OUT_OF_SCOPE;
 ai_dca(s,FCODD);if(!ai_ts(s,FCOLD))ai_dxch(s,PIF);else LM_THR_PATH(22);ai_dcs(s,FP);ai_das(s,PIF);
 return doit(s,io);}
/* WHATOUT: TS PIFPSET; CS ZERO; TS FCOLD; TS PIF; EXTEND; QXCH RTNHOLD;
 * TCF DOIT. */
static enum exec_status whatout_body(agc_image *s,lm_flight_io *io){
 if(!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 if(!ai_ts(s,PIFPSET))ai_csf(s,ZERO);else LM_THR_PATH(24);
 (void)ai_ts(s,FCOLD);(void)ai_ts(s,PIF);ai_qxch(s,RTNHOLD);
 return doit(s,io);
}
static enum exec_status whatout(const agc_native_program *p,agc_image *s,void *io){(void)p;
 if(!entry(s)||!io)return EXEC_OUT_OF_SCOPE;
 return whatout_body(s,io);}
/* FLATOUT: CAF BIT13 (4096 pulses); WHATOUT. */
static enum exec_status flatout(const agc_native_program *p,agc_image *s,void *io){(void)p;
 if(!entry(s)||!io||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,BIT13);return whatout_body(s,io);}
/* MASSMULT: EXTEND; QXCH BUF; DXCH MPAC; TC DMP; ADRES MASS; TC DMP;
 * ADRES SCALEFAC; TC TPAGREE; CA MPAC; EXTEND; BZF +3; CAF POSMAX; TC BUF
 * (or +3: DXCH MPAC +1; TC BUF). */
static enum exec_status massmult(const agc_native_program *p,agc_image *s,void *io){(void)p;(void)io;
 if(!entry(s)||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 ai_qxch(s,BUF);ai_dxch(s,MPAC);
 ai_tc(s,02404u);dmp(s,MASS,0,02405u);
 ai_tc(s,02406u);dmp(s,02005u,scalefac,02407u);
 tpagree(s,02410u);
 ai_ca(s,MPAC);
 if(ai_bzf(s)){LM_THR_PATH(19);ai_dxch(s,MPAC1);}else{LM_THR_PATH(20);ai_caf(s,POSMAX);}
 return tc_indirect(s,ai_word(s,BUF),BUF);
}
/* LUNAR_LANDING_GUIDANCE_EQUATIONS FASTCHNG: CA EBANK3; XCH EBANK; DXCH L;
 * TS PHSNAME3; LXCH EBANK; TC A. */
static enum exec_status fastchng(const agc_native_program *p,agc_image *s,void *io){agc_word q;(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 q=W[2];if(!tc_word(q&077777u)||(q&0100000u))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,EBANK3);ai_xch(s,EBANK);ai_dxch(s,1u);
 if(!ai_ts(s,PHSNAME3))ai_lxch(s,EBANK);else LM_THR_PATH(23);
 W[5]=(agc_word)(W[0]&07777u);ai_tc(s,1u); /* TC A: Z = A, Q = 1 */
 return EXEC_OK;
}
static const agc_routine_entry entries[9]={
 {P(02216u),throttle},{P(02221u),throttle3},{P(02224u),present},{P(02230u),desired},{P(02303u),pif},
 {P(02370u),flatout},{P(02371u),whatout},{P(02400u),massmult},{P(03740u),fastchng}};
const agc_routine_table lm_throttle_routines={entries,9};
