/* SPDX-License-Identifier: MIT */
/* General compiled Luminary099 interpretive order bodies (INTERPRETER.agc),
 * written instruction for instruction with the agc_image_ops helpers. Each
 * function is one fixed source path family from its body entry (or address
 * formation step) to DANZIG or the next dispatcher step; branches inside
 * follow the original's runtime data tests (CCS, BZF, BZMF, overflow
 * skips) exactly as the original instructions take them. Nothing here
 * reads an order word, decodes, or fetches code: order and operand words
 * of the calling routine arrive as literal arguments from its compiled
 * caller, and fixed operands as literal data. Verified body by body against
 * the pinned yaAGC engine by tools/lmi_body_differential.py. */
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#define W s->words

/* Erasable assignments (Luminary099 ERASABLE_ASSIGNMENTS). */
enum {
 rA=0,rL=1,rQ=2,rEB=3,rFB=4,rBB=6,rZERO=7,rCYR=020,rSR=021,rCYL=022,rEDOP=023,
 rINTBIT15=0115,rADDRWD=0116,rPOLISH=0117,rFIXLOC=0120,rOVFIND=0121,rVBUF=0122,
 rBUF=0130,rMPTEMP=0135,rDOTINC=0136,rDOTRET=0137,rPOLYCNT=0140,rPOLYRET=0141,
 rMPAC=0154,rMODE=0163,rLOC=0164,rBANKSET=0165,rPUSHLOC=0166
};
/* Work-area tags relative to FIXLOC. */
enum {tX1=046,tX2=047,tLVSQUARE=042,tQPRET=052};
/* Fixed constants (FIXED_FIXED_CONSTANT_POOL and INTERPRETER). */
#define ZERO 0u
#define ONE 1u
#define TWO 2u
#define FOUR 4u
#define FIVE 5u
#define SEVEN 7u
#define LOW2 3u
#define LOW3 7u
#define LOW7 0177u
#define LOW8 0377u
#define LOW10 01777u
#define LOW11 03777u
#define BIT1 1u
#define BIT5 020u
#define BIT6 040u
#define BIT8 0200u
#define BIT10 01000u
#define BIT14 020000u
#define BIT15 040000u
#define HALF 020000u
#define QUARTER 010000u
#define POSMAX 037777u
#define NEGONE 077776u
#define NEG4 077773u
#define NEG12 077763u
#define B12T14 034000u
#define OCT1400 01400u
#define OCT23 023u
#define OCT176 0176u
#define TWO_K 02000u
#define DEC45 055u
#define ENDVAC_NEG 077722u
#define ENDERAS_NEG 074056u
#define OCT10_NEG 077767u
#define DLOADCOD 040014u
#define VLOADCOD BIT15
#define LVBUF 0122u

#define CA(a) ai_ca(s,(a))
#define CAF(k) ai_caf(s,(k))
#define CS(a) ai_cs(s,(a))
#define CSF(k) ai_csf(s,(k))
#define AD(a) ai_ad(s,(a))
#define ADF(k) ai_adf(s,(k))
#define MASK(a) ai_mask(s,(a))
#define MASKF(k) ai_maskf(s,(k))
#define TS(a) ai_ts(s,(a))
#define XCH(a) ai_xch(s,(a))
#define LXCH(a) ai_lxch(s,(a))
#define ADS(a) ai_ads(s,(a))
#define INCR(a) ai_incr(s,(a))
#define DAS(a) ai_das(s,(a))
#define DCA(a) ai_dca(s,(a))
#define DCS(a) ai_dcs(s,(a))
#define DXCH(a) ai_dxch(s,(a))
#define MP(a) ai_mp(s,(a))
#define MPF(k) ai_mpf(s,(k))
#define CCS(a) ai_ccs(s,(a))
#define AT(a) ((unsigned)(a))
/* TC to a subroutine: Q gets the address after the TC. */
#define TC(ret) ai_tc(s,(ret))
/* TC Q (return): executes the TC held in Q, leaving Q = 3. */
#define RETQ() ai_tc(s,3u)

static void danzig(apollo_overlay_state *s){W[5]=06061;}

