/* SPDX-License-Identifier: MIT */
/* Native Luminary099 Lambert subroutine (Luminary099/CONIC_SUBROUTINES.agc):
 * LAMBERT through TARGETV (bank 12 03212-03546), ITERATOR (12,2536-2643)
 * and, for TARGETV, LAMENTER (12,2701-2731). GEOM, GETX and DELTIME are the
 * time-theta milestone's steps (src/lm_conic_timethet.c); CHECKCTR
 * (12,2644-2652) is compiled inline at its RTB as the Kepler milestone does.
 * Every interpretive pair is compiled: its original order and operand words
 * appear below only as the literal bookkeeping arguments of the
 * lm_interp_steps and lm_interp_orders paths, and each runtime branch
 * follows the outcome those paths return. MUTABLE (bank 4) and ONE (bank 2)
 * are read from the certified fixed-data table; the CONICS1 constants appear
 * as literal words. */
#include "lm_conic_lambert.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#ifdef LM_CONIC_LAMBERT_TRACE
void lm_conic_lambert_trace(const apollo_overlay_state *);
#define TRACE(s) lm_conic_lambert_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#include "agc_image_w.h"
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,SIGN=07655,VXSC=07401,DLOAD=06052,SLOAD=06501,PDDL=06523,TSLC=07616,
 DDV=07577,GSHIFT=07621,VXV=07460,DSU=07005,BDSU=07062,DAD=06775,DMP=07572,DMPR=07574,VAD=06751,
 SETPD=07613,SSP=06620,SQRT=03207,DSQ=03174,ABVAL=03176,STADR=06354,RVQ=03274,PUSH=03247,
 ROUND=02116,XSU=02411,CALLITA=02507,RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,SWITCH=02516,BOVB=02457};

/* CONICS1 constants (bank 4, 03027-03063); BEE17 and BEE19 are the DP words
 * at 4,3027 and 4,3041 (BEE17 DEC 0 with D1/8, D1/16's low word with D1/32). */
static const agc_word BEE17[2]={0,04000},BEE19[2]={0,01000},D1_4[2]={010000,0},D1_32[2]={01000,0},
 D1_128[2]={0200,0},D1_256[2]={0100,0},D1_1024[2]={020,0},DP9_10[2]={034631,023146},
 ONEBIT[2]={0,01},COGUPLIM[2]={037767,037737},COGLOLIM[2]={040010,040040};

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
/* SETPD 0 as the left order of a pair. */
static void setpd0(apollo_overlay_state *s){lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
/* STODL: the store, then the load of the given word. */
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);lmi_dload_op(s,0);next(s);
}
/* STADR with the complemented store word given (the STORE pushes up). */
static void stadr(apollo_overlay_state *s,agc_word word){lmi_newops(s,077626);lmi_unary(s,STADR);lmi_stadr(s,word);}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
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
static uint8_t bze(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,BZEGOTO);
 if(!lmi_bze(s))return 0;
 TRACE(s);return 1;
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
/* A switch branch to a fixed label. */
static void swgo(apollo_overlay_state *s,agc_word word){lmi_swbranch(s,word);TRACE(s);}
/* SWBRANCH to an erasable branch word: CA BANKSET; TS FBANK; INDEX LOC,
 * CA 1; TS POLISH; TCF GOTO +1 (MASK HIGH4; BZF GOTOERS). */
static void swbranch_erasable(apollo_overlay_state *s,agc_word word){
 ai_ca(s,0165);(void)ai_ts(s,4);ai_caf(s,word);(void)ai_ts(s,0117);lmi_goto_any(s);TRACE(s);
}
/* DLOAD then a DP9/10 multiply and GOTO NEWDEL (MODNGDEL, MODPSDEL):
 * DLOAD DSU word INDEP; DMP GOTO DP9/10 NEWDEL. */
