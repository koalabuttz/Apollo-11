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
#include "lm_interp_steps.h"
#include "interp_mission.h"
#define W s->words

/* Erasable assignments (Luminary099 ERASABLE_ASSIGNMENTS). */
enum {
 rA=0,rL=1,rQ=2,rEB=3,rFB=4,rBB=6,rZERO=7,rCYR=020,rSR=021,rCYL=022,rEDOP=023,
 rINTBIT15=0115,rADDRWD=0116,rPOLISH=0117,rFIXLOC=0120,rOVFIND=0121,rVBUF=0122,
 rBUF=0130,rMPTEMP=0135,rDOTINC=0136,rDOTRET=0137,rPOLYCNT=0140,rPOLYRET=0141,
 rMPAC=0154,rMODE=0163,rLOC=0164,rBANKSET=0165,rPUSHLOC=0166,
 rBUF2=0133,rDVSIGN=0136,rDVNORMCT=0137,rMAXDVSW=0140,rTEM1=0141
};
/* Work-area tags relative to FIXLOC. */
enum {tX1=046,tX2=047,tS1=050,tLVSQUARE=042,tLV=044,tQPRET=052};
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

static void danzig(apollo_overlay_state *s){W[5]=IF_(06061,06030);}

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
 static const agc_word next[8]={IF_(06061,06030),IF_(06061,06030),IF_(06061,06030),IF_(06460,06427),IF_(06144,06113),IF_(06463,06432),IF_(06466,06435),0};
 unsigned kind;
 (void)TS(rADDRWD);MASKF(LOW11);XCH(rADDRWD);MASKF(B12T14);MPF(BIT5);
 kind=(unsigned)W[rA]/2u;
 TC((agc_word)(IF_(06373u,06342u)+2u*kind));
 if(kind==1u){CS(AT(W[rFIXLOC]+tX1));ADS(rADDRWD);}
 else if(kind==2u){CS(AT(W[rFIXLOC]+tX2));ADS(rADDRWD);}
 store(s);
 if(kind==7u){
  /* STOTC: CAF CALLCODE; TS CYR; TCF 15BITADR. */
  CAF(032u);(void)TS(rCYR);W[5]=IF_(06252,06221);return;
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
void lmi_dodload(apollo_overlay_state *s){CAF(DLOADCOD);(void)TS(rCYR);W[5]=IF_(06106,06055);}
void lmi_dovload(apollo_overlay_state *s){CAF(VLOADCOD);(void)TS(rCYR);W[5]=IF_(06106,06055);}

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
/* INDEX v; CAF k for k in FIXED_FIXED 04730-04777 (the bit table runs
 * BIT15 04735 to BIT1 04753), v taken as a one's-complement index. */
static agc_word fixed_indexed(unsigned k,agc_word v){
 static const agc_word t[]={01007,02,037777,037777,057777,040000,020000,010000,04000,02000,01000,0400,0200,0100,
#ifdef INTERP_COMANCHE
  /* Comanche055 has no OCT33 and OCT35 here (pool at 04774). */
  040,020,010,04,02,01,077777,0,05,07,013,015,017,030,032,045,046,050,055,060,062,0120,0140,0144,0310,0401};
#else
  040,020,010,04,02,01,077777,0,05,07,013,015,017,030,033,035,032,045,046,050,055,060,062,0120,0140,0144};
#endif
 int i=(int)k-(int)IF_(04730,04774)+signed_word((agc_word)(v&077777u));
 return i>=0&&i<(int)(sizeof t/sizeof t[0])?t[i]:0u;
}
#define BIT15_AT IF_(04735u,05001u)
#define BIT14_AT IF_(04736u,05002u)
/* INDEX r; CAF BIT14. */
static agc_word bit14_indexed(const apollo_overlay_state *s,unsigned r){return fixed_indexed(BIT14_AT,W[r]);}

/* BRANCH 06723 (Q holds the caller's return): 0 positive (TC Q),
 * 1 zero (INDEX Q, TC 1), 2 negative (INDEX Q, TC 2). -0 is passed over. */
static uint8_t branch(apollo_overlay_state *s){
 unsigned i;uint8_t c;
 for(i=0;i<3u;++i){
  c=CCS(rMPAC+i);
  if(c==0u){RETQ();return 0;}
  if(c==2u){TC(IF_(06743,06712));return 2;}
 }
 TC(IF_(06741,06710));return 1;
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
 if(!ai_bzf(s)){TC(IF_(07004,06753));(void)overflow(s);}
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
void lmi_dmp(apollo_overlay_state *s,const agc_word *k){TC(IF_(07573,07542));dmpsub(s,k);danzig(s);}
void lmi_dmpr(apollo_overlay_state *s,const agc_word *k){
 TC(IF_(07575,07544));dmpsub(s,k);TC(IF_(07576,07545));roundsub(s,1);danzig(s);
}
/* TPAGREE 07257 (Q = return); returns via TC L. */
static void tpagree(apollo_overlay_state *s){
 uint8_t c;
 LXCH(rQ);TC(IF_(07261,07230));c=branch(s);
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
 if(TS(rMPAC)){TC(IF_(07214,07163));(void)overflow(s);}
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
 TC(IF_(07161,07130));dmpsub(s,k);DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rBUF);CA(rMPAC+2);(void)TS(rBUF+2);
 CA(rDOTINC);ADS(rADDRWD);
 TC(IF_(07171,07140));dmpsub(s,k?k+(W[rADDRWD]-base):0);DXCH(rMPAC+1);DAS(rBUF+1);AD(rMPAC);AD(rBUF);
 if(TS(rBUF))(void)TS(rOVFIND);
 DXCH(rMPAC+5);DXCH(rMPAC);CA(rDOTINC);ADS(rADDRWD);
 TC(IF_(07205,07154));dmpsub(s,k?k+(W[rADDRWD]-base):0);enddot(s);
}
void lmi_dot(apollo_overlay_state *s,const agc_word *k){
 /* DOT TC PREDOT (CAF TWO; TS DOTINC; DOTSUB); DMODE. */
 TC(IF_(07332,07301));CAF(TWO);(void)TS(rDOTINC);dotsub(s,k);dmode(s);
}