/* Operand pair at C(ADDRWD)+off: live erasable, or the caller's fixed data. */
static void dca_op(apollo_overlay_state *s,const agc_word *k,unsigned off){
 if(k)ai_dcaf(s,k+off);else DCA(AT(W[rADDRWD]+off));
}
/* NO.WDS/REVCNT region 06241-06247 as INDEX MODE/A select it. */
static agc_word nowds(int index){
 static const agc_word t[7]={2,6,6,2,3,6,010020};
 return t[index+3];
}
static int signed_word(agc_word v){return (v&040000u)?-(int)(~v&037777u):(int)v;}

/* DIRADRES from INCR LOC (an address word was given) to the INDJUMP body. */
static void diradres_given(apollo_overlay_state *s,agc_word target){
 uint8_t c;
 INCR(rLOC);(void)TS(rADDRWD);ADF(ENDVAC_NEG);c=CCS(rA);
 if(c==2u||c==3u){
  /* NETZERO CA FIXLOC; ADS ADDRWD (or -0 straight to ADS). */
  if(c==2u)CA(rFIXLOC);
  ADS(rADDRWD);W[5]=target;return;
 }
 /* + : AD -ENDERAS; IERASTST EXTEND BZMF GEADDR. */
 if(c==0u)ADF(ENDERAS_NEG);
 if(ai_bzmf(s)){
  MASKF(LOW8);ADF(OCT1400);XCH(rADDRWD);(void)TS(rEB);W[5]=target;return;
 }
 MASKF(LOW10);ADF(LOW10);XCH(rADDRWD);AD(rINTBIT15);(void)TS(rFB);W[5]=target;
}
/* PUSHUP: operand from the push-down list. */
static void pushup(apollo_overlay_state *s,agc_word target){
 uint8_t c;
 CAF(OCT23);MASK(rCYR);ADF(OCT10_NEG);c=CCS(rA);
 if(c==0u)goto regup;
 /* +0 cannot occur (no -0 in the masked code). */
 ADF(NEG4);c=CCS(rA);
 if(c==0u){CSF(nowds(signed_word(agc_correct(W[rA]))));}
 else if(c==1u){CSF(nowds(0));}
 else if(c==3u){CSF(nowds(signed_word(W[rMODE])-2));} /* REVCNT = NO.WDS -2 */
 goto regup2;
regup:
 CSF(nowds(signed_word(W[rMODE])));
regup2:
 ADS(rPUSHLOC);(void)TS(rADDRWD);W[5]=target;
}
void lmi_diradres(apollo_overlay_state *s,agc_word operand,agc_word target){
 /* DIRADRES INDEX LOC, CS 1; CCS A (negative: an address is given). */
 CSF(operand);(void)CCS(rA);diradres_given(s,target);
}
void lmi_diradres_pushup(apollo_overlay_state *s,agc_word next,agc_word target){
 /* DIRADRES INDEX LOC, CS 1; CCS A (positive: next word is an order). */
 CSF(next);(void)CCS(rA);pushup(s,target);
}
void lmi_address_pushup(apollo_overlay_state *s,agc_word next,agc_word target){
 /* ADDRESS MASK BIT1; CCS A (+0); DIRADRES as above. */
 MASKF(BIT1);(void)CCS(rA);lmi_diradres_pushup(s,next,target);
}