static void dp910_newdel(apollo_overlay_state *s,agc_word word){
 lmi_newops(s,045345);opnd(s,word,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,002766,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,052005);opnd(s,011051,DMP);lmi_dmp(s,DP9_10);right(s);go(s,024634);
}
/* SETPD DLOAD 0 COGA; STORE word (HIENERGY's COGAMIN, LOENERGY's COGAMAX). */
static void bound(apollo_overlay_state *s,agc_word code){
 lmi_newops(s,071201);setpd0(s);opnd(s,002766,DLOAD);lmi_dload_op(s,0);next(s);
 store(s,code);
}

uint8_t lla_lambert(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[6];
 /* LAMBERT STQ SETPD RTNLAMB 0D; BOV +1. */
 lmi_newops(s,040220);lmi_misc(s,002710,CALLITA);lmi_ita(s);right(s);lmi_direct(s,000001,SETPD);lmi_setpd(s);setpd_next(s);
 lmi_newops(s,077600);
 switch(bov(s,025217)){case 0:next(s);break;case 1:break;default:return LTT_FAILED;}
 /* CLEAR VLOAD* SOLNSW MUTABLE,1 (1/MU, ROOTMU, 1/ROOTMU of the Earth or
  * Moon); STODL 1/MU TDESIRED; DMPR BEE19; STORE EPSILONL. */
 lmi_newops(s,076614);if(sw(s,io,002674))return LTT_FAILED;right(s);lmi_indexed(s,010005,VLOAD);
 if(W[4]!=010000u||W[0116]<02006u||W[0116]>02016u||!fixed_words((uint16_t)(4u*1024u+W[0116]-02000u),k,6))return LTT_FAILED;
 lmi_vload(s,k);next(s);
 stodl(s,014017,002671);
 lmi_newops(s,077675);opnd(s,011042,DMPR);lmi_dmpr(s,BEE19);next(s);
 store(s,002764);
 /* SET VLOAD SLOPESW R1VEC; PDVL CALL R2VEC GEOM. */
 lmi_newops(s,077214);if(sw(s,io,000474))return LTT_FAILED;right(s);opnd(s,002655,VLOAD);lmi_vload(s,0);next(s);
 lmi_newops(s,045115);opnd(s,002663,PDVL);lmi_pdvl(s,0);right(s);call(s,011130);
 return LTT_GEOM;
}
uint8_t lla_aftergeom(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* STODL SNTH MAGVEC2; NORM PDDL X1 R1; SR1 DDV (pushes up); SL* PDDL
  * 0 -6,1 (PDDL pushes up); STADR STORE CSTH. */
 stodl(s,016730,002720);
 lmi_newops(s,065301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,000041,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,056342);shorts(s);right(s);opnd(s,065257,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,065257);gshift(s,020173);right(s);opnd(s,077626,PDDL);lmi_pddl_op(s,0);next(s);
 stadr(s,075045);next(s);
 /* SR1 BDSU D1/4; STORE 1-CSTH; ROUND BZE 360LAMB. */
 lmi_newops(s,044342);shorts(s);right(s);opnd(s,011037,BDSU);lmi_bdsu(s,D1_4);next(s);
 store(s,002734);
 lmi_newops(s,053106);unary(s,ROUND,lmi_round);right(s);if(bze(s,025463))return LLA_360LAMB;
 next(s);
 /* NORM PDDL X1 0D; SR1 DDV (pushes up); SL* SQRT 0 -3,1; PDDL SR SNTH 6;
  * DDV DAD 1-CSTH (DAD pushes up); STADR STORE COGAMAX. */
 lmi_newops(s,065301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,000001,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,056342);shorts(s);right(s);opnd(s,075457,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,075457);gshift(s,020176);right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 lmi_newops(s,054325);opnd(s,002730,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,020607,GSHIFT);lmi_gshift(s);next(s);
 lmi_newops(s,043271);opnd(s,002734,DDV);lmi_ddv(s,0);right(s);opnd(s,077626,DAD);lmi_dad(s,0);next(s);
 stadr(s,077760);next(s);
 /* BOV BMN UPLIM MAXCOGA; DSU BMN COGUPLIM MAXCOGA. */
 lmi_newops(s,050000);
 switch(bov(s,025277)){case 0:break;case 1:return LLA_UPLIM;default:return LTT_FAILED;}
 right(s);if(bplbmn(s,025302))return LLA_MAXCOGA;
 next(s);
 lmi_newops(s,050025);opnd(s,011061,DSU);lmi_dsu(s,COGUPLIM);right(s);if(bplbmn(s,025302))return LLA_MAXCOGA;
 next(s);
 return LLA_UPLIM;
}
uint8_t lla_uplim(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* UPLIM DLOAD COGUPLIM; STORE COGAMAX. */
 lmi_newops(s,077745);opnd(s,011061,DLOAD);lmi_dload_op(s,COGUPLIM);next(s);
 store(s,000017);
 return LLA_MAXCOGA;
}
uint8_t lla_maxcoga(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* MAXCOGA DLOAD CSTH; SR DSU 6 (DSU pushes up); STADR STODL CSTH-RHO
  * GEOMSGN; BMN DLOAD LOLIM CSTH-RHO; SL1 DDV SNTH; BOV LOLIM. */
 lmi_newops(s,077745);opnd(s,002732,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,045261);opnd(s,020607,GSHIFT);lmi_gshift(s);right(s);opnd(s,077626,DSU);lmi_dsu(s,0);next(s);
 stadr(s,061041);lmi_dodload(s);lmi_diradres(s,002673,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,071240);if(bplbmn(s,025503))return LLA_LOLIM;
 right(s);opnd(s,002736,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,056352);shorts(s);right(s);opnd(s,002730,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,077600);
 switch(bov(s,025503)){case 0:next(s);break;case 1:return LLA_LOLIM;default:return LTT_FAILED;}
 return LLA_MINCOGA;
}
uint8_t lla_mincoga(apollo_overlay_state *s,p66_bias_io *io){
 /* MINCOGA STORE COGAMIN; BON SSP GUESSW NOGUESS TWEEKIT 00001; DLOAD COGA. */
 store(s,000011);
 lmi_newops(s,066214);if(sw(s,io,000715)){swgo(s,025467);return LLA_NOGUESS;}
 right(s);lmi_direct(s,000051,SSP);lmi_ssp(s,000001);next(s);
 lmi_newops(s,077745);opnd(s,002766,DLOAD);lmi_dload_op(s,0);next(s);
 return LLA_LAMBLOOP;
}
uint8_t lla_noguess(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* NOGUESS SSP DLOAD TWEEKIT 20000 COGAMIN; SR1 PDDL COGAMAX; SR1 DAD
  * (pushes up); STADR STORE COGA; STORE DCOGA; GOTO LAMBLOOP. */
 lmi_newops(s,071331);lmi_direct(s,000051,SSP);lmi_ssp(s,010000);right(s);opnd(s,000011,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,065342);shorts(s);right(s);opnd(s,000017,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,043342);shorts(s);right(s);opnd(s,077626,DAD);lmi_dad(s,0);next(s);
 stadr(s,075011);next(s);
 store(s,000015);
 lmi_newops(s,077650);go(s,025330);
 return LLA_LAMBLOOP;
}
uint8_t lla_lolim(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* LOLIM DLOAD GOTO COGLOLIM MINCOGA. */
 lmi_newops(s,052145);opnd(s,011063,DLOAD);lmi_dload_op(s,COGLOLIM);right(s);go(s,025320);
 return LLA_MINCOGA;
}
uint8_t lla_lambloop(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* LAMBLOOP DMP SNTH; SR1 DSU CSTH-RHO; NORM PDDL X1 1-CSTH; SL* DDV
  * 0 -9D,1 (DDV pushes up); BMN BZE NEGP NEGP. */
 lmi_newops(s,077605);opnd(s,002730,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,045342);shorts(s);right(s);opnd(s,002736,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,065301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,002734,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,056257);gshift(s,020170);right(s);opnd(s,053040,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,053040);if(bplbmn(s,025417))return LLA_NEGP;
 right(s);if(bze(s,025417))return LLA_NEGP;
 next(s);
 /* STODL P COGA; DSQ DAD D1/1024; NORM DMP X1 P; SR* BDSU 0 -8D,1 D1/32;
  * STODL R1A P; BOV CALL HIENERGY GETX. */
 stodl(s,016740,002766);
 lmi_newops(s,043316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,011045,DAD);lmi_dad(s,D1_1024);next(s);
 lmi_newops(s,041301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,002740,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,044257);gshift(s,020571);right(s);opnd(s,011043,BDSU);lmi_bdsu(s,D1_32);next(s);
 stodl(s,016742,002740);
 lmi_newops(s,045000);
 switch(bov(s,025422)){case 0:break;case 1:return LLA_HIENERGY;default:return LTT_FAILED;}
 right(s);call(s,024767);
 return LTT_GETX;
}
uint8_t lla_aftergetx(apollo_overlay_state *s,p66_bias_io *io){
 /* DLOAD T; STODL TPREV XI; BON CALL INFINFLG NEGP DELTIME. */
 lmi_newops(s,077745);opnd(s,000037,DLOAD);lmi_dload_op(s,0);next(s);
 stodl(s,016762,000031);
 lmi_newops(s,045014);if(sw(s,io,004310)){swgo(s,025417);return LLA_NEGP;}
 right(s);call(s,024421);
 return LTT_DELTIME;
}
uint8_t lla_afterdt(apollo_overlay_state *s,p66_bias_io *io){
 agc_word one;unsigned ctr;
 (void)io;
 /* BOV BDSU BIGTIME TDESIRED; STORE TERRLAMB; ABS BDSU EPSILONL. */
 lmi_newops(s,044200);
 switch(bov(s,025437)){case 0:break;case 1:return LLA_BIGTIME;default:return LTT_FAILED;}
 right(s);opnd(s,002671,BDSU);lmi_bdsu(s,0);next(s);
 store(s,002760);
 lmi_newops(s,044246);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;right(s);opnd(s,002764,BDSU);lmi_bdsu(s,0);next(s);
 /* BPL RTB INITV CHECKCTR: CHECKCTR CS ONE; INDEX FIXLOC, AD ITERCTR;
  * INDEX FIXLOC, TS ITERCTR; TS MPAC; TC DANZIG. The instruction words
  * (12,2644-2652) give the addresses: CS ONE reads the fixed-fixed
  * constant at 04753 (bank 2, 0753); INDEX FIXLOC adds C(FIXLOC). */
 lmi_newops(s,047044);if(bplbmn(s,025506))return LLA_INITV;
 right(s);lmi_misc(s,024644,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);
 if(W[5]!=02644u||!fixed_words((uint16_t)(2u*1024u+(044753&01777u)),&one,1))return LTT_FAILED;
 ctr=(unsigned)(W[050120&01777u]+(060026&01777u));
 ai_csf(s,one);ai_ad(s,ctr);ctr=(unsigned)(W[050120&01777u]+(054026&01777u));(void)ai_ts(s,ctr);(void)ai_ts(s,054154&01777u);
 ai_tc(s,02653);W[5]=06061;
 next(s);
 /* BHIZ CALL SUFFCHEK ITERATOR. */
 lmi_newops(s,045030);lmi_misc(s,025450,RTBBHIZ);(void)ai_ccs(s,020);
 if(lmi_bhiz(s)){branch_taken(s);return LLA_SUFFCHEK;}
 right(s);call(s,024536);
 return LLA_ITERATOR;
}
uint8_t lla_afteriter(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* DLOAD BZE MPAC SUFFCHEK; DAD COGA; STORE COGA; GOTO LAMBLOOP. */
 lmi_newops(s,053145);opnd(s,000155,DLOAD);lmi_dload_op(s,0);right(s);if(bze(s,025450))return LLA_SUFFCHEK;
 next(s);
 lmi_newops(s,077615);opnd(s,002766,DAD);lmi_dad(s,0);next(s);
 store(s,002766);
 lmi_newops(s,077650);go(s,025330);
 return LLA_LAMBLOOP;
}
uint8_t lla_negp(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* NEGP DLOAD BPL DCOGA LOENERGY. */
 lmi_newops(s,051145);opnd(s,000015,DLOAD);lmi_dload_op(s,0);right(s);if(bplbmn(s,025442))return LLA_LOENERGY;
 next(s);
 return LLA_HIENERGY;
}
uint8_t lla_hienergy(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* HIENERGY SETPD DLOAD 0 COGA; STORE COGAMIN. */
 bound(s,000011);
 return LLA_COMMONLM;
}
uint8_t lla_commonlm(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* COMMONLM DLOAD SR1 DCOGA; STORE DCOGA; BZE BDSU SUFFCHEK COGA; STORE
  * COGA; GOTO LAMBLOOP. */
 lmi_newops(s,070545);opnd(s,000015,DLOAD);lmi_dload_op(s,0);right(s);shorts(s);next(s);
 store(s,000015);
 lmi_newops(s,044254);if(bze(s,025450))return LLA_SUFFCHEK;
 right(s);opnd(s,002766,BDSU);lmi_bdsu(s,0);next(s);
 store(s,002766);
 lmi_newops(s,077650);go(s,025330);
 return LLA_LAMBLOOP;
}
uint8_t lla_bigtime(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* BIGTIME DLOAD TPREV; STORE T. */
 lmi_newops(s,077745);opnd(s,002762,DLOAD);lmi_dload_op(s,0);next(s);
 store(s,000037);
 return LLA_LOENERGY;
}
uint8_t lla_loenergy(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* LOENERGY SETPD DLOAD 0 COGA; STORE COGAMAX; GOTO COMMONLM. */
 bound(s,000017);
 lmi_newops(s,077650);go(s,025426);
 return LLA_COMMONLM;
}
uint8_t lla_suffchek(apollo_overlay_state *s,p66_bias_io *io){
 /* SUFFCHEK DLOAD ABS TERRLAMB; PDDL DMP TDESIRED BEE17; DAD DSU ONEBIT
  * (DSU pushes up); BPL SETGO INITV SOLNSW INITV. */
 lmi_newops(s,051545);opnd(s,002760,DLOAD);lmi_dload_op(s,0);right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;next(s);
 lmi_newops(s,041325);opnd(s,002671,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,011030,DMP);lmi_dmp(s,BEE17);next(s);
 lmi_newops(s,045215);opnd(s,011057,DAD);lmi_dad(s,ONEBIT);right(s);opnd(s,043044,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,043044);if(bplbmn(s,025506))return LLA_INITV;
 right(s);if(!sw(s,io,002434))return LTT_FAILED;
 swgo(s,025506);
 return LLA_INITV;
}
uint8_t lla_360lamb(apollo_overlay_state *s,p66_bias_io *io){
 /* 360LAMB SETPD SETGO 0 SOLNSW RTNLAMB. */
 lmi_newops(s,043001);setpd0(s);if(!sw(s,io,002434))return LTT_FAILED;
 swbranch_erasable(s,002710);
 return LTT_RETURNED;
}
uint8_t lla_initv(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* INITV DLOAD NORM R1 X1; PDDL SR1 P; DDV (pushes up); SL* SQRT 0 -4,1;
  * DMP SL1 ROOTMU; PUSH DMP COGA; SL VXSC 5 UR1. */
 lmi_newops(s,060345);opnd(s,000041,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,070525);opnd(s,002740,PDDL);lmi_pddl_op(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,077671);opnd(s,075457,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,075457);gshift(s,020175);right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 lmi_newops(s,072405);opnd(s,000021,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,041206);unary(s,PUSH,lmi_push);right(s);opnd(s,002766,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,074261);opnd(s,020206,GSHIFT);lmi_gshift(s);right(s);opnd(s,002722,VXSC);lmi_vxsc(s,0);next(s);
 /* PDDL (pushes up); VXSC VSL1 UN; VXV VAD UR1 (VAD pushes up); VSL1;
  * STORE VVEC; SLOAD BZE VTARGTAG TARGETV; GOTO RTNLAMB. */
 lmi_newops(s,077725);opnd(s,076561,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,076561);opnd(s,002674,VXSC);lmi_vxsc(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,053235);opnd(s,002722,VXV);lmi_vxv(s,0);right(s);opnd(s,077772,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,077772);shorts(s);next(s);
 store(s,002744);
 lmi_newops(s,053135);lmi_direct(s,002702,SLOAD);lmi_sload(s);right(s);if(bze(s,025541))return LLA_TARGETV;
 next(s);
 lmi_newops(s,077650);go_erasable(s,002710);
 return LTT_RETURNED;
}
/* LAMENTER (12,2701-2731) through its RVQ: NORM X1; STODL R2 XI; DMP DSU
 * S(XI) D1/128; DMP SL1R ROOTMU; DMP SLR* X 0 -3,1; DDV VXSC R2 UR1; VSL1
 * PDDL XSQC(XI); SLR* DDV 0 -4,1 R2; BDSU D1/256; VXSC VAD VVEC (VAD
 * pushes up); VSL8 RVQ. The same words as the time-theta milestone's
 * NEWSTATE tail (ltt_newstate). */