/* ---- Division (bank 0) ------------------------------------------------ */
#define MP_HALF() MPF(HALF)
/* FINALDV +3 (A = minor quotient): DVSIGN -0 complements MPAC; TC Q with
 * C(A) = 0. */
static void finaldv3(apollo_overlay_state *s){
 if(TS(rMPAC+1)){RETQ();return;}
 if(CCS(rDVSIGN)!=3u){RETQ();return;}
 DCS(rMPAC);DXCH(rMPAC);CAF(ZERO);RETQ();
}
static void finaldv(apollo_overlay_state *s){ZL();ai_dv(s,rBUF);finaldv3(s);}
static void endmaxdv(apollo_overlay_state *s){AD(rBUF);finaldv(s);}
/* GENDDV, or GENDDV +1 with the dividend already in A,L. */
static void genddv(apollo_overlay_state *s,uint8_t plus1){
 if(!plus1)DXCH(rMPAC);
 ai_dv(s,rBUF);DXCH(rMPAC);CS(rMPAC);MP(rBUF+1);AD(rMPAC+1);
 if(ai_ovsk(s)){ai_su(s,rBUF);INCR(rMPAC);}
 else if(ai_bzmf(s)){
  /* -UP: BZF FINALDV +3; DIM MPAC; ENDMAXDV. */
  if(ai_bzf(s)){finaldv3(s);return;}
  ai_dim(s,rMPAC);endmaxdv(s);return;
 }
 /* +DOWN SU BUF; BZF +3; BZMF ENDMAXDV; +3 INCR MPAC; TCF FINALDV. */
 ai_su(s,rBUF);
 if(!ai_bzf(s)&&ai_bzmf(s)){endmaxdv(s);return;}
 INCR(rMPAC);finaldv(s);
}
static void maxdv(apollo_overlay_state *s){
 CS(rMPAC);AD(rBUF);
 if(!ai_bzf(s)){genddv(s,0);return;}
 CAF(POSMAX);(void)TS(rMPAC);CS(rBUF+1);AD(rMPAC+1);endmaxdv(s);
}
/* MAXTEST CCS MAXDVSW (-1, or +0 when the majors may be equal). */
static void maxtest(apollo_overlay_state *s){
 switch(CCS(rMAXDVSW)){
 case 1:maxdv(s);return;
 case 3:genddv(s,1);return;
 default:genddv(s,0);return;   /* - (MAXDVSW is never positive) */
 }
}
/* The dividend normalization chain entered at `e` (02540 is the UNIT
 * prologue -21D; 02547-02563 DDOUBLs; 02564 DXCH MPAC; 02565 MAXTEST), as
 * INDEX DVNORMCT; TC MAXTEST and INDEX MPTEMP; TCF MAXTEST -1 select it. */
