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
#undef W
