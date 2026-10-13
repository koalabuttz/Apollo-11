/* SPDX-License-Identifier: MIT */
/* Native Luminary099 time-theta subroutine (Luminary099/CONIC_SUBROUTINES.agc):
 * TIMETHET/COMMNOUT/COGAOVFL (bank 12 02732-02766), PARAM and GEOM (bank 4
 * 03064-03164), GETX (12,2767-3211) and NEWSTATE/LAMENTER (12,2653-2731).
 * DELTIME is the Kepler milestone's (src/lm_conic_kepler.c). Every
 * interpretive pair is compiled: its original order and operand words appear
 * below only as the literal bookkeeping arguments of the lm_interp_steps and
 * lm_interp_orders paths, and each runtime branch (BPL/BMN, BOV, BON, TIX)
 * follows the outcome those paths return. GETX's POLY table is assembled in
 * this file; MUTABLE (bank 4) is read from the certified fixed-data table and
 * the CONICS1 constants appear as literal words. */
#include "lm_conic_timethet.h"
#include "lm_conic_kepler.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#ifdef LM_CONIC_TIMETHET_TRACE
void lm_conic_timethet_trace(const apollo_overlay_state *);
#define TRACE(s) lm_conic_timethet_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,SIGN=07655,VXSC=07401,DLOAD=06052,PDDL=06523,TLOAD=06470,TSLC=07616,
 DDV=07577,BDDV=07603,GSHIFT=07621,DOT=07331,VXV=07460,DSU=07005,BDSU=07062,DAD=06775,DMP=07572,
 DMPR=07574,TAD=07071,VAD=06751,SETPD=07613,SSP=06620,SQRT=03207,DSQ=03174,UNIT=03023,ABVAL=03176,
 RVQ=03274,PUSH=03247,ROUND=02116,AXT=02344,TIX=02415,CALLITA=02507,RTBBHIZ=02447,BZEGOTO=02467,
 BPLBMN=02474,SWITCH=02516,BOVB=02457};

/* CONICS1 constants (bank 4, 03030-03063). */
static const agc_word D1_4[2]={010000,0},D1_16[2]={02000,0},D1_32[2]={01000,0},D1_64[2]={0400,0},
 D1_128[2]={0200,0},D1_256[2]={0100,0},PISC2[2]={03110,017665};
/* GETX's POLY table (12,3042): N-1, then a0..a6. */
static const agc_word WPOLY[15]={05,020000,0,072525,052471,03146,015003,075556,045210,01615,013553,
 076371,063777,01232,027367};

/* Certified fixed words at bank*1024+sub (sub 0-01777). */
static uint8_t fixed_words(uint16_t at,agc_word *k,unsigned n){
 unsigned i;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(LM,(uint16_t)(at+i),&k[i]))return 0;
 return 1;
}

static void next(apollo_overlay_state *s){lmi_danzig_next(s);TRACE(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* SETPD as the last order of a pair: NOIBNKSW CCS EDOP (+0); CCS NEWJOB
 * (+0); INCR LOC; NEWOPS. */
static void setpd_next(apollo_overlay_state *s){
 (void)ai_ccs(s,023);(void)ai_ccs(s,067);ai_incr(s,0164);W[5]=06070;TRACE(s);
}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
/* STODL/STOVL: the store, then the load of the given word (or PUSHUP). */
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word,const agc_word *k){
 lmi_store_code(s,code);lmi_dodload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,DLOAD);else lmi_diradres(s,word,DLOAD);
 lmi_dload_op(s,k);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,VLOAD);else lmi_diradres(s,word,VLOAD);
 lmi_vload(s,0);next(s);
}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
static void rvq(apollo_overlay_state *s){lmi_newops(s,077616);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);}
/* RVQ as the right order of a pair. */
static void right_rvq(apollo_overlay_state *s){right(s);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);}
static void branch_taken(apollo_overlay_state *s){lmi_goto(s);TRACE(s);}
static void go(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);lmi_goto_order(s);TRACE(s);}
/* GOTO through an erasable CADR (GOTOERS) to its NEWOPS. */
static void go_erasable(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);TRACE(s);
}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);branch_taken(s);}
static uint8_t bplbmn(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,BPLBMN);
 if(!lmi_bplbmn(s))return 0;
 branch_taken(s);return 1;
}
/* BOV label: 0 at DANZIG, 1 taken (target NEWOPS), 2 out of scope. */
static uint8_t bov(apollo_overlay_state *s,agc_word word){
 uint8_t r;
 lmi_misc(s,word,BOVB);r=lmi_bov(s);
 if(r==LMI_BOV_NONE)return 0;
 if(r!=LMI_BOV_GOTO)return 2;
 branch_taken(s);return 1;
}
/* A switch order (word = its switch/operation word): 1 when it branches. */
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
/* SWBRANCH to an erasable branch word: CA BANKSET; TS FBANK; INDEX LOC,
 * CA 1; TS POLISH; TCF GOTO +1 (MASK HIGH4; BZF GOTOERS). */