static void ddv_chain(apollo_overlay_state *s,unsigned e){
 if(e==02540u){LXCH(rSR);MP_HALF();XCH(rL);AD(rSR);XCH(rL);genddv(s,1);return;}
 if(e<=02564u){for(;e<02564u;++e)DDOUBL();DXCH(rMPAC);}
 maxtest(s);
}
static unsigned chain_at(unsigned base,agc_word v){return (unsigned)((int)base+signed_word((agc_word)(v&077777u)));}
/* DVOVF 02405: +-POSMAX through FINALDV +3, OVFIND set, TC DANZIG. */
static void dvovf(apollo_overlay_state *s){
 CAF(POSMAX);(void)TS(rMPAC);TC(02410);finaldv3(s);CAF(ONE);(void)TS(rOVFIND);TC(02413);danzig(s);
}
/* DDV/BDDV 02353 (dividend MPAC, divisor BUF) to DANZIG. */
static void ddv_bddv(apollo_overlay_state *s){
 uint8_t c;
 CSF(ONE);(void)TS(rDVSIGN);(void)TS(rDVNORMCT);(void)TS(rMAXDVSW);
 c=CCS(rBUF);
 if(c==0u)goto bufpos;
 if(c==2u)goto bufneg;
bufzero:
 (void)TS(rMPAC+2);TC(02365);tpagree(s);
 c=CCS(rMPAC);
 if(c==0u)goto ovfplus;
 if(c==2u)goto ovfminus;
 XCH(rBUF+1);XCH(rBUF);XCH(rMPAC+1);XCH(rMPAC);
 c=CCS(rBUF);
 if(c==0u)goto bufplus;
 if(c==2u)goto bufminus;
 CS(rMPAC);
sgndvovf:
 if(!ai_bzmf(s))INCR(rDVSIGN);
 dvovf(s);return;
ovfminus:
 INCR(rDVSIGN);
ovfplus:
 CS(rBUF+1);goto sgndvovf;
bufminus:
 DCS(rBUF);DXCH(rBUF);INCR(rDVSIGN);
bufplus:
 c=CCS(rMPAC);
 if(c==1u||c==3u){
  c=CCS(rMPAC+1);
  if(c==1u||c==3u){danzig(s);return;}  /* zero dividend */
 }
 if(c==2u){DCS(rMPAC);DXCH(rMPAC);INCR(rDVSIGN);}
 /* MPAC+ CS MPAC; AD NEGONE; AD BUF; CCS A. */
 CS(rMPAC);ADF(NEGONE);AD(rBUF);c=CCS(rA);
 if(c==0u)goto dvnorm;
 if(c==1u)AD(rL);              /* -1/2+2 (OCT 60001) runs as AD L */
 CAF(HALF);DOUBLE();AD(rMPAC+1);if(!TS(rMPAC+1))CAF(ZERO);ADF(POSMAX);ADS(rMPAC);
 CAF(HALF);DOUBLE();AD(rBUF+1);if(!TS(rBUF+1))CAF(ZERO);ADF(POSMAX);ADS(rBUF);
 CS(rMPAC);AD(rBUF);c=CCS(rA);
 if(c==0u)goto dvnorm;
 if(c!=3u){dvovf(s);return;}   /* (+0 cannot occur: the majors differ) */
 (void)TS(rMAXDVSW);CS(rMPAC+1);AD(rBUF+1);
 if(ai_bzmf(s)){dvovf(s);return;}
dvnorm:
 for(;;){
  CA(rBUF);DOUBLE();
  if(ai_ovsk(s))break;
  ai_aug(s,rDVNORMCT);DCA(rBUF);DAS(rBUF);
 }
 DXCH(rMPAC);TC(02514);ddv_chain(s,chain_at(02565u,W[rDVNORMCT]));
 (void)TS(rMPAC+2);danzig(s);return;
bufpos:
 /* BUFPOS CCS A (A = |BUF| - 1). */
 if(CCS(rA)==0u)goto bufplus;
 CS(rBUF+1);
 if(ai_bzmf(s))goto bufplus;
 CAF(HALF);
bufpos6:
 DOUBLE();ADS(rBUF+1);CAF(ZERO);(void)TS(rBUF);goto bufzero;
bufneg:
 if(CCS(rA)==0u)goto bufminus;
 CA(rBUF+1);
 if(ai_bzmf(s))goto bufminus;
 CSF(HALF);goto bufpos6;
}
/* DDV/BDDV prologue tail BDDV +4: DXCH BUF; CAF ZERO; TS FBANK. */
static void bddv4(apollo_overlay_state *s){DXCH(rBUF);CAF(ZERO);(void)TS(rFB);ddv_bddv(s);}
void lmi_ddv(apollo_overlay_state *s,const agc_word *k){dca_op(s,k,0);bddv4(s);}
void lmi_bddv(apollo_overlay_state *s,const agc_word *k){dca_op(s,k,0);DXCH(rMPAC);bddv4(s);}

/* ---- Square root and length (bank 0) ---------------------------------- */
#define SLOPEHI 022650u
#define BIASHI 06552u
#define SLOPELO 032506u
#define BIASLO 04604u
#define NEGHALF2 060001u
/* SQRTSUB 03343 to TC Q; 1 when it ends at SQRTABRT's TC POODOO1 (1302). */
static uint8_t sqrtsub(apollo_overlay_state *s){
 uint8_t c;
 CAF(ZERO);(void)TS(rMPTEMP);
 c=CCS(rMPAC);
 if(c==0u)goto smpac;
 if(c==2u)goto sqrtneg;
 XCH(rMPAC+2);XCH(rMPAC+1);(void)TS(rMPAC);CAF(SEVEN);(void)TS(rMPTEMP);
 c=CCS(rMPAC);
 if(c==0u)goto smpac;
 if(c==2u)goto zeroans;
 XCH(rMPAC+1);(void)TS(rMPAC);CAF(SEVEN);ADS(rMPTEMP);
 c=CCS(rMPAC);
 if(c==0u)goto smpac;
 if(c==1u){RETQ();return 0;}
 if(c==2u)goto zeroans;
 goto fixroot;
sqrtneg:
 c=CCS(rA);
 if(c==0u)goto abrt;
 if(c==2u)goto zeroans;
 if(c==3u)goto fixroot;
 c=CCS(rMPAC+1);
 if(c==0u)goto zeroans;
 if(c==2u)goto abrt;
 goto fixroot;
zeroans:
 CAF(ZERO);
fixroot:
 if(!TS(rMPAC))(void)TS(rMPAC+1);
 RETQ();return 0;
abrt:
 /* SQRTABRT: Luminary DXCH LOC, TC POODOO1; Comanche TC POODOO. */
#ifdef INTERP_COMANCHE
 TC(03403);W[5]=05721;return 1;
#else
 DXCH(rLOC);TC(03404);W[5]=05726;return 1;
#endif
smpac:
 ADF(NEGHALF2);
 if(ai_bzmf(s))goto srtest;
 DXCH(rMPAC);LXCH(rSR);MP_HALF();DXCH(rMPAC);XCH(rSR);ADS(rMPAC+1);
arghi:
 CAF(SLOPEHI);MP(rMPAC);ADF(BIASHI);
arghi4:
 if(!TS(rBUF))CA(rMPAC);
 ZL();ai_dv(s,rBUF);MP_HALF();ADS(rBUF);MP_HALF();DXCH(rMPAC);ai_dv(s,rBUF);
 if(!TS(rBUF+1))CAF(ZERO);
 XCH(rL);ai_dv(s,rBUF);
 if(!TS(rL))CA(rBUF+1);
 DAS(rMPAC);
 if(ai_bzf(s)){RETQ();return 0;}
 CAF(POSMAX);goto fixroot;
srtest:
 ADF(QUARTER);
 if(ai_bzmf(s))goto sqrtnorm;
 DXCH(rMPAC);LXCH(rSR);MP_HALF();DXCH(rMPAC);XCH(rSR);ADS(rMPAC+1);
arglo:
 CAF(SLOPELO);MP(rMPAC);ADF(BIASLO);goto arghi4;
sqrtnm2:
 DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);
