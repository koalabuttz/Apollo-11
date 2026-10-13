/* SPDX-License-Identifier: MIT
 * Luminary099 TJET_LAW.agc, all of it (bank 17, 03252-03743), as a routine
 * of the control-transfer ABI. Each source instruction is one
 * agc_image_ops call (yaAGC-qualified register effects), named with its
 * erasable address or, for a fixed operand, the source constant. INDEX
 * adds the indexer's signed value to the operand, as the original's
 * instruction-word sum does. TS overflow skips and CCS's four exits are
 * followed as the original does; a TS that skips an EXTEND leaves the next
 * word to run as a basic order, which is composed here with its rope word. */
#include "lm_tjet_law.h"
#include "agc_image_ops.h"
/* Every instruction runs on the one canonical image through
 * include/agc_fast_redirect.h: a plain-cell operand under this file's
 * EBANK 6 goes to the fast helper (include/agc_fast_ops.h), any other
 * operand to a one-argument wrapper of the agc_image_ops helper; both have
 * its effects. -DLM_TJ_GENERIC builds the same source on agc_image_ops alone. */
#ifdef LM_TJ_GENERIC
#define AFR_GENERIC
#endif
#define AFR_EBN (LM_TJ_EB>>8)
#include "agc_fast_redirect.h"
#define W AFR_S(s)->words
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_TJ_COUNT_PATHS
extern unsigned lm_tj_paths[];
#define LM_TJ_PATH(n) ((void)++lm_tj_paths[n])
#else
#define LM_TJ_PATH(n) ((void)0)
#endif
#define P(z) (uint16_t)(017u*1024u+((z)-02000u))
/* Erasable (EBANK= TJP is EBANK6; EDOT = OMEGAV, the DAPTEMPs and DAPTREGs,
 * and the 1/ACCS parameter block BLOCKTOP .. 1/ANET1 +15). */
#define EDOT 01427u
#define SENSETYP 01500u
#define AXISCTR 01505u
#define TJETU 01525u
#define ACCSWU 01547u
#define FLAT 01555u
#define ZONE3LIM 01556u
#define ANET1 01567u    /* 1/ANET1 */
#define ANET2 01570u    /* 1/ANET2 */
#define ANET1_2 01571u  /* 1/ANET1 +2 */
#define ACOAST 01573u   /* 1/ACOAST */
#define ACCFCTZ1 01575u
#define ACCFCTZ5 01576u
#define FIREDB 01601u
#define COASTDB 01603u
#define AXISDIST 01605u
#define EDOTSQ 01737u
#define ROTSENSE 01740u
#define FIREFCT 01741u
#define TTOAXIS 01742u
#define NUMBERT 01743u
#define ADRSDIF2 01744u
#define HOLDQ 01745u
#define ADRSDIF1 01746u
#define HH 01747u
#define E 01752u
/* Fixed constants: ONE 04753, TWO 04752, FOUR 04751, BIT4 04750,
 * BIT5 04747, BIT6 04746, BIT11 04741, BIT12 04740, BIT13 04737,
 * BIT14 04736, NEGMAX 04735, ZERO 04755, NEG2 07746, -1/8 07741,
 * NEGONE 07747; this file's AXISDIFF (17,3721-3723, in tjetlaw) and DEC
 * constants (17,3724-3743). */
#define ONE 1u
#define TWO 2u
#define FOUR 4u
#define BIT4 010u
#define BIT5 020u
#define BIT6 040u
#define BIT11 02000u
#define BIT12 04000u
#define BIT13 010000u
#define BIT14 020000u
#define NEGMAX 040000u
#define ZERO 0u
#define NEG2 077775u
#define NEG1_8 073777u   /* -1/8 */
#define NEGONE 077776u
#define SENSOR 014400u
#define M3DEG 075673u    /* -3DEG */
#define M0112A8 077750u  /* -.0112A8 */
#define P1AT4 00632u     /* .1AT4 */
#define P1AT2 01463u     /* .1AT2 */
#define P0375AT4 00232u  /* .0375AT4 */
#define M025AT2 077462u  /* -.025AT2 */
#define M025AT4 077631u  /* -.025AT4 */
#define M05AT2 077145u   /* -.05AT2 */
#define M15AT2 075462u   /* -.15AT2 */
#define MTJMAX 076631u   /* -TJMAX */
#define TJMIN 00122u
#define MTJMIN 077655u   /* -TJMIN */
#define RUFRATE 04476u
#define CCSHOLE 05705u
#define QREG 2u
/* The word at 17,3542 (TC Z123COMP), run as AD when TS FIREFCT skips the
 * EXTEND before BZMF ZON1,2,3. */