/* STORE (from its first instruction) through STARTSTO and TC Q. */
static void store(apollo_overlay_state *s){
 uint8_t c;unsigned at;
 CS(rADDRWD);ADF(DEC45);c=CCS(rA);
 if(c==0u){CA(rFIXLOC);}
 else if(c>=2u){
  /* - : CA OCT1400; -0 (address 45) enters at the XCH with A = +0. */
  if(c==2u)CAF(OCT1400);
  XCH(rADDRWD);(void)TS(rEB);MASKF(LOW8);
 }
 ADS(rADDRWD);
 DCA(rMPAC);at=W[rADDRWD];DXCH(at);c=CCS(rMODE);
 if(c==0u){CA(rMPAC+2);(void)TS(at+2u);}
 else if(c==2u){DCA(rMPAC+3);DXCH(at+2u);DCA(rMPAC+5);DXCH(at+4u);}
 RETQ();
}
/* DOSTORE (A holds the store code) through STORJUMP's TC and the store. */
static void dostore(apollo_overlay_state *s){
 static const agc_word next[8]={06061,06061,06061,06460,06144,06463,06466,0};
 unsigned kind;
 (void)TS(rADDRWD);MASKF(LOW11);XCH(rADDRWD);MASKF(B12T14);MPF(BIT5);
 kind=(unsigned)W[rA]/2u;
 TC((agc_word)(06373u+2u*kind));
 if(kind==1u){CS(AT(W[rFIXLOC]+tX1));ADS(rADDRWD);}
 else if(kind==2u){CS(AT(W[rFIXLOC]+tX2));ADS(rADDRWD);}
 store(s);
 if(kind==7u){
  /* STOTC: CAF CALLCODE; TS CYR; TCF 15BITADR. */
  CAF(032u);(void)TS(rCYR);W[5]=06252;return;
 }
 W[5]=next[kind];
}
void lmi_store_code(apollo_overlay_state *s,agc_word code){
 /* NEWOPS INDEX LOC, CA 0; CCS A (positive) TCF DOSTORE. */
 CAF(code);(void)CCS(rA);dostore(s);
}
void lmi_stadr(apollo_overlay_state *s,agc_word word){
 /* STADR CA BANKSET; TS FBANK; INCR LOC; INDEX LOC, CS 0; AD NEGONE. */
 CA(rBANKSET);(void)TS(rFB);INCR(rLOC);CSF(word);ADF(NEGONE);dostore(s);
}
void lmi_dodload(apollo_overlay_state *s){CAF(DLOADCOD);(void)TS(rCYR);W[5]=06106;}
void lmi_dovload(apollo_overlay_state *s){CAF(VLOADCOD);(void)TS(rCYR);W[5]=06106;}

/* NEWMODE (TS MODE) and VMODE. */
static void newmode(apollo_overlay_state *s){(void)TS(rMODE);danzig(s);}
static void vmode(apollo_overlay_state *s){CSF(ONE);newmode(s);}
void lmi_dload_op(apollo_overlay_state *s,const agc_word *k){
 dca_op(s,k,0);DXCH(rMPAC);CAF(ZERO);(void)TS(rMPAC+2);newmode(s);
}
static void endvload4(apollo_overlay_state *s,const agc_word *k){dca_op(s,k,4);DXCH(rMPAC+5);vmode(s);}
static void endvload(apollo_overlay_state *s,const agc_word *k){dca_op(s,k,2);DXCH(rMPAC+3);endvload4(s,k);}
void lmi_vload(apollo_overlay_state *s,const agc_word *k){
 dca_op(s,k,0);DXCH(rMPAC);endvload(s,k);
}
void lmi_pdvl(apollo_overlay_state *s,const agc_word *k){
 uint8_t c;
 dca_op(s,k,0);DXCH(rMPAC);DXCH(W[rPUSHLOC]);CAF(nowds(signed_word(W[rMODE])));ADS(rPUSHLOC);c=CCS(rMODE);
 if(c==0u){
  /* TPDVL */
  dca_op(s,k,2);DXCH(rMPAC+3);CA(rMPAC+2);(void)TS(AT(W[rPUSHLOC]-1u));endvload4(s,k);return;
 }
 if(c==1u){endvload(s,k);return;}
 /* VPDVL */
 dca_op(s,k,2);DXCH(rMPAC+3);DXCH(AT(W[rPUSHLOC]-4u));dca_op(s,k,4);DXCH(rMPAC+5);DXCH(AT(W[rPUSHLOC]-2u));danzig(s);
}
void lmi_pddl_op(apollo_overlay_state *s,const agc_word *k){
 uint8_t c;
 dca_op(s,k,0);DXCH(rMPAC);DXCH(W[rPUSHLOC]);CAF(nowds(signed_word(W[rMODE])));ADS(rPUSHLOC);c=CCS(rMODE);
 if(c==0u){(void)TS(rMODE);XCH(rMPAC+2);(void)TS(AT(W[rPUSHLOC]-1u));danzig(s);return;}
 if(c==1u){(void)TS(rMPAC+2);danzig(s);return;}
 (void)TS(rMODE);(void)TS(rMPAC+2);DXCH(rMPAC+3);DXCH(AT(W[rPUSHLOC]-4u));DXCH(rMPAC+5);DXCH(AT(W[rPUSHLOC]-2u));danzig(s);
}
void lmi_push(apollo_overlay_state *s){
 uint8_t c;
 DCA(rMPAC);DXCH(W[rPUSHLOC]);CAF(nowds(signed_word(W[rMODE])));ADS(rPUSHLOC);c=CCS(rMODE);
 if(c==0u){CA(rMPAC+2);(void)TS(AT(W[rPUSHLOC]-1u));danzig(s);return;}
 if(c==1u){danzig(s);return;}
 DCA(rMPAC+3);DXCH(AT(W[rPUSHLOC]-4u));DCA(rMPAC+5);DXCH(AT(W[rPUSHLOC]-2u));danzig(s);
}