sqrtnorm:
 INCR(rMPTEMP);DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);DOUBLE();
 /* TS CYL; NORMTEST CCS CYL; CCS CYL (each CCS cycles CYL again). */
 c=TS(rCYL)?0u:CCS(rCYL);
 if(c==0u){
  c=CCS(rCYL);
  if(c==0u)goto sqrtnm2;
  if(c==1u)goto arghi;
  goto arglo;                  /* (-0 would run on into COSINE) */
 }
 if(c==1u)goto sqrtnm2;
 if(c==2u)goto arghi;
 goto arglo;
}
/* SQRT 03207 to DANZIG, or 1 at POODOO1 (alarm 1302). */
static uint8_t sqrt_body(apollo_overlay_state *s){
 uint8_t c;
 TC(03210);if(sqrtsub(s))return 1;
 c=CCS(rMPTEMP);
 if(c==1u){danzig(s);return 0;}
 if(c!=3u){
  ADF(NEG12);
  if(!ai_bzmf(s)){ZL();LXCH(rADDRWD);genscr1(s,1);return 0;}
 }
 /* SQRTSHFT INDEX MPTEMP; CAF BIT15; TS MPTEMP; CAF ZERO; MPACSHR +3. */
 CAF(fixed_indexed(BIT15_AT,W[rMPTEMP]));(void)TS(rMPTEMP);CAF(ZERO);mpacshr3(s);return 0;
}
uint8_t lmi_sqrt(apollo_overlay_state *s){return sqrt_body(s);}
/* ABVALABS 03176: ABS on a scalar, ABVAL (length, LVSQUARE kept) on a vector. */
uint8_t lmi_abvalabs(apollo_overlay_state *s){
 if(CCS(rMODE)<2u){lmi_abs(s);return 0;}
 TC(03202);vsqsub(s);LXCH(rMODE);DCA(rMPAC);DXCH(AT(W[rFIXLOC]+tLVSQUARE));
 return sqrt_body(s);
}