#define ZON123_WORD 03526u

/* A plain TC into fixed memory, as every return word is. */
static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
/* The signed value of an indexer (ones' complement; -0 adds nothing). */
static int ix(const agc_image *s,unsigned a){agc_word v=ai_word(s,a);return v&040000u?-(int)(~v&037777u):(int)v;}
static enum exec_status transfer(agc_image *s,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;return EXEC_OK;}
/* INDEX X; OP B (B in the EB window), and MP of A or L. The fast build
 * reaches the indexed cell directly (the indexers stay within a few words
 * of each base: AXISCTR is checked at entry, ADRSDIF1/2 are AXISDIFF plus
 * at most 3) and multiplies a register by value, as ai_mp does, on the
 * fast multiplier. */
#ifdef LM_TJ_GENERIC
static unsigned at(const agc_image *s,unsigned k,unsigned a){return (unsigned)((int)k+ix(s,a));}
#define XOP(n,b,x) ai_##n(s,at(s,b,x))
#define MPR(r) ai_mp(s,r)
#else
#define TJ_CELL(o) (&AFO_S->words[AFR_EBN*0400u+(o)])
static __attribute__((noinline)) afo_cell *xcell(uint8_t b,uint8_t x){
 agc_word v=(agc_word)(*TJ_CELL(x)&077777u);
 return TJ_CELL((uint8_t)(b+(v&040000u?(unsigned)-(int)(~v&037777u):(unsigned)v)));
}
#define XOP(n,b,x) afo_##n(xcell(AFW_OFF(b),AFW_OFF(x)))
#define MPR(r) afo_mpf((agc_word)((W[r]&037777u)|((W[r]>>1)&040000u)))
#endif

/* Z123COMP: CS ROTSENSE; TS ROTSENSE; CAE EDOT; EXTEND; INDEX ADRSDIF2;
 * MP 1/ANET1; TS TTOAXIS; AD -TJMAX; EXTEND; BZMF +2 (RETURN); TCF FULLTIME.
 * Entered by TC with return ret; 1 = returned, 0 = went to FULLTIME. */
static uint8_t z123comp(agc_image *s,agc_word ret){
 ai_tc(s,ret);ai_cs(s,ROTSENSE);(void)ai_ts(s,ROTSENSE);
 ai_ca(s,EDOT);XOP(mp,ANET1,ADRSDIF2);(void)ai_ts(s,TTOAXIS);ai_adf(s,MTJMAX);
 if(ai_bzmf(s)){LM_TJ_PATH(0);ai_tc(s,QREG+1u);return 1u;} /* RETURN: TC Q, Q = 3 */
 LM_TJ_PATH(1);return 0u;
}
/* RUFSETUP: CS ROTSENSE; TS ROTSENSE; +2 CAF FOUR; TS NUMBERT; CAF NEGMAX;
 * TS FIREFCT; TC Q. full = 0 enters at RUFSETUP +2. */
static void rufsetup(agc_image *s,agc_word ret,uint8_t full){
 ai_tc(s,ret);
 if(full){ai_cs(s,ROTSENSE);(void)ai_ts(s,ROTSENSE);}
 ai_caf(s,FOUR);(void)ai_ts(s,NUMBERT);ai_caf(s,NEGMAX);(void)ai_ts(s,FIREFCT);
 ai_tc(s,QREG+1u); /* TC Q: Z = ret, Q = 3 */
}