static void swbranch_erasable(apollo_overlay_state *s,agc_word word){
 ai_ca(s,0165);(void)ai_ts(s,4);ai_caf(s,word);(void)ai_ts(s,0117);lmi_goto_any(s);TRACE(s);
}
/* RTB TPMODE (bank 10, 03633), as the left order of RTB PDDL. */
static void rtb_tpmode(apollo_overlay_state *s){
 lmi_misc(s,021633,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);lmi_tpmode(s);
}

uint8_t ltt_entry(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* TIMETHET STQ SETPD RTNTT 0; BOV +1. */
 lmi_newops(s,040220);lmi_misc(s,002710,CALLITA);lmi_ita(s);right(s);lmi_direct(s,000001,SETPD);lmi_setpd(s);setpd_next(s);
 lmi_newops(s,077600);
 switch(bov(s,024737)){case 0:next(s);break;case 1:break;default:return LTT_FAILED;}
 /* VLOAD PDVL RVEC VVEC; CALL PARAM. */
 lmi_newops(s,063375);opnd(s,002655,VLOAD);lmi_vload(s,0);right(s);opnd(s,002744,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,077624);call(s,011064);
 return LTT_PARAM;
}
uint8_t ltt_param(apollo_overlay_state *s,p66_bias_io *io){
 /* PARAM STQ CLEAR RTNPRM NORMSW; CLEAR COGAFLAG; SSP CALL GEOMSGN 37777 GEOM. */
 lmi_newops(s,043020);lmi_misc(s,002753,CALLITA);lmi_ita(s);right(s);if(sw(s,io,003665))return LTT_FAILED;next(s);
 lmi_newops(s,077614);if(sw(s,io,004273))return LTT_FAILED;next(s);
 lmi_newops(s,045131);lmi_direct(s,002673,SSP);lmi_ssp(s,027777);right(s);call(s,011130);
 return LTT_GEOM;
}
uint8_t ltt_geom(apollo_overlay_state *s,p66_bias_io *io){
 /* GEOM UNIT; STODL U2 36D; STOVL MAGVEC2 (pushes up); UNIT; STORE UR1. */
 lmi_newops(s,077656);lmi_unary(s,UNIT);if(lmi_unit(s))return LTT_ABORTED;next(s);
 stodl(s,016712,000045,0);
 stovl(s,026720,077656);
 lmi_newops(s,077656);lmi_unary(s,UNIT);if(lmi_unit(s))return LTT_ABORTED;next(s);
 store(s,002722);
 /* DOT SL1 U2; PDDL 36D; STOVL R1 UR1; VXV VSL1 U2. */
 lmi_newops(s,072441);opnd(s,002712,DOT);lmi_dot(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,077725);opnd(s,000045,PDDL);lmi_pddl_op(s,0);next(s);
 stovl(s,024041,002722);
 lmi_newops(s,076435);opnd(s,002712,VXV);lmi_vxv(s,0);right(s);shorts(s);next(s);
 /* BON SIGN NORMSW HAVENORM GEOMSGN. */
 lmi_newops(s,075214);
 if(sw(s,io,003705)){lmi_swbranch(s,011162);TRACE(s);return LTT_HAVENORM;}
 right(s);opnd(s,002673,SIGN);lmi_sign(s,0);next(s);
 /* UNIT BOV COLINEAR. */
 lmi_newops(s,040056);lmi_unary(s,UNIT);if(lmi_unit(s))return LTT_ABORTED;right(s);
 switch(bov(s,011160)){case 0:next(s);break;case 1:return LTT_COLINEAR;default:return LTT_FAILED;}
 return LTT_UNITNORM;
}
uint8_t ltt_unitnorm(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* UNITNORM STODL UN 36D; SIGN RVQ GEOMSGN. */
 stodl(s,016674,000045,0);
 lmi_newops(s,043565);opnd(s,002673,SIGN);lmi_sign(s,0);right_rvq(s);
 return LTT_PARAM2;
}
uint8_t ltt_colinear(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* COLINEAR VSR1 GOTO UNITNORM. */
 lmi_newops(s,052162);shorts(s);right(s);go(s,011154);
 return LTT_UNITNORM;
}
uint8_t ltt_havenorm(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* HAVENORM ABVAL SIGN GEOMSGN; RVQ. */
 lmi_newops(s,075246);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;right(s);opnd(s,002673,SIGN);lmi_sign(s,0);next(s);
 rvq(s);
 return LTT_PARAM2;
}
uint8_t ltt_param2(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[6];
 (void)io;
 /* STODL 36D (pushes up); SR DDV 5 36D. */
 stodl(s,014045,056261,0);
 lmi_newops(s,056261);opnd(s,020606,GSHIFT);lmi_gshift(s);right(s);opnd(s,000045,DDV);lmi_ddv(s,0);next(s);
 /* STOVL* COGA MUTABLE,1: the store, then DOVLOAD* CAF VLOAD* (fixed-fixed
  * 06107, the interpreter's VLOAD* code 040001); TCF DODLOAD* +1 (TS CYR)
  * into INDEX. */
 lmi_store_code(s,032766);
 if(W[5]!=06466u)return LTT_FAILED;
 ai_caf(s,040001);(void)ai_ts(s,020);lmi_indexed(s,010005,VLOAD);
 if(W[4]!=010000u||W[0116]<02006u||W[0116]>02016u||!fixed_words((uint16_t)(4u*1024u+W[0116]-02000u),k,6))return LTT_FAILED;
 lmi_vload(s,k);next(s);
 /* STODL 1/MU MAGVEC2; DSQ NORM X1; DMPR DMP 1/MU R1; SRR* 0 -3,1. */
 stodl(s,014017,002720,0);
 lmi_newops(s,060316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,041275);opnd(s,000017,DMPR);lmi_dmpr(s,0);right(s);opnd(s,000041,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,077657);gshift(s,021576);next(s);
 /* PUSH BDSU D1/32; STODL R1A (pushes up); DMP NORM 36D X1. */
 lmi_newops(s,044206);unary(s,PUSH,lmi_push);right(s);opnd(s,011043,BDSU);lmi_bdsu(s,D1_32);next(s);
 stodl(s,016742,060205,0);
 lmi_newops(s,060205);opnd(s,000045,DMP);lmi_dmp(s,0);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 /* DMP SR* 36D 0 -4,1; STORE P; GOTO RTNPRM. */
 lmi_newops(s,053605);opnd(s,000045,DMP);lmi_dmp(s,0);right(s);gshift(s,020575);next(s);
 store(s,002740);
 lmi_newops(s,077650);go_erasable(s,002753);
 return LTT_AFTERPARAM;
}
uint8_t ltt_afterparam(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* BOV CALL COGAOVFL GETX. */
 lmi_newops(s,045000);
 switch(bov(s,024764)){case 0:break;case 1:return LTT_COGAOVFL;default:return LTT_FAILED;}
 right(s);call(s,024767);
 return LTT_GETX;
}
uint8_t ltt_cogaovfl(apollo_overlay_state *s,p66_bias_io *io){
 /* COGAOVFL SETGO COGAFLAG ABTCONIC. */
 lmi_newops(s,077614);if(!sw(s,io,004033))return LTT_FAILED;lmi_swbranch(s,025742);TRACE(s);
 return LTT_ABTCONIC;
}
uint8_t ltt_abtconic(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* ABTCONIC EXIT; TC POODOO (Q at OCT 00607). */
 lmi_newops(s,077776);lmi_exit(s);
 if(W[5]!=03743u)return LTT_FAILED;
 ai_tc(s,03744);W[5]=05652;
 return LTT_CONICABORT;
}
uint8_t ltt_getx(apollo_overlay_state *s,p66_bias_io *io){
 /* GETX AXT,2 SSP 3 S2 1; CLEAR 360SW. */
 lmi_newops(s,066374);lmi_misc(s,000003,AXT);lmi_axt(s);right(s);lmi_direct(s,000052,SSP);lmi_ssp(s,000001);next(s);
 lmi_newops(s,077614);if(sw(s,io,004276))return LTT_FAILED;next(s);
 /* SQRT PDDL CSTH; SR1 BDSU D1/4; PDDL SRR SNTH 6; DDV (pushes up). */
 lmi_newops(s,065366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;right(s);opnd(s,002732,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,044342);shorts(s);right(s);opnd(s,011037,BDSU);lmi_bdsu(s,D1_4);next(s);
 lmi_newops(s,054325);opnd(s,002730,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,021607,GSHIFT);lmi_gshift(s);next(s);
 lmi_newops(s,077671);opnd(s,077600,DDV);lmi_ddv(s,0);next(s);
 /* BOV 360CHECK. */
 lmi_newops(s,077600);
 switch(bov(s,025122)){case 0:next(s);break;case 1:return LTT_360CHECK;default:return LTT_FAILED;}
 /* DSU DMP COGA (DMP pushes up); SL2R BOV 360CHECK. */
 lmi_newops(s,041225);opnd(s,002766,DSU);lmi_dsu(s,0);right(s);opnd(s,040132,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,040132);shorts(s);right(s);
 switch(bov(s,025122)){case 0:next(s);break;case 1:return LTT_360CHECK;default:return LTT_FAILED;}
 return LTT_WLOOP;
}
uint8_t ltt_wloop(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* WLOOP PUSH DSQ; TLOAD PDDL MPAC R1A; SR4 TAD (pushes up). */
 lmi_newops(s,063406);unary(s,PUSH,lmi_push);right(s);unary(s,DSQ,lmi_dsq);next(s);
 lmi_newops(s,065351);opnd(s,000155,TLOAD);lmi_tload(s);right(s);opnd(s,002742,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,076202);shorts(s);right(s);opnd(s,075440,TAD);lmi_tad(s,0);next(s);
 /* BMN SQRT INFINITY; ROUND DAD (pushes up). */
 lmi_newops(s,075440);if(bplbmn(s,025205))return LTT_INFINITY;
 right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 lmi_newops(s,043306);unary(s,ROUND,lmi_round);right(s);opnd(s,061000,DAD);lmi_dad(s,0);next(s);
 /* BOV TIX,2 RESETX2 WLOOP. */
 lmi_newops(s,061000);
 switch(bov(s,025120)){case 0:break;case 1:return LTT_RESETX2;default:return LTT_FAILED;}
 right(s);lmi_misc(s,025013,TIX);
 if(lmi_tix(s)){branch_taken(s);return LTT_WLOOP;}
 next(s);
 /* BDDV BOV D1/128 INFINITY. */
 lmi_newops(s,040065);opnd(s,011033,BDDV);lmi_bddv(s,D1_128);right(s);
 switch(bov(s,025205)){case 0:next(s);break;case 1:return LTT_INFINITY;default:return LTT_FAILED;}
 return LTT_POLYCOEF;
}
uint8_t ltt_polycoef(apollo_overlay_state *s,p66_bias_io *io){
 /* POLYCOEF BMN PUSH INFINITY; DSQ; NORM DMP X1 R1A. */
 lmi_newops(s,041440);if(bplbmn(s,025205))return LTT_INFINITY;
 right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,077716);unary(s,DSQ,lmi_dsq);next(s);
 lmi_newops(s,041301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,002742,DMP);lmi_dmp(s,0);next(s);
 /* SRR* EXIT 0 -10D,1; TC POLY (the table at 12,3042); TC INTPRET. */
 lmi_newops(s,077457);gshift(s,021567);right(s);lmi_exit(s);
 if(W[5]!=03041u)return LTT_FAILED;
 ai_tc(s,03042);W[5]=07222;lmi_poly(s,WPOLY);if(W[5]!=03061u)return LTT_FAILED;
 ai_tc(s,03062);W[5]=06037;lmi_intpret(s,io);TRACE(s);
 /* DMP SL1R (DMP pushes up); PUSH BON 360SW TRUE360X. */
 lmi_newops(s,076405);opnd(s,043006,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,043006);unary(s,PUSH,lmi_push);right(s);
 if(sw(s,io,004316)){lmi_swbranch(s,025172);TRACE(s);return LTT_TRUE360X;}
 next(s);
 return LTT_XCOMMON;
}
uint8_t ltt_xcommon(apollo_overlay_state *s,p66_bias_io *io){
 /* XCOMMON DSQ NORM X1; DMP SRR* R1A 0 -12D,1; STODL XI R1; SR1 SQRT. */
 lmi_newops(s,060316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,053605);opnd(s,002742,DMP);lmi_dmp(s,0);right(s);gshift(s,021565);next(s);
 stodl(s,014031,000041,0);
 lmi_newops(s,075542);shorts(s);right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 /* ROUND DMP (pushes up); SL4R; STORE X. */
 lmi_newops(s,041306);unary(s,ROUND,lmi_round);right(s);opnd(s,077632,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,077632);shorts(s);next(s);
 store(s,000025);
 /* DSQ NORM X1; PDDL DMP P R1; SL3 SQRT; DMP SL3R COGA; STODL KEPC1 R1A. */
 lmi_newops(s,060316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,041325);opnd(s,002740,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,000041,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,075452);shorts(s);right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 lmi_newops(s,056405);opnd(s,002766,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 stodl(s,014043,002742,0);
 /* BDSU CLEAR D1/64 INFINFLG; STORE KEPC2; RVQ. */
 lmi_newops(s,043021);opnd(s,011035,BDSU);lmi_bdsu(s,D1_64);right(s);if(sw(s,io,004270))return LTT_FAILED;next(s);
 store(s,000045);
 rvq(s);
 return LTT_COMMNOUT;
}
uint8_t ltt_resetx2(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* RESETX2 AXT,2 3. */
 lmi_newops(s,077774);lmi_misc(s,000003,AXT);lmi_axt(s);next(s);
 return LTT_360CHECK;
}
uint8_t ltt_360check(apollo_overlay_state *s,p66_bias_io *io){
 /* 360CHECK SETPD BPL 0D INVRSEQN; SET 360SW. */
 lmi_newops(s,051001);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);
 if(bplbmn(s,025127))return LTT_INVRSEQN;
 next(s);
 lmi_newops(s,077614);if(sw(s,io,004076))return LTT_FAILED;next(s);
 return LTT_INVRSEQN;
}
uint8_t ltt_invrseqn(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* INVRSEQN DLOAD SQRT P; PDDL DMP SNTH COGA; SL1 PDDL CSTH; SR4 DAD D1/32. */
 lmi_newops(s,075545);opnd(s,002740,DLOAD);lmi_dload_op(s,0);right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 lmi_newops(s,041325);opnd(s,002730,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,002766,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,065352);shorts(s);right(s);opnd(s,002732,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,043202);shorts(s);right(s);opnd(s,011043,DAD);lmi_dad(s,D1_32);next(s);
 /* DSU DMP (both push up); NORM BDDV X1 SNTH; SLR* ABS 0 -5,1; PUSH DSQ. */
 lmi_newops(s,041225);opnd(s,055301,DSU);lmi_dsu(s,0);right(s);opnd(s,055301,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,055301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,002730,BDDV);lmi_bddv(s,0);next(s);
 lmi_newops(s,051457);gshift(s,021174);right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;next(s);
 lmi_newops(s,063406);unary(s,PUSH,lmi_push);right(s);unary(s,DSQ,lmi_dsq);next(s);
 /* STODL 34D D1/16. */
 stodl(s,014043,011041,D1_16);
 return LTT_1WLOOP;
}
uint8_t ltt_1wloop(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* 1/WLOOP PUSH DSQ; RTB PDDL TPMODE R1A; DMP SR4 34D; TAD (pushes up). */
 lmi_newops(s,063406);unary(s,PUSH,lmi_push);right(s);unary(s,DSQ,lmi_dsq);next(s);
 lmi_newops(s,065234);rtb_tpmode(s);right(s);opnd(s,002742,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,040405);opnd(s,000043,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,077771);opnd(s,075440,TAD);lmi_tad(s,0);next(s);
 /* BMN SQRT INFINITY; DAD (pushes up). */
 lmi_newops(s,075440);if(bplbmn(s,025205))return LTT_INFINITY;
 right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 lmi_newops(s,077615);opnd(s,060304,DAD);lmi_dad(s,0);next(s);
 /* TIX,2 NORM 1/WLOOP X1. */
 lmi_newops(s,060304);lmi_misc(s,025151,TIX);
 if(lmi_tix(s)){branch_taken(s);return LTT_1WLOOP;}
 right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 /* BDDV (pushes up); SLR* GOTO 0 -7,1 POLYCOEF. */
 lmi_newops(s,077665);opnd(s,052057,BDDV);lmi_bddv(s,0);next(s);
 lmi_newops(s,052057);gshift(s,021172);right(s);go(s,025031);
 return LTT_POLYCOEF;
}
uint8_t ltt_true360x(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* TRUE360X DLOAD BMN R1A INFINITY; SQRT NORM X1; BDDV SL* 2PISC 0 -3,1. */
 lmi_newops(s,050145);opnd(s,002742,DLOAD);lmi_dload_op(s,0);right(s);
 if(bplbmn(s,025205))return LTT_INFINITY;
 next(s);
 lmi_newops(s,060366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,053665);opnd(s,011055,BDDV);lmi_bddv(s,PISC2);right(s);gshift(s,020176);next(s);
 /* DSU PUSH (DSU pushes up); GOTO XCOMMON. */
 lmi_newops(s,041425);opnd(s,077650,DSU);lmi_dsu(s,0);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,077650);go(s,025066);
 return LTT_XCOMMON;
}
uint8_t ltt_infinity(apollo_overlay_state *s,p66_bias_io *io){
 /* INFINITY SETPD BOV 0 OVFLCLR; OVFLCLR SET RVQ INFINFLG. */
 lmi_newops(s,040001);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);
 switch(bov(s,025210)){case 0:next(s);break;case 1:break;default:return LTT_FAILED;}
 lmi_newops(s,043414);if(sw(s,io,004070))return LTT_FAILED;right_rvq(s);
 return LTT_COMMNOUT;
}
uint8_t ltt_commnout(apollo_overlay_state *s,p66_bias_io *io){
 /* COMMNOUT DLOAD BON XI INFINFLG ABTCONIC; CLEAR CALL COGAFLAG DELTIME. */
 lmi_newops(s,043145);opnd(s,000031,DLOAD);lmi_dload_op(s,0);right(s);
 if(sw(s,io,004310)){lmi_swbranch(s,025742);TRACE(s);return LTT_ABTCONIC;}
 next(s);
 lmi_newops(s,045014);if(sw(s,io,004273))return LTT_FAILED;right(s);call(s,024421);
 return LTT_DELTIME;
}
uint8_t ltt_deltime(apollo_overlay_state *s,p66_bias_io *io){
 return lck_deltime(s,io)==LCK_DELTIME2?LTT_DELTIME2:LTT_FAILED;
}
uint8_t ltt_deltime2(apollo_overlay_state *s,p66_bias_io *io){
 return lck_deltime2(s,io)==LCK_DELTAIL?LTT_DELTAIL:LTT_FAILED;
}
uint8_t ltt_deltail(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 return lck_deltail(s)==LCK_KEPRET?LTT_AFTERDT:LTT_FAILED;
}
uint8_t ltt_afterdt(apollo_overlay_state *s,p66_bias_io *io){
 /* BON CALL RVSW RTNTT NEWSTATE. */
 lmi_newops(s,045014);
 if(sw(s,io,003706)){swbranch_erasable(s,002710);return LTT_RETURNED;}
 right(s);call(s,024653);
 return LTT_NEWSTATE;
}
uint8_t ltt_newstate(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* NEWSTATE DLOAD SR4R R1; DSU VXSC XSQC(XI) UR1; VSL1 PDDL X; DSQ NORM X1. */
 lmi_newops(s,044545);opnd(s,000041,DLOAD);lmi_dload_op(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,074225);opnd(s,000035,DSU);lmi_dsu(s,0);right(s);opnd(s,002722,VXSC);lmi_vxsc(s,0);next(s);
 lmi_newops(s,065372);shorts(s);right(s);opnd(s,000025,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,060316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 /* DMPR DMPR 1/ROOTMU X; DMP SRR* S(XI) 0 -7,1; BDSU T; SL1 VXSC VVEC. */
 lmi_newops(s,057275);opnd(s,000023,DMPR);lmi_dmpr(s,0);right(s);opnd(s,000025,DMPR);lmi_dmpr(s,0);next(s);
 lmi_newops(s,053605);opnd(s,000033,DMP);lmi_dmp(s,0);right(s);gshift(s,021572);next(s);
 lmi_newops(s,077621);opnd(s,000037,BDSU);lmi_bdsu(s,0);next(s);
 lmi_newops(s,074352);shorts(s);right(s);opnd(s,002744,VXSC);lmi_vxsc(s,0);next(s);
 /* VSL1 VAD (pushes up); VSL4 PUSH; ABVAL. */
 lmi_newops(s,053372);shorts(s);right(s);opnd(s,041512,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,041512);shorts(s);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,077646);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;next(s);
 /* LAMENTER NORM X1; STODL R2 XI; DMP DSU S(XI) D1/128; DMP SL1R ROOTMU. */
 lmi_newops(s,077701);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 stodl(s,016720,000031,0);
 lmi_newops(s,045205);opnd(s,000033,DMP);lmi_dmp(s,0);right(s);opnd(s,011033,DSU);lmi_dsu(s,D1_128);next(s);
 lmi_newops(s,076405);opnd(s,000021,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 /* DMP SLR* X 0 -3,1; DDV VXSC R2 UR1; VSL1 PDDL XSQC(XI); SLR* DDV 0 -4,1 R2. */
 lmi_newops(s,053605);opnd(s,000025,DMP);lmi_dmp(s,0);right(s);gshift(s,021176);next(s);
 lmi_newops(s,074271);opnd(s,002720,DDV);lmi_ddv(s,0);right(s);opnd(s,002722,VXSC);lmi_vxsc(s,0);next(s);
 lmi_newops(s,065372);shorts(s);right(s);opnd(s,000035,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,056257);gshift(s,021175);right(s);opnd(s,002720,DDV);lmi_ddv(s,0);next(s);
 /* BDSU D1/256; VXSC VAD VVEC (VAD pushes up); VSL8 RVQ. */
 lmi_newops(s,077621);opnd(s,011047,BDSU);lmi_bdsu(s,D1_256);next(s);
 lmi_newops(s,053361);opnd(s,002744,VXSC);lmi_vxsc(s,0);right(s);opnd(s,043412,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,043412);shorts(s);right_rvq(s);
 return LTT_FINAL;
}
uint8_t ltt_final(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* GOTO RTNTT (an erasable CADR). */
 lmi_newops(s,077650);go_erasable(s,002710);
 return LTT_RETURNED;
}

uint8_t lm_conic_timethet_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LTT_ENTRY:return ltt_entry(s,io);
 case LTT_PARAM:return ltt_param(s,io);
 case LTT_GEOM:return ltt_geom(s,io);
 case LTT_UNITNORM:return ltt_unitnorm(s,io);
 case LTT_COLINEAR:return ltt_colinear(s,io);
 case LTT_HAVENORM:return ltt_havenorm(s,io);
 case LTT_PARAM2:return ltt_param2(s,io);
 case LTT_AFTERPARAM:return ltt_afterparam(s,io);
 case LTT_COGAOVFL:return ltt_cogaovfl(s,io);
 case LTT_GETX:return ltt_getx(s,io);
 case LTT_WLOOP:return ltt_wloop(s,io);
 case LTT_POLYCOEF:return ltt_polycoef(s,io);
 case LTT_XCOMMON:return ltt_xcommon(s,io);
 case LTT_RESETX2:return ltt_resetx2(s,io);
 case LTT_360CHECK:return ltt_360check(s,io);
 case LTT_INVRSEQN:return ltt_invrseqn(s,io);
 case LTT_1WLOOP:return ltt_1wloop(s,io);
 case LTT_TRUE360X:return ltt_true360x(s,io);
 case LTT_INFINITY:return ltt_infinity(s,io);
 case LTT_COMMNOUT:return ltt_commnout(s,io);
 case LTT_DELTIME:return ltt_deltime(s,io);
 case LTT_DELTIME2:return ltt_deltime2(s,io);
 case LTT_DELTAIL:return ltt_deltail(s,io);
 case LTT_AFTERDT:return ltt_afterdt(s,io);
 case LTT_NEWSTATE:return ltt_newstate(s,io);
 case LTT_FINAL:return ltt_final(s,io);
 case LTT_ABTCONIC:return ltt_abtconic(s,io);
 default:return LTT_FAILED;
 }
}

