/* SPDX-License-Identifier: MIT
 * Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM.agc, all of it (bank 21,
 * 03077-03645; bank 16, 03735-03771), as routines of the control-transfer
 * ABI. Each source instruction is one agc_image_ops call (yaAGC-qualified
 * register effects) or, for DTCB and the channel orders, a helper here with
 * yaAGC's effect, named with its erasable address, channel or the source's
 * fixed constant. TS overflow skips and CCS exits land where the
 * original's do, including the ones no admitted input reaches. */
#include "lm_trim_gimbal.h"
#include "agc_image_ops.h"
/* Every instruction runs on the one canonical image through
 * include/agc_fast_redirect.h: a plain-cell operand under this file's
 * EBANK 6 goes to the fast helper (include/agc_fast_ops.h), any other
 * operand to a one-argument wrapper of the agc_image_ops helper; both have
 * its effects. -DLM_TG_GENERIC builds the same source on agc_image_ops alone. */
#ifdef LM_TG_GENERIC
#define AFR_GENERIC
#endif
#define AFR_EBN (LM_TG_EB >> 8)
#include "agc_fast_redirect.h"
#define W AFR_S(s)->words
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_TG_COUNT_PATHS
extern unsigned lm_tg_paths[];
#define LM_TG_PATH(n) ((void)++lm_tg_paths[n])
#else
#define LM_TG_PATH(n) ((void)0)
#endif
#define P16(z) (uint16_t)(016u*1024u+((z)-02000u))
#define P21(z) (uint16_t)(021u*1024u+((z)-02000u))
/* Erasable: unswitched, and EBANK6 (EBANK= QDIFF, NEGUQ). */
#define LREG 1u
#define QREG 2u
#define ZEROREG 7u
#define SR 021u
#define ITEMP2 062u
#define NZACCDOT 063u   /* = ITEMP3 */
#define CHNL12 066u     /* = ITEMP6 */
#define QRNDXER 066u    /* = ITEMP6 */
#define RUPTREG2 071u
#define RUPTREG3 072u
#define RCSFLAGS 01273u
#define EDOTQ 01436u
#define QDIFF 01450u
#define NEGUQ 01501u
#define ALLOWGTS 01502u
#define NEGUR 01503u
#define KQ 01504u
#define SAVESR 01505u
#define ACCDOTQ 01507u
#define QACCDOT 01510u
#define ACCDOTR 01511u
#define RACCDOT 01512u
#define SKIPU 01535u
#define SKIPV 01536u
#define AOSQ 01537u
#define COTROLER 01631u
#define QGIMTIMR 01632u
#define INGTS 01633u
#define RGIMTIMR 01634u
#define K2THETA 01737u
#define K2THETA1 01740u  /* K2THETA +1 */
#define KCENTRAL 01741u
#define SHFTFLAG 01741u
#define K2CNTRAL 01742u
#define K2CNTRAL1 01743u /* K2CNTRAL +1 */
#define WCENTRAL 01743u
#define ACENTRAL 01744u
#define ININDEX 01744u
#define DEL 01745u
#define A2CNTRAL 01746u
#define SCRATCH 01746u
#define A2CNTRAL1 01747u /* A2CNTRAL +1 */
#define HALFARG 01747u
#define QRCNTR 01750u
#define FUNCTION 01751u
#define FUNCTION1 01752u /* FUNCTION +1 */
/* Fixed constants: NEGONE 07747, FOUR 04751, TWO = BIT2 04752, BIT5 =
 * CALLGMBL 04747, BIT14 04736, POSMAX 04733, BIT9 04743, BIT1 = ONE 04753,
 * ZERO 04755, BIT10 04742, BIT13 04737, BIT12 04740, BIT11 04741, OCT31
 * 06010, SEVEN 04757, TWELVE = OCT14 05751, NEG2 07746; LIMITS -1/0/+1
 * (04733-04735) and BIT15-BIT2 (04735-04752) under INDEX; this file's
 * NEG1/3 (21,3243), OCT23146 (21,3327), OCT00240 (21,3440), CLOSEADR
 * (21,3630), ROOTHALF (21,3632), GMBLBITA/GMBLBITB with their neighbours
 * (21,3632-3635) under INDEX QRCNTR, and BGIM (16,3735). */
