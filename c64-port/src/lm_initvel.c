/* SPDX-License-Identifier: MIT */
/* Native Luminary099 INITVEL (Luminary099/P34-35_P74-75.agc, bank 11
 * 02000-02274) with SETITCTR (Luminary099/ORBITAL_INTEGRATION.agc,
 * 11,3765-3775) compiled inline at its CALL. LAMBERT and the conic steps it
 * reaches are the lm-conic-lambert milestone's (src/lm_conic_lambert.c);
 * INTSTALL and INTEGRVS (bank 13) are not ported, and their NEWOPS is an
 * end frontier. Every interpretive pair is compiled: its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * the lm_interp_steps and lm_interp_orders paths, and each runtime branch
 * follows the outcome those paths return. MUEARTH (bank 13), ZEROVEC (bank
 * 12) and MUTABLE (bank 4) are read from the certified fixed-data table. */
#include "lm_initvel.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "sband_trig.h"
#ifdef LM_INITVEL_TRACE
void lm_initvel_trace(const apollo_overlay_state *);
#define TRACE(s) lm_initvel_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,DLOAD=06052,SLOAD=06501,DAD=06775,DMP=07572,DDV=07577,GSHIFT=07621,
 VXV=07460,DOT=07331,VPROJ=07425,VXSC=07401,BVSU=07036,VSU=06747,VAD=06751,TLOAD=06470,
 SETPD=07613,SSP=06620,COMP=07670,UNIT=03023,ABVAL=03176,PUSH=03247,COSINE=03517,
 LXA=02354,SXA=02364,INCR=02406,XSU=02411,CALLITA=02507,RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,SWITCH=02516};

/* The certified fixed words that ADDRWD names after address formation (a
 * fixed-fixed address, or the FBANK the formation switched to). */
static uint8_t fixed_operand(const apollo_overlay_state *s,agc_word *k,unsigned n){
 unsigned i,a=W[0116],bank;
 if(a>=04000u&&a+n<=010000u){bank=a>>10;a&=01777u;}
 else if(a>=02000u&&a+n<=04000u){bank=W[4]>>10;a-=02000u;}
 else return 0;
 if(bank>=030u)return 0;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(LM,(uint16_t)(bank*1024u+a+i),&k[i]))return 0;
 return 1;
}