/* The actual entry: a CALL TIMETHET (or STCALL) from one of six callers
 * leaves NEWOPS at 12,2732 with QPRET its return; X1 -2 or -10 (Earth or
 * Moon, from the callers' VECSHIFT). */
static const agc_word RETURNS[6]={064320,070250,070363,071171,072664,071600};
static uint8_t admitted(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120];agc_word x1,q;uint8_t ok=0;
 if(W[5]!=06070u||W[4]!=024000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(024000u|(W[3]>>8))||W[0165]!=W[6]||W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 x1=W[vac+046u];q=W[vac+052u];
 for(i=0;i<6u;++i)if(q==RETURNS[i])ok=1;
 return ok&&W[0164]==02732u&&(x1==077775u||x1==077765u);
}
uint8_t lm_conic_timethet(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase=LTT_ENTRY;unsigned guard;
 if(!s||!io||!admitted(s,io))return 0;
 /* GETX's loops run at most four passes each (X2 = 3, TIX); the guard only
  * stops a runaway. */
 for(guard=0;guard<200u&&phase<LTT_RETURNED;++guard)phase=lm_conic_timethet_step(s,io,phase);
 return phase==LTT_RETURNED?1u:phase==LTT_ABORTED?2u:phase==LTT_CONICABORT?3u:0u;
}