#define NEGONE 077776u
#define FOUR 4u
#define TWO 2u
#define BIT2 2u
#define BIT5 020u
#define CALLGMBL 020u
#define BIT14 020000u
#define POSMAX 037777u
#define BIT9 0400u
#define BIT1 1u
#define ONE 1u
#define ZERO 0u
#define BIT10 01000u
#define BIT13 010000u
#define BIT12 04000u
#define BIT11 02000u
#define OCT31 031u
#define SEVEN 7u
#define TWELVE 014u
#define NEG2 077775u
#define NEG1_3 065252u
#define OCT23146 023146u
#define OCT00240 0240u
#define ROOTHALF 026501u
#define OCT11276 011276u
#define BGIM 07400u

static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
static int entry(const agc_image *s, const void *io, agc_word fb)
{
    AFR_BIND(s);
    return s && io && (W[4] & 076000u) == fb && W[3] == LM_TG_EB;
}
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
/* The signed value of an erasable word, and of the 16-bit A. */
static int sv(const agc_image *s,unsigned k){agc_word v=ai_word(s,k);return v&040000u?-(int)(~v&037777u):(int)v;}
/* INDEX k; CA BIT15 (k 0-13: BIT15 .. BIT2). */
static agc_word bit15(int k){static const agc_word t[14]={040000u,020000u,010000u,04000u,02000u,01000u,0400u,0200u,0100u,040u,020u,010u,4u,2u};
 return t[k];}
/* Base + a signed index (INDEX). */
static unsigned at(unsigned base,int k){return (unsigned)((int)base+k);}
static int sa(const agc_image *s){agc_register a=W[0];return a&0100000u?-(int)(~a&077777u):(int)(a&077777u);}
/* Channel orders on channel 12, as yaAGC's: RAND, WAND, WRITE. */
static void rand(agc_image *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
static void wand(agc_image *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])&*chan);*chan=v;W[0]=agc_extend(v);}
static void wr(agc_image *s,agc_word *chan){*chan=(agc_word)(oc(W[0])&077777u);}
/* DTCB (DXCH Z) executed at `at`: Z,BB <-> A,L. */
static void dtcb(agc_image *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=z;
 W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);
}

/* The words a GTS pass may write, restored when DOSHIFT refuses: A-BB,
 * SR, channel 12 and the EBANK6 cells (NEGUQ and QACCDOT under INDEX
 * QRCNTR -1 .. +2). */
static const agc_word gts_cells[]={0u,1u,2u,3u,4u,5u,6u,SR,RCSFLAGS,NEGUQ-1u,NEGUQ,ALLOWGTS,NEGUR,SAVESR,
 ACCDOTQ,QACCDOT,ACCDOTR,RACCDOT,SKIPU,SKIPV,COTROLER,QGIMTIMR,INGTS,RGIMTIMR,K2THETA,K2THETA1,KCENTRAL,K2CNTRAL,
 K2CNTRAL1,ACENTRAL,DEL,A2CNTRAL,A2CNTRAL1,QRCNTR,FUNCTION,FUNCTION1};
#define GTS_CELLS (sizeof gts_cells/sizeof gts_cells[0])

/* ROOTCYCL, entered by TC from ret - 1: TS SCRATCH; TS SR; CA HALFARG; ZL;
 * EXTEND; DV SCRATCH; AD SR; TC Q. */
static void rootcycl(agc_image *s,agc_word ret){
 ai_tc(s,ret);
 if(ai_ts(s,SCRATCH))LM_TG_PATH(43);
 else (void)ai_ts(s,SR);
 ai_ca(s,HALFARG);ai_zl(s);ai_dv(s,SCRATCH);ai_ad(s,SR);
 ai_tc(s,QREG+1u); /* TC Q: Z = ret, Q = 3 */
}
/* GTSQRT, entered by TC GTSQRT from RSTOFGTS; returns at PRODUCT with Q as
 * the original leaves it (3 after ZEROOT's TC Q, FUNCTION +2 after
 * TC FUNCTION +1, whose word is the TC PRODUCT that SCALDONE's QXCH saved).
 * Returns 1 when the zero root was taken (ININDEX is then not set). */