/* ---- Shared basic subroutines (INTERPRETER.agc) ---------------------- */
#define ZL() ai_zl(s)
#define DOUBLE() ai_double(s)
#define COM() ai_com(s)
#define SQUARE() ai_square(s)
#define DDOUBL() ai_ddoubl(s)
#define AUGA() ai_aug(s,0u)
/* INDEX r; CAF BIT14: FIXED_FIXED words 04730-04777 (the bit table runs
 * BIT15 04735 to BIT1 04753), indexed by the one's-complement C(r). */
static agc_word bit14_indexed(const apollo_overlay_state *s,unsigned r){
 static const agc_word t[]={01007,02,037777,037777,057777,040000,020000,010000,04000,02000,01000,0400,0200,0100,
  040,020,010,04,02,01,077777,0,05,07,013,015,017,030,033,035,032,045,046,050,055,060,062,0120,0140,0144};
 int i=6+signed_word((agc_word)(W[r]&077777u));
 return i>=0&&i<(int)(sizeof t/sizeof t[0])?t[i]:0u;
}

/* BRANCH 06723 (Q holds the caller's return): 0 positive (TC Q),
 * 1 zero (INDEX Q, TC 1), 2 negative (INDEX Q, TC 2). -0 is passed over. */
static uint8_t branch(apollo_overlay_state *s){
 unsigned i;uint8_t c;
 for(i=0;i<3u;++i){
  c=CCS(rMPAC+i);
  if(c==0u){RETQ();return 0;}
  if(c==2u){TC(06743);return 2;}
 }
 TC(06741);return 1;
}
/* OVERFLOW 07017 (A = +-1, L = component offset 0, 3 or 5). Returns 1 when
 * the propagated overflow reached OVFIND (SETOVF2). Ends with TC Q. */