static void lamenter(apollo_overlay_state *s){
 lmi_newops(s,077701);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 stodl(s,016720,000031);
 lmi_newops(s,045205);opnd(s,000033,DMP);lmi_dmp(s,0);right(s);opnd(s,011033,DSU);lmi_dsu(s,D1_128);next(s);
 lmi_newops(s,076405);opnd(s,000021,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,053605);opnd(s,000025,DMP);lmi_dmp(s,0);right(s);gshift(s,021176);next(s);
 lmi_newops(s,074271);opnd(s,002720,DDV);lmi_ddv(s,0);right(s);opnd(s,002722,VXSC);lmi_vxsc(s,0);next(s);
 lmi_newops(s,065372);shorts(s);right(s);opnd(s,000035,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,056257);gshift(s,021175);right(s);opnd(s,002720,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,077621);opnd(s,011047,BDSU);lmi_bdsu(s,D1_256);next(s);
 lmi_newops(s,053361);opnd(s,002744,VXSC);lmi_vxsc(s,0);right(s);opnd(s,043412,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,043412);shorts(s);right_rvq(s);
}
uint8_t lla_targetv(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* TARGETV DLOAD CALL MAGVEC2 LAMENTER; (LAMENTER); STORE VTARGET; GOTO
  * RTNLAMB. */
 lmi_newops(s,045145);opnd(s,002720,DLOAD);lmi_dload_op(s,0);right(s);call(s,024701);
 lamenter(s);
 if(W[0164]!=03544u)return LTT_FAILED;
 store(s,002703);
 lmi_newops(s,077650);go_erasable(s,002710);
 return LTT_RETURNED;
}