/* ---- UNIT (bank 0) ---------------------------------------------------- */
/* ALSIGNAG 07544: sign agreement of A,L, to TC Q. */
static void alsignag(apollo_overlay_state *s){
 switch(CCS(rA)){
 case 0:XCH(rL);ADF(HALF);ADF(HALF);if(TS(rA))INCR(rL);XCH(rL);RETQ();return;
 case 2:XCH(rL);ADF(BIT15);ADF(NEGONE);if(TS(rA))INCR(rL);XCH(rL);COM();RETQ();return;
 default:RETQ();return;
 }
}
/* VECAGREE 03010 (Q = return): ends with TC A, leaving Q = 1. */
static void vecagree(apollo_overlay_state *s){
 XCH(rQ);
 DXCH(rMPAC);TC(03013);alsignag(s);DXCH(rMPAC);
 DXCH(rMPAC+3);TC(03016);alsignag(s);DXCH(rMPAC+3);
 DXCH(rMPAC+5);TC(03021);alsignag(s);DXCH(rMPAC+5);
 ai_tc(s,1u);
}
/* MPACVBUF 07532 to TC Q. */
static void mpacvbuf(apollo_overlay_state *s){
 DCA(rMPAC);DXCH(rVBUF);DCA(rMPAC+3);DXCH(rVBUF+2);DCA(rMPAC+5);DXCH(rVBUF+4);RETQ();
}
/* UNITDV 03151 (divisor in BUF, MPTEMP the pre-divide count) to TC Q. */
static void unitdv(apollo_overlay_state *s){
 uint8_t c=CCS(rMPAC);
 if(c==1u||c==3u){
  c=CCS(rMPAC+1);
  if(c==1u||c==3u){RETQ();return;}
 }
 if(c==2u){CSF(ZERO);(void)TS(rDVSIGN);DCS(rMPAC);}
 else{(void)TS(rDVSIGN);DXCH(rMPAC);}
 ddv_chain(s,chain_at(02564u,W[rMPTEMP]));
}
/* UNIT 03023 to DANZIG (or DVOVF's TC DANZIG); 1 at POODOO1. */
uint8_t lmi_unit(apollo_overlay_state *s){
 uint8_t c;
 TC(03024);vecagree(s);TC(03025);mpacvbuf(s);
 CAF(ZERO);XCH(rOVFIND);(void)TS(rTEM1);TC(03031);vsqsub(s);CA(rTEM1);XCH(rOVFIND);
 if(!ai_bzf(s)){dvovf(s);return 0;}
 DCA(rMPAC);DXCH(AT(W[rFIXLOC]+tLVSQUARE));
 TC(03043);if(sqrtsub(s))return 1;
 c=CCS(rMPAC);
 if(c!=0u){
  /* +0 TS L; - INDEX FIXLOC; -0 (unindexed) DXCH LV; TCF DVOVF. */
  if(c==1u)(void)TS(rL);
  DXCH(c==3u?AT(tLV):AT(W[rFIXLOC]+tLV));dvovf(s);return 0;
 }
 CSF(016u);AD(rMPTEMP);c=CCS(rA);
 if(c==0u)COM();
 if(c<2u){
  /* SMALL TS MPTEMP; shift VBUF left 14; INDEX A (-MPTEMP); CAF BIT14; MP MPAC. */
  (void)TS(rMPTEMP);CAF(ZERO);
  XCH(rVBUF+1);XCH(rVBUF);XCH(rVBUF+3);XCH(rVBUF+2);XCH(rVBUF+5);XCH(rVBUF+4);
  CS(rMPTEMP);CAF(fixed_indexed(BIT14_AT,W[rA]));MP(rMPAC);
  goto small2;
 }
 if(c==3u){CSF(015u);(void)TS(rMPTEMP);CA(rMPAC);goto small2;}
 /* LARGE CCS MPTEMP. */
 c=CCS(rMPTEMP);
 if(c==0u){
  /* LARGE3 COM; TS MPTEMP; COM; INDEX A; CAF BIT14 ... unnormalize. */
  COM();(void)TS(rMPTEMP);COM();CAF(fixed_indexed(BIT14_AT,W[rA]));(void)TS(rBUF);
  MP(rMPAC+1);XCH(rBUF);MP(rMPAC);XCH(rL);AD(rBUF);XCH(rL);goto large2;
 }
 if(c==1u)CSF(024u);           /* CS SRDDV */
 if(c<3u)(void)TS(rMPTEMP);
 DCA(rMPAC);goto large2;
small2:
 (void)TS(rL);CAF(ZERO);
large2:
 DXCH(AT(W[rFIXLOC]+tLV));CSF(ONE);(void)TS(rMAXDVSW);
 DXCH(rVBUF);DXCH(rMPAC);DXCH(rBUF);TC(03122);unitdv(s);
 DXCH(rVBUF+2);DXCH(rMPAC);DXCH(rMPAC+3);TC(03126);unitdv(s);
 DXCH(rVBUF+4);DXCH(rMPAC);DXCH(rMPAC+5);TC(03132);unitdv(s);
 vrotatex(s);return 0;
}

/* ---- Vector times scalar and cross product (bank 3) ------------------- */
static const agc_word *kat(const agc_word *k,unsigned i){return k?k+i:0;}
/* DVXSC 07430 (Comanche 07377): scalar in MPAC, vector operand at C(ADDRWD), to DANZIG. */
static void dvxsc(apollo_overlay_state *s,const agc_word *k){
 DCA(rMPAC);DXCH(rMPAC+3);TC(IF_(07434,07403));dmpsub(s,k);TC(IF_(07435,07404));roundsub(s,2);
 CAF(TWO);ADS(rADDRWD);DCA(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+5);TC(IF_(07444,07413));dmpsub(s,kat(k,2));TC(IF_(07445,07414));roundsub(s,2);
 CAF(TWO);ADS(rADDRWD);DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+3);TC(IF_(07453,07422));dmpsub(s,kat(k,4));TC(IF_(07454,07423));roundsub(s,2);
 DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rMPAC);vmode(s);
}
void lmi_vxsc(apollo_overlay_state *s,const agc_word *k){
 uint8_t c=CCS(rMODE);
 if(c<2u){dvxsc(s,k);return;}
 /* VVXSC (-0 enters at its TC VROUND). */
 if(c==2u){TC(IF_(07405,07354));dmpsub(s,k);}
 TC(IF_(07406,07355));roundsub(s,2);
 DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+3);TC(IF_(07412,07361));dmpsub(s,k);TC(IF_(07413,07362));roundsub(s,2);
 DXCH(rMPAC+5);DXCH(rMPAC);DXCH(rMPAC+5);TC(IF_(07417,07366));dmpsub(s,k);TC(IF_(07420,07367));roundsub(s,2);
 vrotatex(s);
}
/* VPROJ 07425 (Comanche 07374): TC PREDOT (DOTINC 2, DOTSUB); CS FOUR; ADS ADDRWD back to
 * the operand; DVXSC. */