static uint8_t overflow(apollo_overlay_state *s){
 unsigned l;
 CSF(W[rA]==1u?040000u:037777u);(void)TS(rBUF);AUGA();
 l=W[rL];ADS(AT(rMPAC+1u+l));
 if(!TS(rZERO))CAF(ZERO);
 AD(rBUF);ADS(AT(rMPAC+l));
 if(!TS(rZERO)){RETQ();return 0;}
 (void)TS(rOVFIND);RETQ();return 1;
}
static void overflwy(apollo_overlay_state *s){(void)TS(rL);CAF(3u);XCH(rL);(void)overflow(s);}
static void overflwz(apollo_overlay_state *s){(void)TS(rL);CAF(FIVE);XCH(rL);(void)overflow(s);}
/* ENDVXV: DAS MPAC; BZF DANZIG; else TC OVERFLOW, TCF DANZIG. */
static void endvxv(apollo_overlay_state *s){
 DAS(rMPAC);
 if(!ai_bzf(s)){TC(07004);(void)overflow(s);}
 danzig(s);
}
void lmi_dad(apollo_overlay_state *s,const agc_word *k){dca_op(s,k,0);endvxv(s);}
void lmi_dsu(apollo_overlay_state *s,const agc_word *k){
 if(k)ai_dcsf(s,k);else DCS(W[rADDRWD]);
 endvxv(s);
}
void lmi_bdsu(apollo_overlay_state *s,const agc_word *k){
 dca_op(s,k,0);DXCH(rMPAC);ai_dcom(s);endvxv(s);
}
/* VROUND/ROUNDSUB 07136-07153. `entry` 0 ROUNDSUB, 1 ROUNDSUB +1, 2 VROUND. */
static void roundsub(apollo_overlay_state *s,unsigned entry){
 if(entry==0u)CAF(ZERO);
 if(entry<2u)(void)TS(rMODE);
 XCH(rMPAC+2);DOUBLE();
 if(!TS(rL)){RETQ();return;}
 AD(rMPAC+1);
 if(!TS(rMPAC+1)){RETQ();return;}
 AD(rMPAC);
 if(!TS(rMPAC)){RETQ();return;}
 (void)TS(rOVFIND);RETQ();
}
/* DMPSUB 07107 with the operand pair at C(ADDRWD) (or fixed `k`). */
static void dmpsub(apollo_overlay_state *s,const agc_word *k){
 if(k)CAF(k[1]);else CA(AT(W[rADDRWD]+1u));
 (void)TS(rMPAC+2);CAF(ZERO);XCH(rMPAC+1);(void)TS(rMPTEMP);
 MP(rMPAC+2);XCH(rMPAC+2);MP(rMPAC);DAS(rMPAC+1);
 if(k)CAF(k[0]);else CA(W[rADDRWD]);
 XCH(rMPTEMP);
 MP(rMPTEMP);DAS(rMPAC+1);XCH(rMPAC);MP(rMPTEMP);DAS(rMPAC);RETQ();
}
void lmi_dmp(apollo_overlay_state *s,const agc_word *k){TC(07573);dmpsub(s,k);danzig(s);}
void lmi_dmpr(apollo_overlay_state *s,const agc_word *k){
 TC(07575);dmpsub(s,k);TC(07576);roundsub(s,1);danzig(s);
}
/* TPAGREE 07257 (Q = return); returns via TC L. */
static void tpagree(apollo_overlay_state *s){
 uint8_t c;
 LXCH(rQ);TC(07261);c=branch(s);
 if(c==1u){(void)TS(rMPAC+2);goto argzero2;}
 if(c==2u)CSF(POSMAX);else CAF(POSMAX);
 (void)TS(rQ);AUGA();AD(rMPAC+2);
 if(!TS(rMPAC+2))CAF(ZERO);
 AD(rQ);AD(rMPAC+1);
 if(!TS(rMPAC+1))CAF(ZERO);
 AD(rQ);AD(rMPAC);
argzero2:
 if(!TS(rMPAC))(void)TS(rMPAC+1);
 /* TC L: executes the return TC held in L, leaving Q = 2. */
 ai_tc(s,2u);
}

/* ---- Shifts (bank 0) -------------------------------------------------- */
void lmi_shortv(apollo_overlay_state *);
static void mpacshr3(apollo_overlay_state *s){
 /* MPACSHR +3 TS MPAC+2; CA MPTEMP; MP MPAC; DXCH MPAC; CA MPTEMP; MP L;
  * DAS MPAC+1; TCF DANZIG. */
 (void)TS(rMPAC+2);CA(rMPTEMP);MP(rMPAC);DXCH(rMPAC);CA(rMPTEMP);MP(rL);DAS(rMPAC+1);danzig(s);
}
static void mpacshr(apollo_overlay_state *s){CA(rMPTEMP);MP(rMPAC+2);mpacshr3(s);}
/* VSHR2 (A = value to double) to TC Q. */
static void vshr2(apollo_overlay_state *s){
 DOUBLE();
 if(TS(rMPAC+2))ADS(rMPAC+1);
 CAF(ZERO);(void)TS(rMPAC+2);XCH(rMPAC);MP(rMPTEMP);DAS(rMPAC);RETQ();
}
static void mpacsrnd(apollo_overlay_state *s){
 CA(rMPAC+2);MP(rMPTEMP);XCH(rMPAC+1);MP(rMPTEMP);XCH(rMPAC+1);AD(rL);vshr2(s);
}
static void vshrrnd(apollo_overlay_state *s){CA(rMPTEMP);MP(rMPAC+1);(void)TS(rMPAC+1);XCH(rL);vshr2(s);}
/* RIGHTR TC MPACSRND; TCF NEWMODE (C(A) = 0). */
static void rightr(apollo_overlay_state *s){TC(02032);mpacsrnd(s);newmode(s);}
static void vrotatex(apollo_overlay_state *s){DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rMPAC+3);DXCH(rMPAC);danzig(s);}
/* VSSR (MPTEMP = count - 1); `indexed` 0 enters at VSSR +1, past the
 * INDEX MPTEMP (CCS -0 of CYR in SHORTV lands there). */