uint8_t lla_iterator(apollo_overlay_state *s,p66_bias_io *io){
 /* ITERATOR BONCLR DLOAD SLOPESW FIRSTIME DEP; DSU NORM DEPREV X1; PDDL
  * NORM DELINDEP X2; XSU,1 DMP X2 DELDEP; SLR* DDV 1,1 (DDV pushes up). */
 lmi_newops(s,071214);if(sw(s,io,000614)){swgo(s,024606);return LLA_FIRSTIME;}
 right(s);opnd(s,000037,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,060225);opnd(s,002762,DSU);lmi_dsu(s,0);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,060325);opnd(s,000015,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,000050,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,041260);lmi_misc(s,000047,XSU);lmi_xsu(s);right(s);opnd(s,002760,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,056257);gshift(s,021202);right(s);opnd(s,043142,DDV);lmi_ddv(s,0);next(s);
 /* SR1 BOFF ORDERSW SGNCHECK; ABS SIGN DELDEP. */
 lmi_newops(s,043142);shorts(s);right(s);if(sw(s,io,004351)){swgo(s,024562);return LLA_SGNCHECK;}
 next(s);
 lmi_newops(s,075246);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;right(s);opnd(s,002760,SIGN);lmi_sign(s,0);next(s);
 return LLA_SGNCHECK;
}
uint8_t lla_sgncheck(apollo_overlay_state *s,p66_bias_io *io){
 /* SGNCHECK PUSH BPL POSDEL; DLOAD BON INDEP ORDERSW MINCHECK; STORE MAX. */
 lmi_newops(s,051006);unary(s,PUSH,lmi_push);right(s);if(bplbmn(s,024620))return LLA_POSDEL;
 next(s);
 lmi_newops(s,043145);opnd(s,002766,DLOAD);lmi_dload_op(s,0);right(s);if(sw(s,io,004311)){swgo(s,024571);return LLA_MINCHECK;}
 next(s);
 store(s,000017);
 return LLA_MINCHECK;
}
uint8_t lla_mincheck(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* MINCHECK BDSU DSU MIN (DSU pushes up); BOV BPL MODNGDEL MODNGDEL;
  * GOTO DELOK. */
 lmi_newops(s,045221);opnd(s,000011,BDSU);lmi_bdsu(s,0);right(s);opnd(s,051000,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,051000);
 switch(bov(s,024600)){case 0:break;case 1:return LLA_MODNGDEL;default:return LTT_FAILED;}
 right(s);if(bplbmn(s,024600))return LLA_MODNGDEL;
 next(s);
 lmi_newops(s,077650);go(s,024632);
 return LLA_DELOK;
}
uint8_t lla_modngdel(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* MODNGDEL DLOAD DSU MIN INDEP; DMP GOTO DP9/10 NEWDEL. */
 dp910_newdel(s,000011);
 return LLA_NEWDEL;
}
uint8_t lla_firstime(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* FIRSTIME DLOAD DMP MIN TWEEKIT; PDDL DMP MAX TWEEKIT; DSU (pushes up);
  * SIGN GOTO DELDEP SGNCHECK. */
 lmi_newops(s,041345);opnd(s,000011,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000051,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,041325);opnd(s,000017,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,000051,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,077625);opnd(s,052165,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,052165);opnd(s,002760,SIGN);lmi_sign(s,0);right(s);go(s,024562);
 return LLA_SGNCHECK;
}
uint8_t lla_posdel(apollo_overlay_state *s,p66_bias_io *io){
 /* POSDEL DLOAD BON INDEP ORDERSW MAXCHECK; STORE MIN. */
 lmi_newops(s,043145);opnd(s,002766,DLOAD);lmi_dload_op(s,0);right(s);if(sw(s,io,004311)){swgo(s,024625);return LLA_MAXCHECK;}
 next(s);
 store(s,000011);
 return LLA_MAXCHECK;
}
uint8_t lla_maxcheck(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* MAXCHECK BDSU DSU MAX (DSU pushes up); BOV BMN MODPSDEL MODPSDEL. */
 lmi_newops(s,045221);opnd(s,000017,BDSU);lmi_bdsu(s,0);right(s);opnd(s,050000,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,050000);
 switch(bov(s,024636)){case 0:break;case 1:return LLA_MODPSDEL;default:return LTT_FAILED;}
 right(s);if(bplbmn(s,024636))return LLA_MODPSDEL;
 next(s);
 return LLA_DELOK;
}
uint8_t lla_delok(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* DELOK DLOAD 0D. */
 lmi_newops(s,077745);opnd(s,000001,DLOAD);lmi_dload_op(s,0);next(s);
 return LLA_NEWDEL;
}
uint8_t lla_newdel(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* NEWDEL STORE DELINDEP; RVQ (to LAMBERT, 12,3407). */
 store(s,000015);
 lmi_newops(s,077616);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);
 return W[0164]==03407u?LLA_AFTERITER:LTT_FAILED;
}
uint8_t lla_modpsdel(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* MODPSDEL DLOAD DSU MAX INDEP; DMP GOTO DP9/10 NEWDEL. */
 dp910_newdel(s,000017);
 return LLA_NEWDEL;
}