static enum exec_status tjetlaw(const agc_native_program *p,agc_image *s,void *io){agc_word a;uint8_t c;(void)p;(void)io;
 static const agc_word axisdiff[3]={077757u,0u,020u}; /* AXISDIFF -1, 0, +1: DEC -16, +0, 16 B-14 */
 static const agc_word p00375a8[2]={00007u,025605u};  /* .00375A8: 2DEC .00375 B-3 */
 AFR_BIND(s);
 if(!s||(W[4]&076000u)!=LM_TJ_FB||W[3]!=LM_TJ_EB||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 a=ai_word(s,AXISCTR);if(a!=NEGONE&&a!=0u&&a!=ONE)return EXEC_OUT_OF_SCOPE;
 /* EXTEND; QXCH HOLDQ; INDEX AXISCTR; CAF AXISDIFF; TS ADRSDIF1; CAE EDOT;
  * EXTEND; BZMF NEGEDOT. */
 ai_qxch(s,HOLDQ);ai_caf(s,axisdiff[ix(s,AXISCTR)+1]);(void)ai_ts(s,ADRSDIF1);
 ai_ca(s,EDOT);
 if(!ai_bzmf(s)){LM_TJ_PATH(2);ai_ca(s,ADRSDIF1);(void)ai_ts(s,ADRSDIF2);ai_caf(s,SENSOR);}
 else{ /* NEGEDOT */
  LM_TJ_PATH(3);ai_cs(s,E);(void)ai_ts(s,E);ai_cs(s,EDOT);(void)ai_ts(s,EDOT);
  ai_caf(s,ONE);ai_ads(s,ADRSDIF1);
  if(!ai_ts(s,ADRSDIF2))ai_csf(s,SENSOR);else LM_TJ_PATH(4);
 }
 /* SETSENSE: TS ROTSENSE; CAE E; EXTEND; MP BIT5; CCS A. */
 (void)ai_ts(s,ROTSENSE);ai_ca(s,E);ai_mpf(s,BIT5);
 c=ai_ccs(s,0u);LM_TJ_PATH(5u+c);
 if(c==0u)goto ruflaw2;
 if(c==2u)goto ruflaw1;
 /* SCALEE: CAF BIT13; EXTEND; MP L; TS E; CAE EDOT; EXTEND; MP BIT4;
  * EXTEND; BZF SCALEDOT; TCF RUFLAW3. */
 ai_caf(s,BIT13);MPR(1u);(void)ai_ts(s,E);ai_ca(s,EDOT);ai_mpf(s,BIT4);
 if(!ai_bzf(s)){LM_TJ_PATH(9);goto ruflaw3;}
 /* SCALEDOT: LXCH EDOT; CAE EDOT; EXTEND; SQUARE; EXTEND; MP BIT13;
  * TS EDOTSQ. */
 ai_lxch(s,EDOT);ai_ca(s,EDOT);MPR(0u);ai_mpf(s,BIT13);(void)ai_ts(s,EDOTSQ);
 /* ERRTEST: CCS E; AD -3DEG; TCF +2; AD -3DEG; EXTEND; INDEX ADRSDIF1;
  * SU FIREDB; EXTEND; BZMF SENSTEST. */
 c=ai_ccs(s,E);LM_TJ_PATH(10u+c);
 if(c==0u||c==2u)ai_adf(s,M3DEG);
 XOP(su,FIREDB,ADRSDIF1);
 if(ai_bzmf(s)){
  /* SENSTEST: CCS SENSETYP; TCF TJCALC; TCF MAXJETS; (TJCALC). */
  c=ai_ccs(s,SENSETYP);LM_TJ_PATH(14u+c);
  if(c==1u)goto maxjets;
  if(c==3u)goto tjcalc1; /* -0 lands past TS NUMBERT */
  goto tjcalc;
 }
maxjets: /* CAF TWO; ADS ADRSDIF2; CAF FOUR; TCF TJCALC */
 LM_TJ_PATH(18);ai_caf(s,TWO);ai_ads(s,ADRSDIF2);ai_caf(s,FOUR);
tjcalc: /* TS NUMBERT */
 (void)ai_ts(s,NUMBERT);
tjcalc1: /* CS EDOTSQ; EXTEND; INDEX ADRSDIF2; MP 1/ANET1; INDEX ADRSDIF1;
  * AD FIREDB; EXTEND; SU E; TS FIREFCT; EXTEND; BZMF ZON1,2,3. */
 ai_cs(s,EDOTSQ);XOP(mp,ANET1,ADRSDIF2);XOP(ad,FIREDB,ADRSDIF1);ai_su(s,E);
 if(ai_ts(s,FIREFCT)){LM_TJ_PATH(19);ai_adf(s,ZON123_WORD);} /* skips the EXTEND: AD 03542 */
 else if(ai_bzmf(s))goto zon123;
 /* ZONE4,5: INDEX ADRSDIF1; CAE 1/ACOAST; EXTEND; MP EDOTSQ; AD E;
  * INDEX ADRSDIF1; AD COASTDB; EXTEND; BZMF ZONE5. */
 XOP(ca,ACOAST,ADRSDIF1);ai_mp(s,EDOTSQ);ai_ad(s,E);XOP(ad,COASTDB,ADRSDIF1);
 if(ai_bzmf(s))goto zone5;
 /* ZONE4: INDEX AXISCTR; CS TJETU; EXTEND; MP ROTSENSE; EXTEND;
  * BZMF COASTTJ. */
 XOP(cs,TJETU,AXISCTR);ai_mp(s,ROTSENSE);
 if(ai_bzmf(s)){LM_TJ_PATH(20);goto coasttj;}
 /* JETSON: CCS FLAT; TCF DRIFT/ON; CS FIREFCT; INDEX ADRSDIF1;
  * AD AXISDIST; EXTEND; BZMF COASTTJ. */
 c=ai_ccs(s,FLAT);LM_TJ_PATH(21u+c);
 if(c==0u){
  /* DRIFT/ON: INDEX ADRSDIF1; CS FIREDB; DOUBLE; AD FIREFCT; EXTEND;
   * BZMF +3; COASTTJ ...; +3 TC Z123COMP; TCF ZONE2,3. */
  XOP(cs,FIREDB,ADRSDIF1);ai_double(s);ai_ad(s,FIREFCT);
  if(!ai_bzmf(s)){LM_TJ_PATH(25);goto coasttj;}
  if(!z123comp(s,03425u))goto fulltime;
  goto zone23;
 }
 if(c==1u)ai_cs(s,FIREFCT);
 if(c!=3u)XOP(ad,AXISDIST,ADRSDIF1);
 else ai_ad(s,AXISDIST); /* -0 lands past the INDEX: AD AXISDIST unindexed */
 if(ai_bzmf(s)){LM_TJ_PATH(26);goto coasttj;}
 /* TC Z123COMP; CAE FIREFCT; TCF ZONE1. */
 if(!z123comp(s,03412u))goto fulltime;
 ai_ca(s,FIREFCT);goto zone1;
coasttj: /* CAF ZERO; TCF RETURNTJ */
 ai_caf(s,ZERO);goto returntj;
zone5: /* TS L; CCS ROTSENSE; TCF +4; TC CCSHOLE; CS TWO; ADS ADRSDIF2 */
 if(ai_ts(s,1u))LM_TJ_PATH(27); /* skips the CCS: TCF +4 */
 else{c=ai_ccs(s,ROTSENSE);LM_TJ_PATH(28u+c);
  if(c==1u)return transfer(s,03432u,CCSHOLE);
  if(c==2u)ai_csf(s,TWO);
  if(c>=2u)ai_ads(s,ADRSDIF2);
 }
 /* +4: CAE L; EXTEND; INDEX ADRSDIF2; MP ACCFCTZ5; DDOUBL; DDOUBL;
  * DXCH HH; INDEX ADRSDIF2; CAE 1/ANET2; EXTEND; MP EDOT; TS TTOAXIS;
  * EXTEND; MP -.05AT2; AD HH; AD NEG2; EXTEND; BZMF FORMULA1. */
 ai_ca(s,1u);XOP(mp,ACCFCTZ5,ADRSDIF2);ai_ddoubl(s);ai_ddoubl(s);ai_dxch(s,HH);
 XOP(ca,ANET2,ADRSDIF2);ai_mp(s,EDOT);(void)ai_ts(s,TTOAXIS);
 ai_mpf(s,M05AT2);ai_ad(s,HH);ai_adf(s,NEG2);
 if(ai_bzmf(s)){
  /* FORMULA1: CS -.025AT4; AD TTOAXIS; DXCH HH; EXTEND; DV HH; EXTEND;
   * MP BIT14; TCF CHKMINTJ. */
  LM_TJ_PATH(32);ai_csf(s,M025AT4);ai_ad(s,TTOAXIS);ai_dxch(s,HH);ai_dv(s,HH);ai_mpf(s,BIT14);
  goto chkmintj;
 }
 /* CAE TTOAXIS; EXTEND; MP -.15AT2; AD HH; AD -.0112A8; EXTEND;
  * BZMF FORMULA2. */
 ai_ca(s,TTOAXIS);ai_mpf(s,M15AT2);ai_ad(s,HH);ai_adf(s,M0112A8);
 if(ai_bzmf(s)){
  /* FORMULA2: EXTEND; DCA .00375A8; DAS HH; CAE TTOAXIS; AD .1AT4;
   * DXCH HH; EXTEND; DV HH; EXTEND; MP BIT14; TCF RETURNTJ. */
  LM_TJ_PATH(33);ai_dcaf(s,p00375a8);ai_das(s,HH);ai_ca(s,TTOAXIS);ai_adf(s,P1AT4);ai_dxch(s,HH);
  ai_dv(s,HH);ai_mpf(s,BIT14);goto returntj;
 }
 LM_TJ_PATH(34);
fulltime: /* CAF BIT11 */
 ai_caf(s,BIT11);
returntj: /* EXTEND; MP ROTSENSE; INDEX AXISCTR; TS TJETU; EXTEND;
  * INDEX AXISCTR; MP ACCSWU; CAE L; EXTEND; BZMF +3; CAF FOUR;
  * TS NUMBERT; TC HOLDQ. */
 ai_mp(s,ROTSENSE);(void)XOP(ts,TJETU,AXISCTR);XOP(mp,ACCSWU,AXISCTR);ai_ca(s,1u);
 if(!ai_bzmf(s)){LM_TJ_PATH(35);ai_caf(s,FOUR);(void)ai_ts(s,NUMBERT);}else LM_TJ_PATH(36);
 /* TC HOLDQ: Z = the return word, Q = HOLDQ +1 */
 W[5]=ai_word(s,HOLDQ);ai_tc(s,01746u);return EXEC_OK;
zon123: /* TC Z123COMP; CAE FIREFCT; AD FLAT; EXTEND; BZMF ZONE1 */
 if(!z123comp(s,03543u))goto fulltime;
 ai_ca(s,FIREFCT);ai_ad(s,FLAT);
 if(ai_bzmf(s)){LM_TJ_PATH(37);goto zone1;}
zone23: /* CS ZONE3LIM; AD TTOAXIS; EXTEND; BZMF ZONE3 */
 ai_cs(s,ZONE3LIM);ai_ad(s,TTOAXIS);
 if(!ai_bzmf(s)){LM_TJ_PATH(38);ai_ca(s,TTOAXIS);goto returntj;} /* ZONE2 */
 /* ZONE3: CCS EDOT; CAF BIT6; TCF RETURNTJ; TC CCSHOLE; TCF RETURNTJ */
 c=ai_ccs(s,EDOT);LM_TJ_PATH(39u+c);
 if(c==0u)ai_caf(s,BIT6);
 if(c==2u)return transfer(s,03561u,CCSHOLE);
 goto returntj;
zone1: /* EXTEND; INDEX ADRSDIF1; SU AXISDIST; EXTEND; INDEX ADRSDIF2;
  * MP ACCFCTZ1; DDOUBL; DDOUBL; DXCH HH; CAE TTOAXIS; AD -TJMAX; EXTEND;
  * SQUARE; EXTEND; SU HH; EXTEND; BZMF FULLTIME. */
 XOP(su,AXISDIST,ADRSDIF1);XOP(mp,ACCFCTZ1,ADRSDIF2);ai_ddoubl(s);ai_ddoubl(s);ai_dxch(s,HH);
 ai_ca(s,TTOAXIS);ai_adf(s,MTJMAX);MPR(0u);ai_su(s,HH);
 if(ai_bzmf(s)){LM_TJ_PATH(43);goto fulltime;}
 /* CAE HH; AD NEG2; EXTEND; BZMF FORMULA3. */
 ai_ca(s,HH);ai_adf(s,NEG2);
 if(!ai_bzmf(s)){
  /* CAF .1AT2; DXCH HH; EXTEND; DV HH; AD TTOAXIS; AD .0375AT4;
   * TCF RETURNTJ. */
  LM_TJ_PATH(44);ai_caf(s,P1AT2);ai_dxch(s,HH);ai_dv(s,HH);ai_ad(s,TTOAXIS);ai_adf(s,P0375AT4);
  goto returntj;
 }
 /* FORMULA3: CS -.025AT2; DXCH HH; EXTEND; DV HH; AD TTOAXIS. */
 LM_TJ_PATH(45);ai_csf(s,M025AT2);ai_dxch(s,HH);ai_dv(s,HH);ai_ad(s,TTOAXIS);
chkmintj: /* AD -TJMIN; EXTEND; BZMF COASTTJ; AD TJMIN; TCF RETURNTJ */
 ai_adf(s,MTJMIN);
 if(ai_bzmf(s)){LM_TJ_PATH(46);goto coasttj;}
 LM_TJ_PATH(47);ai_adf(s,TJMIN);goto returntj;
ruflaw1: /* CS RUFRATE; ADS EDOT; EXTEND; BZMF SMALRATE; TC RUFSETUP;
  * CAE EDOT */
 ai_csf(s,RUFRATE);ai_ads(s,EDOT);
 if(ai_bzmf(s)){
  /* SMALRATE: TC RUFSETUP +2; CCS ROTSENSE; CAF ONE; TCF +2; CAF NEGONE;
   * ADS ADRSDIF2; CS EDOT; TCF RUFLAW12. */
  rufsetup(s,03655u,0u);
  c=ai_ccs(s,ROTSENSE);LM_TJ_PATH(48u+c);
  if(c==0u)ai_caf(s,ONE);
  if(c==2u)ai_caf(s,NEGONE);
  ai_ads(s,ADRSDIF2);ai_cs(s,EDOT);goto ruflaw12;
 }
 LM_TJ_PATH(52);rufsetup(s,03635u,1u);ai_ca(s,EDOT);
ruflaw12: /* EXTEND; INDEX ADRSDIF2; MP 1/ANET1 +2; AD -1/8; EXTEND;
  * BZMF +2; TCF FULLTIME; EXTEND; BZF FULLTIME; AD BIT12; DAS A (three
  * times); TCF CHKMINTJ. */
 XOP(mp,ANET1_2,ADRSDIF2);ai_adf(s,NEG1_8);
 if(!ai_bzmf(s)){LM_TJ_PATH(53);goto fulltime;}
 if(ai_bzf(s)){LM_TJ_PATH(54);goto fulltime;}
 LM_TJ_PATH(55);ai_adf(s,BIT12);ai_das(s,0u);ai_das(s,0u);ai_das(s,0u);goto chkmintj;
ruflaw2: /* TC RUFSETUP; CAF RUFRATE; AD EDOT; TS A; TCF RUFLAW12;
  * TCF FULLTIME */
 rufsetup(s,03665u,1u);ai_caf(s,RUFRATE);ai_ad(s,EDOT);
 if(ai_ts(s,0u)){LM_TJ_PATH(56);goto fulltime;}
 LM_TJ_PATH(57);goto ruflaw12;
ruflaw3: /* TC RUFSETUP; INDEX ADRSDIF1; CS FIREDB; AD E; EXTEND; MP BIT11;
  * XCH EDOT; EXTEND; SQUARE; EXTEND; INDEX ADRSDIF1; MP 1/ANET1 +2;
  * AD EDOT; EXTEND; BZMF COASTTJ; TCF FULLTIME. */
 rufsetup(s,03673u,1u);XOP(cs,FIREDB,ADRSDIF1);ai_ad(s,E);ai_mpf(s,BIT11);ai_xch(s,EDOT);
 MPR(0u);XOP(mp,ANET1_2,ADRSDIF1);ai_ad(s,EDOT);
 if(ai_bzmf(s)){LM_TJ_PATH(58);goto coasttj;}
 LM_TJ_PATH(59);goto fulltime;
}
static const agc_routine_entry entries[1]={{P(03252u),tjetlaw}};
const agc_routine_table lm_tjet_law_routines={entries,1};