static void vssr(apollo_overlay_state *s,uint8_t indexed){
 CAF(indexed?bit14_indexed(s,rMPTEMP):BIT14);(void)TS(rMPTEMP);
 TC(02133);vshrrnd(s);DXCH(rMPAC);DXCH(rMPAC+3);DXCH(rMPAC);
 TC(02137);vshrrnd(s);DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rMPAC);
 TC(02143);vshrrnd(s);vrotatex(s);
}
/* TSSL +2 loop (MPTEMP = count - 1), then the CYR round test. */
static void tssl2(apollo_overlay_state *s){
 uint8_t c;
 for(;;){
  DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);AD(rMPAC);
  if(TS(rMPAC))(void)TS(rOVFIND);
  if(CCS(rMPTEMP)!=0u)break;
  (void)TS(rMPTEMP);
 }
 c=CCS(rCYR);
 if(c==0u){TC(02117);roundsub(s,0);}
 /* -0 in CYR (GENSCL's MP leaves -0 for an unrounded count) passes the
  * three exits and runs on into SHORTV, as the original does. */
 if(c==3u){lmi_shortv(s);return;}
 danzig(s);
}
/* VSSL loop from VSSL (MPTEMP = count - 1). */
static void vssl(apollo_overlay_state *s){
 for(;;){
  DCA(rMPAC);DAS(rMPAC);if(!ai_bzf(s)){TC(02153);(void)overflow(s);}
  DCA(rMPAC+3);DAS(rMPAC+3);if(!ai_bzf(s)){TC(02161);overflwy(s);}
  DCA(rMPAC+5);DAS(rMPAC+5);if(!ai_bzf(s)){TC(02167);overflwz(s);}
  if(CCS(rMPTEMP)!=0u)break;
  (void)TS(rMPTEMP);
 }
 danzig(s);
}
void lmi_short_dispatch(apollo_overlay_state *s){
 /* OPJUMP2 CCS CYR (+); OPJUMP3 TS FBANK; CCS CYR (-); CCS MODE: SHORTT
  * for a scalar, SHORTV for a vector. */
 uint8_t c;
 (void)CCS(rCYR);(void)TS(rFB);(void)CCS(rCYR);c=CCS(rMODE);
 W[5]=c==2u?02121:02017;
}
void lmi_shortt(apollo_overlay_state *s){
 uint8_t c;
 CAF(6u);MASK(rCYR);(void)TS(rSR);c=CCS(rCYR);
 if(c==0u){
  /* TSSL CA SR; TS MPTEMP. */
  CA(rSR);(void)TS(rMPTEMP);tssl2(s);return;
 }
 /* TSSR INDEX SR; CAF BIT14 (-0 enters past the INDEX); TS MPTEMP; CCS CYR. */
 CAF(c==3u?BIT14:bit14_indexed(s,rSR));(void)TS(rMPTEMP);
 switch(CCS(rCYR)){
 case 0:rightr(s);return;
 case 1:newmode(s);return;                       /* TCF NEWMODE, C(A) = 0 */
 case 2:mpacshr(s);return;
 default:MP(rMPAC+2);mpacshr3(s);return;         /* MPACSHR +1 */
 }
}
void lmi_shortv(apollo_overlay_state *s){
 uint8_t c;
 CAF(LOW3);MASK(rCYR);(void)TS(rMPTEMP);c=CCS(rCYR);
 if(c==0u)vssl(s);else vssr(s,c!=3u);
}
/* SETROUND 02272 (A = bits to double; L = 0) to TC Q. */
static void setround(apollo_overlay_state *s){
 DOUBLE();if(!TS(rMPAC+2))CAF(ZERO);XCH(rL);RETQ();
}
/* DOSSHFT and the GENSCR right shift (MPTEMP = count - 1). */
static void dosshft(apollo_overlay_state *s){
 CAF(bit14_indexed(s,rMPTEMP));(void)TS(rMPTEMP);CAF(BIT10);MASK(rADDRWD);
 if(CCS(rA)==0u)rightr(s);else mpacshr(s);
}
/* CAF BIT10; MASK ADDRWD; CCS A; TC ROUNDSUB; TCF DANZIG (GENSCR +12, or
 * GENSHIFT +3 for a zero count; `ret` is the address after the TC). */