uint8_t lla_remap(const apollo_overlay_state *s,uint8_t next){
 switch(next){
 case LTT_PARAM2:return W[0164]==03235u?LLA_AFTERGEOM:LTT_FAILED;
 case LTT_COMMNOUT:return W[0164]==03363u?LLA_AFTERGETX:LTT_FAILED;
 case LTT_AFTERDT:return W[0164]==03373u?LLA_AFTERDT:LTT_FAILED;
 case LTT_ENTRY:case LTT_PARAM:case LTT_AFTERPARAM:case LTT_COGAOVFL:case LTT_NEWSTATE:case LTT_FINAL:
 case LTT_CONICABORT:return LTT_FAILED;
 default:return next;
 }
}
uint8_t lm_conic_lambert_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LLA_LAMBERT:return lla_lambert(s,io);
 case LLA_AFTERGEOM:return lla_aftergeom(s,io);
 case LLA_UPLIM:return lla_uplim(s,io);
 case LLA_MAXCOGA:return lla_maxcoga(s,io);
 case LLA_MINCOGA:return lla_mincoga(s,io);
 case LLA_NOGUESS:return lla_noguess(s,io);
 case LLA_LOLIM:return lla_lolim(s,io);
 case LLA_LAMBLOOP:return lla_lambloop(s,io);
 case LLA_AFTERGETX:return lla_aftergetx(s,io);
 case LLA_AFTERDT:return lla_afterdt(s,io);
 case LLA_AFTERITER:return lla_afteriter(s,io);
 case LLA_NEGP:return lla_negp(s,io);
 case LLA_HIENERGY:return lla_hienergy(s,io);
 case LLA_COMMONLM:return lla_commonlm(s,io);
 case LLA_BIGTIME:return lla_bigtime(s,io);
 case LLA_LOENERGY:return lla_loenergy(s,io);
 case LLA_SUFFCHEK:return lla_suffchek(s,io);
 case LLA_360LAMB:return lla_360lamb(s,io);
 case LLA_INITV:return lla_initv(s,io);
 case LLA_TARGETV:return lla_targetv(s,io);
 case LLA_ITERATOR:return lla_iterator(s,io);
 case LLA_SGNCHECK:return lla_sgncheck(s,io);
 case LLA_MINCHECK:return lla_mincheck(s,io);
 case LLA_MODNGDEL:return lla_modngdel(s,io);
 case LLA_FIRSTIME:return lla_firstime(s,io);
 case LLA_POSDEL:return lla_posdel(s,io);
 case LLA_MAXCHECK:return lla_maxcheck(s,io);
 case LLA_DELOK:return lla_delok(s,io);
 case LLA_NEWDEL:return lla_newdel(s,io);
 case LLA_MODPSDEL:return lla_modpsdel(s,io);
 case LTT_ENTRY:case LTT_PARAM:case LTT_AFTERPARAM:case LTT_COGAOVFL:case LTT_NEWSTATE:case LTT_FINAL:return LTT_FAILED;
 default:
  if(phase>=LTT_RETURNED)return LTT_FAILED;
  return lla_remap(s,lm_conic_timethet_step(s,io,phase));
 }
}