static int gtsqrt(agc_image *s){uint8_t c;int k;
 ai_tc(s,03525u);
 c=ai_ccs(s,FUNCTION);LM_TG_PATH(31u+c);
 if(c==2u)goto zeroot;
 if(c==0u)goto goodarg;
 ai_ca(s,FUNCTION1);
 if(ai_bzmf(s)){LM_TG_PATH(35);goto zeroot;}
 /* ZEROHIGH: XCH FUNCTION; XCH FUNCTION +1; CA SEVEN; TCF GOODARG +1. */
 LM_TG_PATH(36);ai_xch(s,FUNCTION);ai_xch(s,FUNCTION1);ai_caf(s,SEVEN);goto goodarg1;
zeroot: /* CA ZERO; TS SHFTFLAG; TC Q. */
 ai_caf(s,ZERO);(void)ai_ts(s,SHFTFLAG);ai_tc(s,QREG+1u);return 1;
goodarg: /* CA ZERO; TS SHFTFLAG; CA TWELVE; TS ININDEX; TCF SCALLOOP. */
 ai_caf(s,ZERO);
goodarg1:
 (void)ai_ts(s,SHFTFLAG);ai_caf(s,TWELVE);(void)ai_ts(s,ININDEX);
 for(;;){
  /* SCALLOOP: CS FUNCTION; INDEX ININDEX; AD BIT15; EXTEND; BZMF MULBUSH. */
  ai_cs(s,FUNCTION);k=sv(s,ININDEX);ai_adf(s,bit15(k));
  if(!ai_bzmf(s))break;
  /* MULBUSH: CA NEG2; ADS ININDEX; EXTEND; BZMF SCALSTRT. */
  LM_TG_PATH(37);ai_caf(s,NEG2);ai_ads(s,ININDEX);
  if(ai_bzmf(s)){LM_TG_PATH(38);ai_ca(s,FUNCTION);goto scaldone;} /* SCALSTRT: CA FUNCTION; TCF SCALDONE */
 }
 /* INDEX ININDEX; CA BIT15; XCH HALFARG; EXTEND; DCA FUNCTION; EXTEND;
  * DV HALFARG. */
 k=sv(s,ININDEX);ai_caf(s,bit15(k));ai_xch(s,HALFARG);ai_dca(s,FUNCTION);ai_dv(s,HALFARG);
scaldone: /* EXTEND; QXCH FUNCTION +1; EXTEND; MP BIT14; TS HALFARG;
  * MASK BIT13; CCS A; CA OCT11276; AD ROOTHALF; TC ROOTCYCL (three);
  * TC FUNCTION +1. */
 ai_qxch(s,FUNCTION1);ai_mpf(s,BIT14);(void)ai_ts(s,HALFARG);ai_maskf(s,BIT13);
 c=ai_ccs(s,0u);LM_TG_PATH(39u+c);
 if(c==0u)ai_caf(s,OCT11276);
 if(c<=1u)ai_adf(s,ROOTHALF);
 if(c<=2u)rootcycl(s,03521u);
 rootcycl(s,03522u);rootcycl(s,03523u);
 ai_tc(s,03524u);ai_tc(s,FUNCTION1+1u); /* TC FUNCTION +1; its word, TC PRODUCT */
 return 0;
}
/* GTS .. GOCLOSE. */
static enum exec_status gts(const agc_native_program *p,agc_image *s,void *v){if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;p66_bias_io *io=v;uint8_t c;int qr,k;
 static const agc_word closeadr[2]={03236u,036106u}; /* 2CADR CLOSEOUT */
 static const agc_word limits[3]={037777u,057777u,040000u}; /* LIMITS -1, 0, +1 */
 static const agc_word gmblbita[4]={026501u,01400u,011276u,06000u}; /* GMBLBITA -1 (ROOTHALF), GMBLBITA, +1 (OCT11276), +2 (GMBLBITB) */
 agc_word saved[GTS_CELLS],chan12;unsigned i;
 if(!entry(s,io,LM_TG_FB21))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<GTS_CELLS;++i)saved[i]=W[ai_ea(s,gts_cells[i])];
 chan12=lmf_hw->channel12;
 /* GTS: CAF NEGONE; TS COTROLER; CAF FOUR; TS SKIPU; TS SKIPV; CAF TWO;
  * TS INGTS; TS QGIMTIMR; TS RGIMTIMR; CA SR; AD A; TS SAVESR;
  * GTSGO+ON CAF TWO; TCF GOQTRIMG +1. */
 ai_caf(s,NEGONE);(void)ai_ts(s,COTROLER);ai_caf(s,FOUR);(void)ai_ts(s,SKIPU);(void)ai_ts(s,SKIPV);
 ai_caf(s,TWO);(void)ai_ts(s,INGTS);(void)ai_ts(s,QGIMTIMR);(void)ai_ts(s,RGIMTIMR);
 ai_ca(s,SR);ai_double(s);
 if(ai_ts(s,SAVESR))LM_TG_PATH(0); /* skips GTSGO+ON: TS QRCNTR takes +-1 */
 else ai_caf(s,TWO);
 for(;;){
  /* GOQTRIMG +1: TS QRCNTR; INDEX QRCNTR; CA AOSQ; EXTEND; MP BIT2; EXTEND;
   * BZF GTSQAXIS -3; INDEX A; CS LIMITS; XCH L. */
  (void)ai_ts(s,QRCNTR);qr=sv(s,QRCNTR);
  ai_ca(s,at(AOSQ,qr));ai_mpf(s,BIT2);
  if(ai_bzf(s))LM_TG_PATH(1);
  else{LM_TG_PATH(2);ai_csf(s,limits[sa(s)+1]);ai_xch(s,LREG);}
  /* CCS QRCNTR; INDEX A; CA EDOTQ; GTSQAXIS DXCH WCENTRAL. */
  c=ai_ccs(s,QRCNTR);LM_TG_PATH(3u+c);
  if(c==0u)ai_ca(s,at(EDOTQ,sa(s)));
  if(c==1u)ai_ca(s,EDOTQ);
  if(c<=2u)ai_dxch(s,WCENTRAL);
  /* INDEX QRCNTR; CA KQ; TS KCENTRAL; EXTEND; BZF POSDRIVE +1. */
  ai_ca(s,at(KQ,qr));(void)ai_ts(s,KCENTRAL);
  if(ai_bzf(s)){LM_TG_PATH(7);goto posdrive1;}
  /* INDEX QRCNTR; CAE QDIFF; ALGORTHM: EXTEND; MP KCENTRAL; LXCH K2THETA;
   * EXTEND; MP BIT5; DXCH K2THETA; EXTEND; MP BIT5; ADS K2THETA +1. */
  ai_ca(s,at(QDIFF,qr));ai_mp(s,KCENTRAL);ai_lxch(s,K2THETA);ai_mpf(s,BIT5);ai_dxch(s,K2THETA);
  ai_mpf(s,BIT5);ai_ads(s,K2THETA1);
  /* CS ACENTRAL; EXTEND; MP BIT14; EXTEND; MP ACENTRAL; AD KCENTRAL;
   * EXTEND; BZMF HUGEQUOT. */
  ai_cs(s,ACENTRAL);ai_mpf(s,BIT14);ai_mp(s,ACENTRAL);ai_ad(s,KCENTRAL);
  if(ai_bzmf(s)){
   /* HUGEQUOT: CA POSMAX; TS L; DXCH A2CNTRAL. */
   LM_TG_PATH(8);ai_caf(s,POSMAX);(void)ai_ts(s,LREG);ai_dxch(s,A2CNTRAL);
  }else{
   /* EXTEND; DCS A; AD KCENTRAL; EXTEND; DV KCENTRAL; XCH A2CNTRAL; CA L;
    * LXCH 7; EXTEND; DV KCENTRAL; XCH A2CNTRAL +1; TCF HAVEQUOT. */
   LM_TG_PATH(9);ai_dcom(s);ai_ad(s,KCENTRAL);ai_dv(s,KCENTRAL);ai_xch(s,A2CNTRAL);ai_ca(s,LREG);
   ai_lxch(s,ZEROREG);ai_dv(s,KCENTRAL);ai_xch(s,A2CNTRAL1);
  }
  /* HAVEQUOT: CA WCENTRAL; EXTEND; MP BIT9; DXCH K2CNTRAL; EXTEND;
   * DCA K2CNTRAL; DXCH FUNCTION; CA ACENTRAL; EXTEND; BZMF +4; EXTEND;
   * DCA A2CNTRAL; TCF +3; EXTEND; DCS A2CNTRAL; DAS FUNCTION. */
  ai_ca(s,WCENTRAL);ai_mpf(s,BIT9);ai_dxch(s,K2CNTRAL);ai_dca(s,K2CNTRAL);ai_dxch(s,FUNCTION);
  ai_ca(s,ACENTRAL);
  if(ai_bzmf(s)){LM_TG_PATH(10);ai_dcs(s,A2CNTRAL);}
  else{LM_TG_PATH(11);ai_dca(s,A2CNTRAL);}
  ai_das(s,FUNCTION);
  /* CCS FUNCTION; TCF POSFNCT1; TCF +2; TCF NEGFNCT1; CCS FUNCTION +1;
   * POSFNCT1 CAF BIT1; TCF +2; NEGFNCT1 CS BIT1; TS DEL. */
  c=ai_ccs(s,FUNCTION);LM_TG_PATH(12u+c);
  if(c==1u||c==3u){c=ai_ccs(s,FUNCTION1);LM_TG_PATH(16u+c);if(c==1u||c==3u)c=4u;}
  if(c==0u)ai_caf(s,BIT1);
  if(c==2u)ai_csf(s,BIT1);
  (void)ai_ts(s,DEL);
  /* CCS DEL; TCF FUNCT2; TCF DEFUNCT; TCF NEGFNCT2; DEFUNCT TS K2CNTRAL;
   * TS K2CNTRAL +1; TCF FUNCT2; NEGFNCT2 EXTEND; DCS K2CNTRAL;
   * DXCH K2CNTRAL. */
  c=ai_ccs(s,DEL);LM_TG_PATH(20u+c);
  if(c==1u||c==3u){(void)ai_ts(s,K2CNTRAL);(void)ai_ts(s,K2CNTRAL1);}
  if(c==2u){ai_dcs(s,K2CNTRAL);ai_dxch(s,K2CNTRAL);}
  /* FUNCT2: EXTEND; DCA A2CNTRAL; DAS K2CNTRAL. FUNCT3: CA A2CNTRAL;
   * EXTEND; MP NEG1/3; DXCH A2CNTRAL; CA L; EXTEND; MP NEG1/3;
   * ADS A2CNTRAL +1; TS L; TCF +2; ADS A2CNTRAL. */
  ai_dca(s,A2CNTRAL);ai_das(s,K2CNTRAL);
  ai_ca(s,A2CNTRAL);ai_mpf(s,NEG1_3);ai_dxch(s,A2CNTRAL);ai_ca(s,LREG);ai_mpf(s,NEG1_3);ai_ads(s,A2CNTRAL1);
  if(ai_ts(s,LREG)){LM_TG_PATH(24);ai_ads(s,A2CNTRAL);}
  /* EXTEND; DCA K2CNTRAL; DAS A2CNTRAL; CA A2CNTRAL; EXTEND; MP ACENTRAL;
   * DAS K2THETA; CA A2CNTRAL +1; EXTEND; MP ACENTRAL; ADS K2THETA +1;
   * TS L; TCF +2; ADS K2THETA. */
  ai_dca(s,K2CNTRAL);ai_das(s,A2CNTRAL);ai_ca(s,A2CNTRAL);ai_mp(s,ACENTRAL);ai_das(s,K2THETA);
  ai_ca(s,A2CNTRAL1);ai_mp(s,ACENTRAL);ai_ads(s,K2THETA1);
  if(ai_ts(s,LREG)){LM_TG_PATH(25);ai_ads(s,K2THETA);}
  /* GETROOT: CA K2CNTRAL; EXTEND; MP KCENTRAL; DXCH FUNCTION;
   * CA K2CNTRAL +1; EXTEND; MP KCENTRAL; ADS FUNCTION +1; TS L; TCF +2;
   * ADS FUNCTION. */
  ai_ca(s,K2CNTRAL);ai_mp(s,KCENTRAL);ai_dxch(s,FUNCTION);ai_ca(s,K2CNTRAL1);ai_mp(s,KCENTRAL);ai_ads(s,FUNCTION1);
  if(ai_ts(s,LREG)){LM_TG_PATH(26);ai_ads(s,FUNCTION);}
  /* CCS DEL; TCF RSTOFGTS; TCF NEGUSUM; TCF NEGATE; TCF NEGUSUM;
   * NEGATE EXTEND; DCS K2CNTRAL; DXCH K2CNTRAL; TCF RSTOFGTS. */
  c=ai_ccs(s,DEL);LM_TG_PATH(27u+c);
  if(c==1u||c==3u)goto negusum;
  if(c==2u){ai_dcs(s,K2CNTRAL);ai_dxch(s,K2CNTRAL);}
  /* RSTOFGTS: TC GTSQRT. PRODUCT: XCH K2CNTRAL; EXTEND; MP K2CNTRAL;
   * DXCH K2CNTRAL; EXTEND; MP L; ADS K2CNTRAL +1; TS L; TCF +2;
   * ADS K2CNTRAL. */
  (void)gtsqrt(s);
  ai_xch(s,K2CNTRAL);ai_mp(s,K2CNTRAL);ai_dxch(s,K2CNTRAL);ai_mp(s,LREG);ai_ads(s,K2CNTRAL1);
  if(ai_ts(s,LREG)){LM_TG_PATH(44);ai_ads(s,K2CNTRAL);}
  /* DOSHIFT: CA ININDEX; EXTEND; MP BIT14; ADS SHFTFLAG; EXTEND;
   * BZF ADDITIN; INDEX SHFTFLAG; CA BIT15; XCH K2CNTRAL; EXTEND;
   * MP K2CNTRAL; DAS K2THETA; XCH K2CNTRAL; EXTEND; MP K2CNTRAL +1;
   * ADS K2THETA +1; TS L; TCF +2; ADS K2THETA; TCF NEGUSUM. */
  ai_ca(s,ININDEX);ai_mpf(s,BIT14);ai_ads(s,SHFTFLAG);
  if(ai_bzf(s)){
   /* ADDITIN: EXTEND; DCA K2CNTRAL; DAS K2THETA. */
   LM_TG_PATH(45);ai_dca(s,K2CNTRAL);ai_das(s,K2THETA);
  }else{
   k=sv(s,SHFTFLAG);
   if(k<0||k>13){ /* only after the zero root: ININDEX is ACENTRAL, so
    * SHFTFLAG is ACENTRAL/2 and INDEX SHFTFLAG makes CA BIT15 (34735) the
    * word 34735 + k. While that stays a CA (k -2525 .. +1570), it reads
    * 04735 + k: erasable memory (CA Z reads 21,3547, the next address),
    * bank 21 or fixed-fixed, from the generated table. Beyond that the
    * word is another order, which is refused. */
    int a=04735+k;agc_word w=0;
    if(a<0||a>07777){
     LM_TG_PATH(64);
     goto refuse;
    }
    if(a<02000){LM_TG_PATH(85);if(a==5)W[5]=03547u;ai_ca(s,(unsigned)a);}
    else{
     /* The fixed word, read through the program's passive fixed_data
      * (the rope); refused without it. */
     LM_TG_PATH(86);
     if(!p->fixed_data||!p->fixed_data(p,agc_fixed_physical((agc_word)a,LM_TG_FB21,0u),&w))goto refuse;
     ai_caf(s,w);
    }
   }else{LM_TG_PATH(46);ai_caf(s,bit15(k));}
   ai_xch(s,K2CNTRAL);ai_mp(s,K2CNTRAL);ai_das(s,K2THETA);ai_xch(s,K2CNTRAL);
   ai_mp(s,K2CNTRAL1);ai_ads(s,K2THETA1);
   if(ai_ts(s,LREG)){LM_TG_PATH(47);ai_ads(s,K2THETA);}
  }
negusum: /* NEGUSUM: CCS K2THETA; TCF NEGDRIVE; TCF +2; TCF POSDRIVE;
  * CCS K2THETA +1; NEGDRIVE CA BIT1; TCF +2; POSDRIVE CS BIT1. */
  c=ai_ccs(s,K2THETA);LM_TG_PATH(48u+c);
  if(c==1u||c==3u){c=ai_ccs(s,K2THETA1);LM_TG_PATH(52u+c);if(c==1u||c==3u)c=4u;}
  if(c==0u)ai_caf(s,BIT1);
  if(c==2u)ai_csf(s,BIT1);
posdrive1: /* TS L; INDEX QRCNTR; XCH NEGUQ; EXTEND; MP L; CCS L; TCF LOUPE;
  * TCF ZEROLOUP; TCF REVERSAL; TCF ZEROLOUP. */
  (void)ai_ts(s,LREG);qr=sv(s,QRCNTR);ai_xch(s,at(NEGUQ,qr));ai_mp(s,LREG);
  c=ai_ccs(s,LREG);LM_TG_PATH(56u+c);
  if(c==2u){
   /* REVERSAL: INDEX QRCNTR; TS QACCDOT; INDEX QRCNTR; CS GMBLBITA;
    * EXTEND; WAND CHAN12. */
   (void)ai_ts(s,at(QACCDOT,qr));ai_csf(s,gmblbita[qr+1]);wand(s,&lmf_hw->channel12);
  }
  if(c!=0u){
   /* ZEROLOUP: CS RCSFLAGS; MASK CALLGMBL; ADS RCSFLAGS. */
   ai_cs(s,RCSFLAGS);ai_maskf(s,CALLGMBL);ai_ads(s,RCSFLAGS);
  }
  /* LOUPE: CCS QRCNTR; TCF GOQTRIMG; CA SAVESR; TS SR; GOCLOSE. */
  c=ai_ccs(s,QRCNTR);LM_TG_PATH(60u+c);
  if(c!=0u)break;
  ai_caf(s,ZERO); /* GOQTRIMG */
 }
 if(c==1u)ai_ca(s,SAVESR);
 if(c<=2u)(void)ai_ts(s,SR);
 /* GOCLOSE: EXTEND; DCA CLOSEADR; DTCB. */
 ai_dcaf(s,closeadr);dtcb(s,03627u);
 return EXEC_OK;