static void genscr_tail(apollo_overlay_state *s,agc_word ret){
 CAF(BIT10);MASK(rADDRWD);
 if(CCS(rA)==0u){TC(ret);roundsub(s,0);}
 danzig(s);
}
/* GENSCR +1 (A = remaining count): AD NEG12; BZMF DOSSHFT; GENSCR +4 AD
 * NEGONE ... shifting 14 places a pass. `at4` enters at GENSCR +4. */
static void genscr1(apollo_overlay_state *s,uint8_t at4){
 for(;;){
  if(!at4){
   ADF(NEG12);
   if(ai_bzmf(s)){dosshft(s);return;}
  }
  at4=0;ADF(NEGONE);
  /* GENSCR +4 */
  (void)TS(rMPTEMP);CAF(ZERO);XCH(rMPAC);XCH(rMPAC+1);(void)TS(rMPAC+2);
  switch(CCS(rMPTEMP)){
  case 0:(void)TS(rMPTEMP);TC(02314);break;     /* TS MPTEMP; TC GENSCR +1 */
  case 1:TC(02314);break;                      /* TC GENSCR +1 */
  default:genscr_tail(s,02321);return;
  }
 }
}
static void genscr(apollo_overlay_state *s){CA(rMPTEMP);genscr1(s,0);}
static void vright2(apollo_overlay_state *s){
 uint8_t c;
 for(;;){
  ADF(NEG12);
  if(ai_bzmf(s)){vssr(s,1);return;}
  ADF(NEGONE);(void)TS(rMPTEMP);CAF(ZERO);(void)TS(rL);
  XCH(rMPAC);XCH(rMPAC+1);TC(02254);setround(s);DAS(rMPAC);
  XCH(rMPAC+3);XCH(rMPAC+4);TC(02260);setround(s);DAS(rMPAC+3);
  XCH(rMPAC+5);XCH(rMPAC+6);TC(02264);setround(s);DAS(rMPAC+5);
  /* CCS MPTEMP: + TS MPTEMP; +0 TCF VRIGHT2; -0 (a count of exactly 14
   * more, as AD NEGONE makes it) TCF DANZIG. */
  c=CCS(rMPTEMP);
  if(c==3u){danzig(s);return;}
  if(c==0u)(void)TS(rMPTEMP);
 }
}
static void right(apollo_overlay_state *s){
 if(CCS(rMODE)!=2u){genscr(s);return;}
 CA(rMPTEMP);vright2(s);
}
static void left(apollo_overlay_state *s){
 if(CCS(rMODE)==2u){vssl(s);return;}
 /* GENSCL CS ADDRWD; MP BIT6; TS CYR; TCF TSSL +2. */
 CS(rADDRWD);MPF(BIT6);(void)TS(rCYR);tssl2(s);
}
void lmi_genshift(apollo_overlay_state *s){
 /* GENSHIFT (A = LOW7): MASK ADDRWD; CCS A. */
 MASK(rADDRWD);
 if(CCS(rA)==1u){genscr_tail(s,02223);return;}
 /* GENSHFT2 TS MPTEMP; CAF BIT8; MP ADDRWD; MASK LOW2; INDEX A; TCF +1. */
 (void)TS(rMPTEMP);CAF(BIT8);MP(rADDRWD);MASKF(LOW2);
 switch(W[rA]){
 case 0:CS(rMPTEMP);ADF(OCT176);(void)TS(rMPTEMP);right(s);return;   /* RIGHT- */
 case 1:left(s);return;                                              /* LEFT */
 case 2:CSF(OCT176);AD(rMPTEMP);COM();(void)TS(rMPTEMP);left(s);return; /* LEFT- */
 default:right(s);return;                                            /* RIGHT */
 }
}
/* GSHIFT CAF LOW7; TS FBANK; TCF GENSHIFT. */
void lmi_gshift(apollo_overlay_state *s){CAF(LOW7);(void)TS(rFB);lmi_genshift(s);}
/* TSLC CAF ZERO; TS FBANK; TSLC2 ... ENDTSLC CS MPTEMP; STORE1. */
void lmi_tslc(apollo_overlay_state *s){
 uint8_t c;
 CAF(ZERO);(void)TS(rFB);(void)TS(rMPTEMP);TC(02174);c=branch(s);
 if(c!=1u){
  TC(02177);tpagree(s);CA(rMPAC);
  for(;;){
   DOUBLE();
   if(ai_ovsk(s))break;
   INCR(rMPTEMP);DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);
  }
 }
 CS(rMPTEMP);(void)TS(W[rADDRWD]);danzig(s);
}