/* The actual entry: INITVEL's CALL SETITCTR (return 11,2145) sets ITERCTR
 * to 20 or 5 and branches to LAMBERT: NEWOPS at 12,3212 with QPRET the
 * return, X1 -2 or -10 (LXA,1 RTX1). */
static uint8_t admitted(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120];agc_word x1,ctr;
 if(W[5]!=06070u||W[4]!=024000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(024000u|(W[3]>>8))||W[0165]!=W[6]||W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 x1=W[vac+046u];ctr=W[vac+026u];
 return W[0164]==03212u&&W[vac+052u]==022145u&&(x1==077775u||x1==077765u)&&(ctr==024u||ctr==5u);
}
uint8_t lm_conic_lambert(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase=LLA_LAMBERT;unsigned guard;
 if(!s||!io||!admitted(s,io))return 0;
 /* At most 20 ITERATOR passes (ITERCTR, CHECKCTR); between them the
  * HIENERGY/LOENERGY passes halve DCOGA until it is zero (28 bits), each
  * pass a bounded GETX and DELTIME. The guard only stops a runaway. */
 for(guard=0;guard<30000u&&!LM_CONIC_LAMBERT_DONE(phase);++guard)phase=lm_conic_lambert_step(s,io,phase);
 return phase==LTT_RETURNED?1u:phase==LTT_ABORTED?2u:0u;
}