static void next(apollo_overlay_state *s){lmi_danzig_next(s);TRACE(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
/* STOVL/STODL: the store, then the load of the given word (PUSHUP when it
 * is an order pair). */
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,VLOAD);else lmi_diradres(s,word,VLOAD);
 lmi_vload(s,0);next(s);
}
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,DLOAD);else lmi_diradres(s,word,DLOAD);
 lmi_dload_op(s,0);next(s);
}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
static uint8_t unit(apollo_overlay_state *s){lmi_unary(s,UNIT);return lmi_unit(s);}
static uint8_t abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);return lmi_abvalabs(s);}
static void branch_taken(apollo_overlay_state *s){lmi_goto(s);TRACE(s);}
static void go(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);lmi_goto_order(s);TRACE(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);branch_taken(s);}
/* STCALL to a fixed label. */
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);branch_taken(s);
}
static uint8_t bplbmn(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,BPLBMN);
 if(!lmi_bplbmn(s))return 0;
 branch_taken(s);return 1;
}
static uint8_t bhiz(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,RTBBHIZ);(void)ai_ccs(s,020);
 if(!lmi_bhiz(s))return 0;
 branch_taken(s);return 1;
}
/* A switch order (word = its switch/operation word): 1 when it branches. */
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
/* LXA, SXA, INCR and XSU (word = the address or increment). */
static void lxa(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,LXA);lmi_lxa(s);}
static void sxa(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,SXA);lmi_sxa(s);}
static void incr(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,INCR);lmi_incr(s);}
static void sload(apollo_overlay_state *s,agc_word word){lmi_direct(s,word,SLOAD);lmi_sload(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
/* SETPD 0D as the left order of a pair. */
static void setpd0(apollo_overlay_state *s){lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);}
/* STORE RTMAG after ABVAL (HAVEGUES twice). */
static uint8_t abval_rtmag(apollo_overlay_state *s){
 lmi_newops(s,077646);if(abval(s))return 1;next(s);
 store(s,003723);
 return 0;
}

uint8_t liv_initvel(apollo_overlay_state *s,p66_bias_io *io){
 /* INITVEL SET GUESSW; then HAVEGUES. */
 lmi_newops(s,077614);if(sw(s,io,000475))return LTT_FAILED;next(s);
 return liv_havegues(s,io);
}
uint8_t liv_havegues(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* HAVEGUES VLOAD STQ RTARG NORMEX; STORE RTARG1; ABVAL; STORE RTMAG;
  * SLOAD BHIZ RTX2 INITVEL1. */
 lmi_newops(s,044175);vload(s,003444);right(s);lmi_misc(s,003463,CALLITA);lmi_ita(s);next(s);
 store(s,003472);
 if(abval_rtmag(s))return LTT_ABORTED;
 lmi_newops(s,046135);sload(s,003000);right(s);
 if(bhiz(s,022026))return LIV_INITVEL1;
 next(s);
 /* VLOAD VSL2 RINIT; STOVL RINIT VINIT; VSL2; STOVL VINIT RTARG1; VSL2;
  * STORE RTARG1; ABVAL; STORE RTMAG. */
 lmi_newops(s,072575);vload(s,002323);right(s);shorts(s);next(s);
 stovl(s,026323,002331);
 lmi_newops(s,077752);shorts(s);next(s);
 stovl(s,026331,003472);
 lmi_newops(s,077752);shorts(s);next(s);
 store(s,003472);
 if(abval_rtmag(s))return LTT_ABORTED;
 return LIV_INITVEL1;
}
uint8_t liv_initvel1(apollo_overlay_state *s,p66_bias_io *io){
 /* INITVEL1 SSP DLOAD ITCTR 0 -1 (the load pushes up); COSINE SR1;
  * STODL COZY4 (pushes up); LXA,2 SXA,2 MPAC VTARGTAG. */
 lmi_newops(s,071331);lmi_direct(s,003617,SSP);lmi_ssp(s,077776);right(s);opnd(s,070546,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,070546);lmi_unary(s,COSINE);if(!sband_trig(s,LM,1))return LTT_FAILED;right(s);shorts(s);next(s);
 stodl(s,017667,067154);
 lmi_newops(s,067154);lxa(s,000154);right(s);sxa(s,002701);next(s);
 /* VLOAD RINIT; STOVL R1VEC RTARG1; STODL R2VEC DELLT4; STORE TDESIRED. */
 lmi_newops(s,077775);vload(s,002323);next(s);
 stovl(s,026655,003472);
 stodl(s,016663,003452);
 store(s,002671);
 /* SETPD VLOAD 0D RINIT; UNIT PUSH; VXV UNIT VINIT; STOVL UN RTARG1;
  * UNIT DOT (pushes up); DAD CLEAR COZY4 NORMSW; STORE COZY4. */
 lmi_newops(s,077201);setpd0(s);vload(s,002323);next(s);
 lmi_newops(s,041456);if(unit(s))return LTT_ABORTED;right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,053435);opnd(s,002331,VXV);lmi_vxv(s,0);right(s);if(unit(s))return LTT_ABORTED;next(s);
 stovl(s,026674,003472);
 lmi_newops(s,050256);if(unit(s))return LTT_ABORTED;right(s);opnd(s,043015,DOT);lmi_dot(s,0);next(s);
 lmi_newops(s,043015);opnd(s,003667,DAD);lmi_dad(s,0);right(s);if(sw(s,io,003665))return LTT_FAILED;next(s);
 store(s,003667);
 return LIV_INITVEL2;
}
uint8_t liv_initvel2(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[3];
 /* INITVEL2 BPL SET INITVEL3 NORMSW. */
 lmi_newops(s,043044);if(bplbmn(s,022105))return LIV_INITVEL3;
 right(s);if(sw(s,io,003465))return LTT_FAILED;next(s);
 /* VLOAD PUSH R2VEC; ABVAL PDVL (pushes up); PUSH VPROJ UN; VSL2 BVSU
  * (pushes up); UNIT VXSC (pushes up); VSL1; STORE R2VEC. */
 lmi_newops(s,041575);vload(s,002663);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,063246);if(abval(s))return LTT_ABORTED;right(s);opnd(s,046206,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,046206);unary(s,PUSH,lmi_push);right(s);opnd(s,002674,VPROJ);lmi_vproj(s,0);next(s);
 lmi_newops(s,051352);shorts(s);right(s);opnd(s,074256,BVSU);lmi_bvsu(s,0);next(s);
 lmi_newops(s,074256);if(unit(s))return LTT_ABORTED;right(s);opnd(s,077772,VXSC);lmi_vxsc(s,0);next(s);
 lmi_newops(s,077772);shorts(s);next(s);
 store(s,002663);
 /* TLOAD SLOAD ZEROVEC ITCTR; BPL VLOAD INITVEL3 R2VEC; STORE RTARG1. */
 lmi_newops(s,067351);opnd(s,024007,TLOAD);if(!fixed_operand(s,k,3))return LTT_FAILED;lmi_tload_fixed(s,k);right(s);sload(s,003617);next(s);
 lmi_newops(s,077244);if(bplbmn(s,022105))return LIV_INITVEL3;
 right(s);vload(s,002663);next(s);
 store(s,003472);
 return LIV_INITVEL3;
}
uint8_t liv_initvel3(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[2];
 /* INITVEL3 DLOAD PDVL MUEARTH R2VEC; UNIT PDVL R1VEC; UNIT PUSH; VXV
  * VCOMP 2D; PUSH; LXA,1 DLOAD RTX1 18D. */
 lmi_newops(s,063345);opnd(s,026007,DLOAD);if(!fixed_operand(s,k,2))return LTT_FAILED;lmi_dload_op(s,k);right(s);opnd(s,002663,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,063256);if(unit(s))return LTT_ABORTED;right(s);opnd(s,002655,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,041456);if(unit(s))return LTT_ABORTED;right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,057435);opnd(s,000003,VXV);lmi_vxv(s,0);right(s);unary(s,COMP,lmi_comp);next(s);
 lmi_newops(s,077606);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,071350);lxa(s,002776);right(s);opnd(s,000023,DLOAD);lmi_dload_op(s,0);next(s);
 /* BMN INCR,1 +2 DEC -8; +2 INCR,1 SLOAD 10D X1; BHIZ VLOAD +2 (pushes
  * up); VCOMP PUSH; +2 VLOAD (pushes up). */
 lmi_newops(s,062040);
 if(!bplbmn(s,022124)){right(s);incr(s,077767);next(s);}
 lmi_newops(s,067310);incr(s,000012);right(s);sload(s,000047);next(s);
 lmi_newops(s,077230);
 if(!bhiz(s,022132)){
  right(s);vload(s,041476);next(s);
  lmi_newops(s,041476);unary(s,COMP,lmi_comp);right(s);unary(s,PUSH,lmi_push);next(s);
 }
 lmi_newops(s,077775);vload(s,050235);next(s);
 /* VXV DOT (both push up); BPL DLOAD INITVEL4 (pushes up); DCOMP PUSH. */
 lmi_newops(s,050235);opnd(s,071244,VXV);lmi_vxv(s,0);right(s);opnd(s,071244,DOT);lmi_dot(s,0);next(s);
 lmi_newops(s,071244);
 if(!bplbmn(s,022137)){
  right(s);opnd(s,041476,DLOAD);lmi_dload_op(s,0);next(s);
  lmi_newops(s,041476);unary(s,COMP,lmi_comp);right(s);unary(s,PUSH,lmi_push);next(s);
 }
 /* INITVEL4 LXA,2 SXA,2 0D GEOMSGN; LXA,1 CALL RTX1 SETITCTR. */
 lmi_newops(s,067154);lxa(s,000000);right(s);sxa(s,002672);next(s);
 lmi_newops(s,045150);lxa(s,002776);right(s);call(s,023765);
 /* SETITCTR SSP BOFF ITERCTR 20D AVEGFLAG LAMBERT; SSP GOTO ITERCTR 5
  * LAMBERT. */
 lmi_newops(s,043131);lmi_direct(s,000027,SSP);lmi_ssp(s,000024);right(s);
 if(sw(s,io,003752)){lmi_swbranch(s,025212);TRACE(s);return LLA_LAMBERT;}
 next(s);
 lmi_newops(s,052131);lmi_direct(s,000027,SSP);lmi_ssp(s,000005);right(s);go(s,025212);
 return LLA_LAMBERT;
}
uint8_t liv_afterlambert(apollo_overlay_state *s,p66_bias_io *io){
 /* CLEAR VLOAD GUESSW VVEC; STODL VIPRIME VTARGTAG; BHIZ CALL INITVEL7
  * INTSTALL. */
 lmi_newops(s,077214);if(sw(s,io,000675))return LTT_FAILED;right(s);vload(s,002744);next(s);
 stodl(s,016337,002702);
 lmi_newops(s,045030);
 if(bhiz(s,022230))return LIV_INITVEL7;
 right(s);call(s,027414);
 return LIV_EXTERNAL;
}
uint8_t liv_afterintstall(apollo_overlay_state *s,p66_bias_io *io){
 /* SLOAD CLEAR RTX2 MOONFLAG; BHIZ SET INITVEL5 MOONFLAG. */
 lmi_newops(s,043135);sload(s,003000);right(s);if(sw(s,io,000263))return LTT_FAILED;next(s);
 lmi_newops(s,043030);
 if(!bhiz(s,022163)){right(s);if(sw(s,io,000063))return LTT_FAILED;next(s);}
 /* INITVEL5 VLOAD RINIT; STORE R1VEC; STOVL RCV VIPRIME; STODL VCV
  * INTIME; STORE TET; DAD CLEAR DELLT4 INTYPFLG; STCALL TDEC1 INTEGRVS. */
 lmi_newops(s,077775);vload(s,002323);next(s);
 store(s,002655);
 stovl(s,025535,002337);
 stodl(s,015543,003610);
 store(s,001517);
 lmi_newops(s,043015);opnd(s,003452,DAD);lmi_dad(s,0);right(s);if(sw(s,io,001673))return LTT_FAILED;next(s);
 stcall(s,034041,027107);
 return LIV_EXTERNAL;
}
uint8_t liv_afterintegrvs(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* VLOAD VATT1; STORE VTARGET; LXA,2 INCR,2 ITCTR 1D; SXA,2 XSU,2 ITCTR
  * VTARGTAG; SLOAD BHIZ X2 INITVEL6. */
 lmi_newops(s,077775);vload(s,000025);next(s);
 store(s,002703);
 lmi_newops(s,063154);lxa(s,003616);right(s);incr(s,000001);next(s);
 lmi_newops(s,055134);sxa(s,003616);right(s);lmi_misc(s,002701,XSU);lmi_xsu(s);next(s);
 lmi_newops(s,046135);sload(s,000050);right(s);
 if(bhiz(s,022225))return LIV_INITVEL6;
 next(s);
 /* VLOAD VSU RTARG1 RATT1; VAD R2VEC; STODL R2VEC COZY4; GOTO INITVEL2. */
 lmi_newops(s,052375);vload(s,003472);right(s);opnd(s,000017,VSU);lmi_vsu(s,0);next(s);
 lmi_newops(s,077655);opnd(s,002663,VAD);lmi_vad(s,0);next(s);
 stodl(s,016663,003667);
 lmi_newops(s,077650);go(s,022062);
 return LIV_INITVEL2;
}
uint8_t liv_initvel6(apollo_overlay_state *s,p66_bias_io *io){
 /* INITVEL6 VLOAD R2VEC; STORE RTARG1; then INITVEL7. */
 lmi_newops(s,077775);vload(s,002663);next(s);
 store(s,003472);
 return liv_initvel7(s,io);
}
uint8_t liv_initvel7(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[2];
 (void)io;
 /* INITVEL7 VLOAD VSU VIPRIME VINIT; STOVL DELVEET3 VTARGET; STORE
  * VTPRIME; SLOAD BHIZ RTX2 INITVELX. */
 lmi_newops(s,052375);vload(s,002337);right(s);opnd(s,002331,VSU);lmi_vsu(s,0);next(s);
 stovl(s,026366,002703);
 store(s,003566);
 lmi_newops(s,046135);sload(s,003000);right(s);
 if(!bhiz(s,022255)){
  next(s);
  /* VLOAD VSR2 VTPRIME; STOVL VTPRIME VIPRIME; VSR2; STOVL VIPRIME
   * RTARG1; VSR2; STOVL RTARG1 DELVEET3; VSR2; STORE DELVEET3. */
  lmi_newops(s,070575);vload(s,003566);right(s);shorts(s);next(s);
  stovl(s,027566,002337);
  lmi_newops(s,077742);shorts(s);next(s);
  stovl(s,026337,003472);
  lmi_newops(s,077742);shorts(s);next(s);
  stovl(s,027472,002366);
  lmi_newops(s,077742);shorts(s);next(s);
  store(s,002366);
 }
 /* INITVELX LXA,1 DLOAD* RTX1 MUTABLE -2,1; PUSH DMP R1A; SR1 DDV R1;
  * STODL MU/A (pushes up); SR 6; STORE MUASTEER. */
 lmi_newops(s,070750);lxa(s,002776);right(s);lmi_indexed(s,010003,DLOAD);if(!fixed_operand(s,k,2))return LTT_FAILED;lmi_dload_op(s,k);next(s);
 lmi_newops(s,041206);unary(s,PUSH,lmi_push);right(s);opnd(s,002742,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,056342);shorts(s);right(s);opnd(s,000041,DDV);lmi_ddv(s,0);next(s);
 stodl(s,017721,077661);
 lmi_newops(s,077661);lmi_direct(s,020607,GSHIFT);lmi_gshift(s);next(s);
 store(s,003717);
 /* SETPD VLOAD 0D RTARG1; STCALL RTARG NORMEX (to C(NORMEX)). */
 lmi_newops(s,077201);setpd0(s);vload(s,003472);next(s);
 lmi_store_code(s,037444);lmi_15bitadr(s,003463,CALLITA);lmi_call(s);lmi_goto_any(s);TRACE(s);
 return LTT_RETURNED;
}