void lmi_vproj(apollo_overlay_state *s,const agc_word *k){
 TC(IF_(07426,07375));CAF(TWO);(void)TS(rDOTINC);dotsub(s,k);CSF(FOUR);ADS(rADDRWD);dvxsc(s,k);
}
void lmi_vxv(apollo_overlay_state *s,const agc_word *k){
 DCA(rMPAC+5);DXCH(rMPAC);DXCH(rVBUF);TC(IF_(07465,07434));dmpsub(s,k);
 DCS(rMPAC+3);DXCH(rMPAC);DXCH(rVBUF+2);TC(IF_(07472,07441));dmpsub(s,k);
 CAF(TWO);ADS(rADDRWD);
 DCS(rMPAC+5);DXCH(rMPAC);DXCH(rMPAC+5);TC(IF_(07501,07450));dmpsub(s,kat(k,2));
 DCA(rVBUF);DXCH(rMPAC);DXCH(rVBUF+4);TC(IF_(07506,07455));dmpsub(s,kat(k,2));
 CAF(TWO);ADS(rADDRWD);
 DCS(rVBUF);DXCH(rMPAC);DAS(rMPAC+5);
 if(!ai_bzf(s)){TC(IF_(07517,07466));overflwz(s);}
 TC(IF_(07520,07467));dmpsub(s,kat(k,4));
 DXCH(rVBUF+2);DXCH(rMPAC+3);DXCH(rMPAC);DAS(rMPAC+3);
 if(!ai_bzf(s)){TC(IF_(07527,07476));overflwy(s);}
 TC(IF_(07530,07477));dmpsub(s,kat(k,4));DXCH(rVBUF+4);endvxv(s);
}

/* ---- BOV(B), RTB and POLY ----------------------------------------------- */
/* BPL/BMN 01,2474: CCS CYR; positive TCF BPL (TC BRANCH; + or zero GOTO),
 * negative BMN (TC BRANCH; only negative GOTO), -0 DANZIG. Returns 1 at GOTO 06646 (the
 * caller continues with lmi_goto), 0 at DANZIG. */
uint8_t lmi_bplbmn(apollo_overlay_state *s){
 uint8_t c=CCS(rCYR),b;
 if(c==0u){TC(IF_(02504,02531));b=branch(s);if(b!=2u){W[5]=IF_(06646,06615);return 1;}danzig(s);return 0;}
 if(c==3u){danzig(s);return 0;}
 TC(IF_(02500,02525));b=branch(s);if(b==2u){W[5]=IF_(06646,06615);return 1;}danzig(s);return 0;
}
uint8_t lmi_bov(apollo_overlay_state *s){
 uint8_t c=CCS(rOVFIND);
 if(c==1u){danzig(s);return LMI_BOV_NONE;}
 if(c!=3u)(void)TS(rOVFIND);
 c=CCS(rCYR);
 if(c==0u){W[5]=IF_(02450,02475);return LMI_BOV_RTB;}
 if(c==2u){W[5]=IF_(06646,06615);return LMI_BOV_GOTO;}
 W[5]=(agc_word)(c==1u?IF_(02465u,02512u):IF_(02467u,02514u));return LMI_BOV_OUT_OF_SCOPE;
}
void lmi_rtb(apollo_overlay_state *s){
 /* RTB CA POLISH; TC SWCALL -1: INCR Q; TS L; LXCH FBANK; MASK LOW10;
  * XCH Q; DXCH BUF2; INDEX Q; TC 2000 (Q = SWRETURN). */
 unsigned sub;
 CA(rPOLISH);TC(IF_(02452,02477));INCR(rQ);(void)TS(rL);LXCH(rFB);MASKF(LOW10);XCH(rQ);DXCH(rBUF2);
 sub=W[rQ]&01777u;TC(IF_(04631,04675));W[5]=(agc_word)(02000u+sub);
}
void lmi_swreturn(apollo_overlay_state *s){
 /* SWRETURN XCH BUF2 +1; XCH FBANK; XCH BUF2 +1; TC BUF2 (the TC held
  * there, leaving Q = BUF2 +1, reaches RTB's TCF DANZIG). */
 XCH(rBUF2+1);XCH(rFB);XCH(rBUF2+1);ai_tc(s,rBUF2+1u);danzig(s);
}
void lmi_poly(apollo_overlay_state *s,const agc_word *table){
 agc_word q=(agc_word)(W[rQ]&077777u);
 CAF(table[0]);(void)TS(rPOLYCNT);DOUBLE();AD(rQ);(void)TS(rPOLISH);ADF(FIVE);(void)TS(rPOLYRET);
 CAF(LVBUF);(void)TS(rADDRWD);
 ai_dcaf(s,table+(W[rPOLISH]-q)+3u);DXCH(rMPAC);DXCH(rVBUF);
 for(;;){
  TC(IF_(07246,07215));dmpsub(s,0);ai_dcaf(s,table+(W[rPOLISH]-q)+1u);DAS(rMPAC);
  if(CCS(rPOLYCNT)!=0u)break;
  (void)TS(rPOLYCNT);CSF(TWO);ADS(rPOLISH);
 }
 /* TC POLYRET: the TC held there, leaving Q = 0142. */
 W[5]=(agc_word)(W[rPOLYRET]&07777u);ai_tc(s,rPOLYRET+1u);
}
/* ---- VAD/VSU, BVSU, TAD, ROUND, SSP --------------------------------------- */
/* VSU 06747 / VAD 06751: ADS ADDRWD with BIT15 or PRIO30 turns the indexed
 * READs into DCS or DCA of the operand's +2, +4 and +0 words; ADDRWD keeps
 * the sum. Ends through ENDVXV at DANZIG. */