refuse:
 for(i=0;i<GTS_CELLS;++i)W[ai_ea(s,gts_cells[i])]=saved[i];
 lmf_hw->channel12=chan12;return EXEC_OUT_OF_SCOPE;
}
/* ACDT+C12: CS NEGUQ; EXTEND; MP ACCDOTQ; LXCH QACCDOT; CS NEGUR; EXTEND;
 * MP ACCDOTR; LXCH RACCDOT; CCS NEGUQ; CAF BIT10; TCF +2; CAF BIT9;
 * TS CHNL12; CCS NEGUR; CAF BIT12; TCF +2; CAF BIT11; ADS CHNL12; CS BGIM;
 * EXTEND; RAND CHAN12; AD CHNL12; EXTEND; WRITE CHAN12; CS CALLGMBL;
 * MASK RCSFLAGS; TS RCSFLAGS; TC Q. */
static enum exec_status acdtc12(const agc_native_program *p,agc_image *s,void *v){if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;p66_bias_io *io=v;uint8_t c;(void)p;
 if(!entry(s,io,LM_TG_FB16)||!tc_word(W[QREG]))return EXEC_OUT_OF_SCOPE;
 ai_cs(s,NEGUQ);ai_mp(s,ACCDOTQ);ai_lxch(s,QACCDOT);ai_cs(s,NEGUR);ai_mp(s,ACCDOTR);ai_lxch(s,RACCDOT);
 c=ai_ccs(s,NEGUQ);LM_TG_PATH(65u+c);
 if(c==0u)ai_caf(s,BIT10);
 if(c==2u)ai_caf(s,BIT9);
 (void)ai_ts(s,CHNL12);
 c=ai_ccs(s,NEGUR);LM_TG_PATH(69u+c);
 if(c==0u)ai_caf(s,BIT12);
 if(c==2u)ai_caf(s,BIT11);
 ai_ads(s,CHNL12);
 ai_csf(s,BGIM);rand(s,lmf_hw->channel12);ai_ad(s,CHNL12);wr(s,&lmf_hw->channel12);
 ai_csf(s,CALLGMBL);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);
 W[5]=W[QREG];ai_tc(s,QREG+1u); /* TC Q: Z = Q, Q = 3 */
 return EXEC_OK;
}
/* TIMEGMBL .. TC IBNKCALL; CADR ACDT+C12. */
static enum exec_status timegmbl(const agc_native_program *p,agc_image *s,void *io){if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;uint8_t c;int k;(void)p;
 if(!entry(s,io,LM_TG_FB21))return EXEC_OUT_OF_SCOPE;
 /* TIMEGMBL: CAF ONE; TS ALLOWGTS; CAF TWO; LXCH Q; LXCH RUPTREG2;
  * TCF +2. */
 ai_caf(s,ONE);(void)ai_ts(s,ALLOWGTS);ai_caf(s,TWO);ai_lxch(s,QREG);ai_lxch(s,RUPTREG2);
 for(;;){
  /* TS QRNDXER; INDEX QRNDXER; CA ACCDOTQ; EXTEND; BZMF TGOFFNOW;
   * TS NZACCDOT. */
  (void)ai_ts(s,QRNDXER);k=sv(s,QRNDXER);
  ai_ca(s,at(ACCDOTQ,k));
  if(ai_bzmf(s)){LM_TG_PATH(73);goto tgoffnow;}
  (void)ai_ts(s,NZACCDOT);
  /* ALPHATRY: INDEX QRNDXER; CS AOSQ; EXTEND; BZF TGOFFNOW; TS Q; EXTEND;
   * MP OCT23146; AD Q; TS L; TCF SETNEGU. */
  ai_cs(s,at(AOSQ,k));
  if(ai_bzf(s)){LM_TG_PATH(74);goto tgoffnow;}
  (void)ai_ts(s,QREG);ai_mpf(s,OCT23146);ai_ad(s,QREG);
  if(ai_ts(s,LREG)){
   /* CS A; INDEX QRNDXER; XCH NEGUQ; TCF NOTALLOW. */
   LM_TG_PATH(75);ai_com(s);ai_xch(s,at(NEGUQ,k));goto notallow;
  }
  /* SETNEGU: EXTEND; BZMF POSALPH; COM; TS ITEMP2; CS BIT1; TCF POSALPH +2;
   * POSALPH TS ITEMP2; CA BIT1; +2 INDEX QRNDXER; TS NEGUQ. */
  if(ai_bzmf(s)){LM_TG_PATH(76);(void)ai_ts(s,ITEMP2);ai_caf(s,BIT1);}
  else{LM_TG_PATH(77);ai_com(s);(void)ai_ts(s,ITEMP2);ai_csf(s,BIT1);}
  (void)ai_ts(s,at(NEGUQ,k));
  /* CA NZACCDOT; EXTEND; MP BIT12; AD ITEMP2; EXTEND; BZMF NOTALLOW;
   * CS ITEMP2; EXTEND; MP OCT00240; EXTEND; DV NZACCDOT; EXTEND;
   * BZF TGOFFNOW; TCF DRIVEON. */
  ai_ca(s,NZACCDOT);ai_mpf(s,BIT12);ai_ad(s,ITEMP2);
  if(ai_bzmf(s)){LM_TG_PATH(78);goto notallow;}
  ai_cs(s,ITEMP2);ai_mpf(s,OCT00240);ai_dv(s,NZACCDOT);
  if(ai_bzf(s)){LM_TG_PATH(79);goto tgoffnow;}
  /* DRIVEON: INDEX QRNDXER; TS QGIMTIMR. */
  LM_TG_PATH(80);(void)ai_ts(s,at(QGIMTIMR,k));goto doneyet;
tgoffnow: /* CAF ZERO; INDEX QRNDXER; TS NEGUQ; TCF DONEYET. */
  ai_caf(s,ZERO);(void)ai_ts(s,at(NEGUQ,k));goto doneyet;
notallow: /* CAF OCT31; INDEX QRNDXER; TS QGIMTIMR; CAF ZERO; TS ALLOWGTS. */
  ai_caf(s,OCT31);(void)ai_ts(s,at(QGIMTIMR,k));ai_caf(s,ZERO);(void)ai_ts(s,ALLOWGTS);
doneyet: /* CCS QRNDXER; TCF TIMQGMBL; DXCH RUPTREG3; DXCH ITEMP2;
  * TC IBNKCALL. TIMQGMBL: CAF ZERO. */
  c=ai_ccs(s,QRNDXER);LM_TG_PATH(81u+c);
  if(c!=0u)break;
  ai_caf(s,ZERO);
 }
 if(c==1u)ai_dxch(s,RUPTREG3);
 if(c<=2u)ai_dxch(s,ITEMP2);
 ai_tc(s,03434u);W[5]=LM_TG_IBNKCALL;
 return EXEC_OK;
}
/* 21,3435 (IBNKCALL's return): DXCH ITEMP2; DXCH RUPTREG3; TC RUPTREG2. */
static enum exec_status timeret(const agc_native_program *p,agc_image *s,void *io){if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_TG_FB21)||!tc_word(W[RUPTREG2]))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,ITEMP2);ai_dxch(s,RUPTREG3);
 W[5]=W[RUPTREG2];ai_tc(s,RUPTREG2+1u); /* the word in RUPTREG2 runs as its TC */
 return EXEC_OK;
}
static const agc_routine_entry entries[4]={
 {P16(LM_TG_ACDTC12),acdtc12},{P21(LM_TG_GTS),gts},{P21(LM_TG_TIMEGMBL),timegmbl},{P21(LM_TG_TIMERET),timeret}};
const agc_routine_table lm_trim_gimbal_routines={entries,4};