uint8_t liv_remap(const apollo_overlay_state *s,uint8_t next){
 if(next==LTT_RETURNED&&W[4]==022000u&&W[0164]==02145u)return LIV_AFTERLAMBERT;
 return next;
}
uint8_t lm_initvel_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LIV_INITVEL:return liv_initvel(s,io);
 case LIV_HAVEGUES:return liv_havegues(s,io);
 case LIV_INITVEL1:return liv_initvel1(s,io);
 case LIV_INITVEL2:return liv_initvel2(s,io);
 case LIV_INITVEL3:return liv_initvel3(s,io);
 case LIV_AFTERLAMBERT:return liv_afterlambert(s,io);
 case LIV_AFTERINTSTALL:return liv_afterintstall(s,io);
 case LIV_AFTERINTEGRVS:return liv_afterintegrvs(s,io);
 case LIV_INITVEL6:return liv_initvel6(s,io);
 case LIV_INITVEL7:return liv_initvel7(s,io);
 default:
  if(LM_INITVEL_DONE(phase)||phase>LLA_MODPSDEL)return LTT_FAILED;
  return liv_remap(s,lm_conic_lambert_step(s,io,phase));
 }
}

/* The Luminary callers' returns: CALL INITVEL in P34's TPI (35,2502 and
 * 35,3366) and P35's MIDCOURSE (27,2372), CALL HAVEGUES in P35 (27,2724). */