/* ---- Unary scalar/vector orders (bank 0 and 3) ------------------------ */
/* COMP 07670 and DCOMP. */
static void comp(apollo_overlay_state *s){
 DCS(rMPAC);DXCH(rMPAC);
 if(CCS(rMODE)<2u){CS(rMPAC+2);(void)TS(rMPAC+2);danzig(s);return;}
 DCS(rMPAC+3);DXCH(rMPAC+3);DCS(rMPAC+5);DXCH(rMPAC+5);danzig(s);
}
void lmi_comp(apollo_overlay_state *s){comp(s);}
void lmi_sign(apollo_overlay_state *s,const agc_word *k){
 uint8_t c=k?ai_ccsf(s,k[0]):CCS(W[rADDRWD]);
 if(c==0u){danzig(s);return;}
 if(c==2u){comp(s);return;}
 c=k?ai_ccsf(s,k[1]):CCS(AT(W[rADDRWD]+1u));
 if(c==2u)comp(s);else danzig(s);
}
void lmi_abs(apollo_overlay_state *s){TC(03227);if(branch(s)==2u)comp(s);else danzig(s);}
/* DSQSUB 03300 to TC Q. */
static void dsqsub(apollo_overlay_state *s){
 CA(rMPAC+1);SQUARE();(void)TS(rMPAC+2);CAF(ZERO);XCH(rMPAC+1);MP(rMPAC);DDOUBL();DAS(rMPAC+1);
 XCH(rMPAC);SQUARE();DAS(rMPAC);RETQ();
}
void lmi_dsq(apollo_overlay_state *s){TC(03175);dsqsub(s);danzig(s);}
/* ENDDOT 07205 to TC DOTRET (Q = DOTRET +1). */
static void enddot(apollo_overlay_state *s){
 DXCH(rBUF+1);DAS(rMPAC+1);AD(rMPAC);AD(rBUF);
 if(TS(rMPAC)){TC(07214);(void)overflow(s);}
 ai_tc(s,rDOTRET+1u);
}
/* VSQSUB 03317 (QXCH DOTRET ... ENDDOT). */
static void vsqsub(apollo_overlay_state *s){
 ai_qxch(s,rDOTRET);
 TC(03322);dsqsub(s);DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rBUF);CA(rMPAC+2);(void)TS(rBUF+2);
 TC(03330);dsqsub(s);DXCH(rMPAC+1);DAS(rBUF+1);AD(rMPAC);AD(rBUF);
 if(TS(rBUF))(void)TS(rOVFIND);
 DXCH(rMPAC+5);DXCH(rMPAC);TC(03342);dsqsub(s);enddot(s);
}
/* DMODE CAF ZERO; TCF NEWMODE. */
static void dmode(apollo_overlay_state *s){CAF(ZERO);newmode(s);}
void lmi_vsq(apollo_overlay_state *s){TC(03246);vsqsub(s);dmode(s);}
/* DOTSUB 07156 (DOTINC set) with the vector operand at C(ADDRWD) or `k`. */
static void dotsub(apollo_overlay_state *s,const agc_word *k){
 unsigned base=W[rADDRWD];
 ai_qxch(s,rDOTRET);
 TC(07161);dmpsub(s,k);DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rBUF);CA(rMPAC+2);(void)TS(rBUF+2);
 CA(rDOTINC);ADS(rADDRWD);
 TC(07171);dmpsub(s,k?k+(W[rADDRWD]-base):0);DXCH(rMPAC+1);DAS(rBUF+1);AD(rMPAC);AD(rBUF);
 if(TS(rBUF))(void)TS(rOVFIND);
 DXCH(rMPAC+5);DXCH(rMPAC);CA(rDOTINC);ADS(rADDRWD);
 TC(07205);dmpsub(s,k?k+(W[rADDRWD]-base):0);enddot(s);
}
void lmi_dot(apollo_overlay_state *s,const agc_word *k){
 /* DOT TC PREDOT (CAF TWO; TS DOTINC; DOTSUB); DMODE. */
 TC(07332);CAF(TWO);(void)TS(rDOTINC);dotsub(s,k);dmode(s);
}
#undef W