static void vaddsub(apollo_overlay_state *s,const agc_word *k,uint8_t sub){
 unsigned a=W[rADDRWD];unsigned i;static const unsigned off[3]={2u,4u,0u};
 if(sub)CAF(BIT15);else CAF(030000u);
 ADS(rADDRWD);
 for(i=0;i<3u;++i){
  if(sub){if(k)ai_dcsf(s,k+off[i]);else DCS(AT(a+off[i]));}
  else{if(k)ai_dcaf(s,k+off[i]);else DCA(AT(a+off[i]));}
  if(i==2u)break;
  DAS(AT(rMPAC+3u+2u*i));
  if(!ai_bzf(s)){if(i==0u){TC(IF_(06762,06731));overflwy(s);}else{TC(IF_(06771,06740));overflwz(s);}}
 }
 endvxv(s);
}
void lmi_vad(apollo_overlay_state *s,const agc_word *k){vaddsub(s,k,0);}
void lmi_vsu(apollo_overlay_state *s,const agc_word *k){vaddsub(s,k,1);}
void lmi_bvsu(apollo_overlay_state *s,const agc_word *k){
 dca_op(s,k,2);DXCH(rMPAC+3);ai_dcom(s);DAS(rMPAC+3);
 if(!ai_bzf(s)){TC(IF_(07050,07017));overflwy(s);}
 dca_op(s,k,4);DXCH(rMPAC+5);ai_dcom(s);DAS(rMPAC+5);
 if(!ai_bzf(s)){TC(IF_(07062,07031));overflwz(s);}
 dca_op(s,k,0);DXCH(rMPAC);ai_dcom(s);endvxv(s);
}
void lmi_tad(apollo_overlay_state *s,const agc_word *k){
 /* TAD 07071: DCA 1; DAS MPAC +1; AD 0; AD MPAC; TS MPAC (skip on
  * overflow to TCF SETOVF). */
 dca_op(s,k,1);DAS(rMPAC+1);
 if(k)ADF(k[0]);else AD(AT(W[rADDRWD]));
 AD(rMPAC);
 if(!TS(rMPAC)){danzig(s);return;}
 TC(IF_(07004,06753));(void)overflow(s);danzig(s);
}
void lmi_round(apollo_overlay_state *s){
 /* ROUND 02116 (bank 0): TC ROUNDSUB; TCF DANZIG. */
 TC(02117);roundsub(s,0);danzig(s);
}
void lmi_ssp(apollo_overlay_state *s,agc_word word){
 /* SSP 06620: INCR LOC; INDEX LOC, CA 0 (the word given); STORE1 INDEX
  * ADDRWD, TS 0; TCF DANZIG. */
 INCR(rLOC);CAF(word);(void)TS(AT(W[rADDRWD]));danzig(s);
}