static const agc_word q_initvel[3]={072503,073367,056373},q_havegues[1]={056725};
static uint8_t member(agc_word q,const agc_word *list,unsigned n){unsigned i;for(i=0;i<n;++i)if(list[i]==q)return 1;return 0;}
/* Canonical image with coherent banks (FBANK 11, BIT15 clear), VAC,
 * RTX1/RTX2 the Earth or Moon pair. At INITVEL and HAVEGUES QPRET is a
 * caller's return and PUSHLOC 4D; where INTSTALL and INTEGRVS return,
 * NORMEX holds a caller's return and PUSHLOC is within the VAC. */
uint8_t lm_initvel_entry(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120],pd=W[0166];agc_word q=0,rtx1=W[02776],rtx2=W[02777];
 if(W[5]!=06070u||W[4]!=022000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(022000u|(W[3]>>8))||W[0165]!=W[6]||W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 if(!((rtx1==077775u&&rtx2==0u)||(rtx1==077765u&&rtx2==2u)))return 0;
 q=W[vac+052u];
 switch(W[0164]){
 case 02000u:return member(q,q_initvel,3)&&pd==vac+4u?LIV_INITVEL:0u;
 case 02002u:return member(q,q_havegues,1)&&pd==vac+4u?LIV_HAVEGUES:0u;
 case 02155u:case 02200u:
  q=W[03463];
  if(!member(q,q_initvel,3)&&!member(q,q_havegues,1))return 0;
  return W[0164]==02155u?LIV_AFTERINTSTALL:LIV_AFTERINTEGRVS;
 default:return 0;
 }
}
uint8_t lm_initvel(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;unsigned guard;
 if(!s||!io||!(phase=lm_initvel_entry(s,io)))return 0;
 /* INITVEL runs a few steps either side of one LAMBERT (bounded as in
  * lm_conic_lambert); the guard only stops a runaway. */
 for(guard=0;guard<30000u&&!LM_INITVEL_DONE(phase);++guard)phase=lm_initvel_step(s,io,phase);
 return phase==LTT_RETURNED?1u:phase==LTT_ABORTED?2u:phase==LIV_EXTERNAL?3u:0u;
}