/* ---- Index orders, BHIZ, CALL, indirect GOTO, EXIT and INTPRET ------------ */
#define rINDEXLOC 0130u
/* 15ADRERS 01,2427 into TAGSUB and back through TC Q. */
static void adrers15(apollo_overlay_state *s){
 uint8_t c;
 CS(rPOLISH);ADF(DEC45);c=CCS(rA);
 if(c==0u){CA(rFIXLOC);}
 else if(c>=2u){if(c==2u)CAF(OCT1400);XCH(rPOLISH);(void)TS(rEB);MASKF(LOW8);}
 ADS(rPOLISH);
 CA(rFIXLOC);(void)TS(rINDEXLOC);
 if(CCS(rCYR)==0u)INCR(rINDEXLOC);
 RETQ();
}
static void xad2(apollo_overlay_state *s){ADS(AT(W[rINDEXLOC]+tX1));danzig(s);}
void lmi_xad(apollo_overlay_state *s){TC(IF_(02401,02426));adrers15(s);CA(AT(W[rPOLISH]));xad2(s);}
void lmi_xsu(apollo_overlay_state *s){TC(IF_(02412,02437));adrers15(s);CS(AT(W[rPOLISH]));xad2(s);}
void lmi_lxa(apollo_overlay_state *s){TC(IF_(02355,02402));adrers15(s);CA(AT(W[rPOLISH]));(void)TS(AT(W[rINDEXLOC]+tX1));danzig(s);}
void lmi_sxa(apollo_overlay_state *s){TC(IF_(02365,02412));adrers15(s);CA(AT(W[rINDEXLOC]+tX1));(void)TS(AT(W[rPOLISH]));danzig(s);}
void lmi_ita(apollo_overlay_state *s){
 /* CALL/ITA CCS CYR (negative: ITA/STQ); TC 15ADRERS; INDEX FIXLOC,
  * CA QPRET; MSTORE1 INDEX POLISH, TS 0; TCF DANZIG. */
 (void)CCS(rCYR);TC(IF_(02513,02540));adrers15(s);CA(AT(W[rFIXLOC]+tQPRET));(void)TS(AT(W[rPOLISH]));danzig(s);
}
uint8_t lmi_tix(apollo_overlay_state *s){
 /* TIX 01,2415: TC TAGSUB; INDEX INDEXLOC, CS S1; INDEX INDEXLOC, AD X1;
  * EXTEND, BZMF DANZIG; DOTIXBR INDEX INDEXLOC, XCH X1; TCF GOTO 06646. */
 TC(IF_(02416,02443));CA(rFIXLOC);(void)TS(rINDEXLOC);if(CCS(rCYR)==0u)INCR(rINDEXLOC);RETQ();
 CS(AT(W[rINDEXLOC]+tS1));AD(AT(W[rINDEXLOC]+tX1));
 if(ai_bzmf(s)){danzig(s);return 0;}
 XCH(AT(W[rINDEXLOC]+tX1));W[5]=IF_(06646,06615);return 1;
}
uint8_t lmi_bhiz(apollo_overlay_state *s){
 /* BHIZ 01,2452: CCS MPAC; + and - DANZIG, +0 and -0 GOTO 06646. */
 uint8_t c=CCS(rMPAC);
 if(c==1u||c==3u){W[5]=IF_(06646,06615);return 1;}
 danzig(s);return 0;
}
void lmi_call(apollo_overlay_state *s){
 /* CALL/ITA CCS CYR (positive) TCF CALL; CALL CA BANKSET; MASK BANKMASK;
  * AD BANKMASK; AD LOC; INDEX FIXLOC, TS QPRET; GOTO (fixed target). */
 (void)CCS(rCYR);CA(rBANKSET);MASKF(076000u);ADF(076000u);AD(rLOC);(void)TS(AT(W[rFIXLOC]+tQPRET));
 W[5]=IF_(06646,06615);
}
void lmi_goto_any(apollo_overlay_state *s){
 /* GOTO CA POLISH; MASK HIGH4; BZF GOTOERS (an erasable CADR is fetched,
  * through the work area or the EBANK it names, until a fixed one). */
 for(;;){
  uint8_t c;
  CA(rPOLISH);MASKF(074000u);
  if(!ai_bzf(s))break;
  CA(rPOLISH);ADF(ENDVAC_NEG);c=CCS(rA);
  if(c==0u||c==1u){
   if(c==0u)CA(rPOLISH);
   (void)TS(rEB);MASKF(LOW8);CA(AT(01400u+W[rA]));
  }else{
   if(c==2u)CA(rFIXLOC);
   AD(rPOLISH);CA(AT(W[rA]));
  }
  (void)TS(rPOLISH);
 }
 lmi_goto(s);
}
void lmi_15bitadr(apollo_overlay_state *s,agc_word word,agc_word target){
 /* 15BITADR 06252 (from STCALL's STOTC): INCR LOC; INDEX LOC, CA 0;
  * TS POLISH; CAF LOW7+2K; TS FBANK; MASK CYR; INDEX A, TCF MISCJUMP. */
 INCR(rLOC);CAF(word);(void)TS(rPOLISH);CAF(02177u);(void)TS(rFB);MASK(rCYR);W[5]=target;
}
uint8_t lmi_bzegoto(apollo_overlay_state *s){
 /* BZE/GOTO 01,2467: CCS CYR (positive) TC BRANCH; zero TCF GOTO (1, at
  * GOTO 06646 for lmi_goto or lmi_goto_any), else DANZIG (0); GOTO when
  * CYR is not positive (1). */
 if(CCS(rCYR)!=0u){W[5]=IF_(06646,06615);return 1;}
 TC(IF_(02471,02516));
 if(branch(s)==1u){W[5]=IF_(06646,06615);return 1;}
 danzig(s);return 0;
}
void lmi_exit(apollo_overlay_state *s){
 /* EXIT 06743: CA BANKSET; TS BBANK; INDEX LOC, TC 1 (to the basic code
  * after the order; Q = 06747). */
 unsigned loc;
 CA(rBANKSET);(void)TS(rBB);loc=W[rLOC];TC(IF_(06747,06716));W[5]=(agc_word)((loc+1u)&07777u);
}
void lmi_intpret(apollo_overlay_state *s,p66_bias_io *io){
 /* TC INTPRET (Q = the word after the TC): RELINT; QXCH LOC; CA BBANK;
  * TS BANKSET; MASK BIT15; TS INTBIT15; TS EDOP; TCF NEWOPS. */
 io->interrupt_enabled=1;ai_qxch(s,rLOC);CA(rBB);(void)TS(rBANKSET);MASKF(BIT15);(void)TS(rINTBIT15);(void)TS(rEDOP);W[5]=IF_(06070,06037);
}
void lmi_tload(apollo_overlay_state *s){
 /* TLOAD 06470 (erasable operand at ADDRWD): INDEX ADDRWD, CA 2; TS MPAC+2;
  * EXTEND; INDEX ADDRWD, DCA 0; DXCH MPAC; TMODE CAF ONE; TCF NEWMODE. */
 CA(AT(W[rADDRWD]+2u));(void)TS(rMPAC+2);DCA(AT(W[rADDRWD]));DXCH(rMPAC);CAF(ONE);newmode(s);
}
/* TLOAD with a fixed operand: the same path, the three words passed in k. */
void lmi_tload_fixed(apollo_overlay_state *s,const agc_word *k){
 CAF(k[2]);(void)TS(rMPAC+2);ai_dcaf(s,k);DXCH(rMPAC);CAF(ONE);newmode(s);
}
void lmi_tpmode(apollo_overlay_state *s){
 /* TPMODE (RTB_OP_CODES, bank 10): CAF ONE; TCF NEWMODE. */
 CAF(ONE);newmode(s);
}
#undef W
