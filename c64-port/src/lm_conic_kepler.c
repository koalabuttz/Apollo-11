/* SPDX-License-Identifier: MIT */
/* Native Luminary099 Kepler subroutine (Luminary099/CONIC_SUBROUTINES.agc,
 * bank 12 words 02023-02652): KEPLERN, DELTIME and CHECKCTR. Every
 * interpretive pair is compiled: its original order and operand words
 * appear below only as the literal bookkeeping arguments of the
 * lm_interp_steps and lm_interp_orders paths, and each runtime branch (BZE,
 * BPL/BMN, BOV, BHIZ) follows the outcome those paths return. DELTIME's
 * POLY tables and the CONICS1 constants (bank 4, 03030-03063) are assembled
 * in this file and appear as literal words; MUTABLE (bank 4) and KEPZERO
 * (LO6ZEROS, bank 12) are read from the certified fixed-data table. */
#include "lm_conic_kepler.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#ifdef LM_CONIC_KEPLER_TRACE
void lm_conic_kepler_trace(const apollo_overlay_state *);
#define TRACE(s) lm_conic_kepler_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,SIGN=07655,VXSC=07401,DLOAD=06052,PDDL=06523,TSLC=07616,DDV=07577,BDDV=07603,
 GSHIFT=07621,DOT=07331,DSU=07005,BDSU=07062,DAD=06775,DMP=07572,DMPR=07574,TAD=07071,VAD=06751,
 SETPD=07613,SSP=06620,SQRT=03207,DSQ=03174,COMP=07670,UNIT=03023,ABVAL=03176,VSQ=03245,STADR=06354,
 RVQ=03274,PUSH=03247,ROUND=02116,XSU=02411,CALLITA=02507,
 RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,BOVB=02457};

/* CONICS1 constants (bank 4, 03030-03063). */
static const agc_word D1_64[2]={0400,0},D1_128[2]={0200,0},D1_256[2]={0100,0},DP9_10[2]={034631,023146},
 M50SC[2]={077467,077777},PISC2[2]={03110,017665},BEE22[2]={0,0100};
/* DELTIME's POLY tables (12,2423 and 12,2455): N-1, then a0..a9. */
static const agc_word SXI[21]={010,02525,012526,067356,075666,015001,023771,064342,043674,06563,04645,
 075173,052672,0656,014331,077633,040512,023,011210,077774,067506};
static const agc_word CXI[21]={010,01000,0,072525,052506,013301,015337,062776,054733,011176,013267,
 073410,051674,01446,033641,077451,065233,055,037266,077767,052336};

/* Certified fixed words at bank*1024+sub (sub 0-01777). */
static uint8_t fixed_pair(uint16_t at,agc_word *k,unsigned n){
 unsigned i;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(LM,(uint16_t)(at+i),&k[i]))return 0;
 return 1;
}
/* KEPZERO (LO6ZEROS, 12,2006). */
static uint8_t kepzero(agc_word k[2]){return fixed_pair((uint16_t)(012u*1024u+06u),k,2);}

static void next(apollo_overlay_state *s){lmi_danzig_next(s);TRACE(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
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
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
static void rvq(apollo_overlay_state *s){lmi_newops(s,077616);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);}
/* GOTO (or a taken branch) to its target's NEWOPS. */
static void branch_taken(apollo_overlay_state *s){lmi_goto(s);TRACE(s);}
/* GOTO label (a single GOTO order, or the right order of a pair). */
static void go(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);lmi_goto_order(s);TRACE(s);}
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
/* DMPR GOTO DP9/10 NEWDELX (NDXCHNGE and PDXCHNGE). */
static void dp910_newdelx(apollo_overlay_state *s){
 lmi_newops(s,052075);opnd(s,011051,DMPR);lmi_dmpr(s,DP9_10);right(s);go(s,024260);
}
/* DSU VXSC XSQC(XI) URRECT; VSL1 PDDL X; DSQ NORM X1; DMPR DMPR 1/ROOTMU X;
 * DMP SRR* S(XI) 0 -7,1; BDSU T; SL1 VXSC VRECT; VSL1 VAD (pushes up). */

uint8_t lck_entry(apollo_overlay_state *s){
 agc_word k[6];
 /* KEPLERN SETPD BOV 0 +1. */
 lmi_newops(s,040001);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);
 switch(bov(s,024026)){case 0:next(s);break;case 1:break;default:return LCK_FAILED;}
 /* VLOAD* MUTABLE,1 (1/MU, ROOTMU, 1/ROOTMU of the Earth or Moon). */
 lmi_newops(s,077773);lmi_indexed(s,010005,VLOAD);
 if(W[4]!=010000u||W[0116]<02006u||W[0116]>02016u||!fixed_pair((uint16_t)(4u*1024u+W[0116]-02000u),k,6))return LCK_FAILED;
 lmi_vload(s,k);next(s);
 /* STOVL 14D RRECT; UNIT SSP ITERCTR 20D; STODL URRECT 36D; STOVL R1 RRECT. */
 stovl(s,024017,001503);
 lmi_newops(s,066256);lmi_unary(s,UNIT);if(lmi_unit(s))return LCK_ABORTED;right(s);lmi_direct(s,000027,SSP);lmi_ssp(s,000024);next(s);
 stodl(s,016647,000045,0);
 stovl(s,024041,001503);
 /* DOT SL1R VRECT; DMP SL1R 1/ROOTMU; STOVL KEPC1 VRECT. */
 lmi_newops(s,076441);opnd(s,001511,DOT);lmi_dot(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,076405);opnd(s,000023,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 stovl(s,024043,001511);
 /* VSQ DMPR 1/MU; DMP SL3 R1; DSU ROUND D1/64; STORE KEPC2. */
 lmi_newops(s,057236);unary(s,VSQ,lmi_vsq);right(s);opnd(s,000017,DMPR);lmi_dmpr(s,0);next(s);
 lmi_newops(s,052405);opnd(s,000041,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,061425);opnd(s,011035,DSU);lmi_dsu(s,D1_64);right(s);unary(s,ROUND,lmi_round);next(s);
 store(s,000045);
 /* BDSU SR1R D1/64; DDV R1; STORE ALPHA. */
 lmi_newops(s,074421);opnd(s,011035,BDSU);lmi_bdsu(s,D1_64);right(s);shorts(s);next(s);
 lmi_newops(s,077671);opnd(s,000041,DDV);lmi_ddv(s,0);next(s);
 store(s,000011);
 /* BPL DLOAD 1REV -50SC. */
 lmi_newops(s,071244);if(bplbmn(s,024073))return LCK_1REV;
 right(s);opnd(s,011053,DLOAD);lmi_dload_op(s,M50SC);next(s);
 /* DDV BOV ALPHA STOREMAX; SQRT GOTO STOREMAX. */
 lmi_newops(s,040071);opnd(s,000011,DDV);lmi_ddv(s,0);right(s);
 switch(bov(s,024077)){case 0:next(s);break;case 1:return LCK_STOREMAX;default:return LCK_FAILED;}
 lmi_newops(s,052166);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LCK_ABORTED;right(s);go(s,024077);
 return LCK_STOREMAX;
}
uint8_t lck_1rev(apollo_overlay_state *s){
 /* 1REV SQRT BDDV 2PISC; BOV STOREMAX (both ways to STOREMAX). */
 lmi_newops(s,055366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LCK_ABORTED;right(s);opnd(s,011055,BDDV);lmi_bddv(s,PISC2);next(s);
 lmi_newops(s,077600);
 switch(bov(s,024077)){case 0:next(s);break;case 1:break;default:return LCK_FAILED;}
 return LCK_STOREMAX;
}
uint8_t lck_storemax(apollo_overlay_state *s){
 /* STOREMAX STORE XMAX; DMP PDDL 1/ROOTMU ALPHA; NORM PDDL X1 (PDDL pushes up). */
 store(s,000013);
 lmi_newops(s,065205);opnd(s,000023,DMP);lmi_dmp(s,0);right(s);opnd(s,000011,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,065301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,056257,PDDL);lmi_pddl_op(s,0);next(s);
 /* SL* DDV 0 -6,1 (DDV pushes up); BOV BMN MODDONE MODDONE. */
 lmi_newops(s,056257);gshift(s,020173);right(s);opnd(s,050000,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,050000);
 switch(bov(s,024124)){case 0:right(s);break;case 1:return LCK_MODDONE;default:return LCK_FAILED;}
 if(bplbmn(s,024124))return LCK_MODDONE;
 next(s);return LCK_PERIODCH;
}
uint8_t lck_periodch(apollo_overlay_state *s){
 /* PERIODCH PDDL ABS TAU.; DSU BMN 0D MODDONE. */
 lmi_newops(s,051525);opnd(s,002074,PDDL);lmi_pddl_op(s,0);right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LCK_ABORTED;next(s);
 lmi_newops(s,050025);opnd(s,000001,DSU);lmi_dsu(s,0);right(s);if(bplbmn(s,024124))return LCK_MODDONE;next(s);
 /* SIGN TAU.; STODL TAU. (pushes up); GOTO PERIODCH. */
 lmi_newops(s,077765);opnd(s,002074,SIGN);lmi_sign(s,0);next(s);
 stodl(s,016074,077650,0);
 lmi_newops(s,077650);go(s,024112);
 return LCK_PERIODCH;
}
uint8_t lck_moddone(apollo_overlay_state *s){
 /* MODDONE SETPD DLOAD 0 XKEPNEW; STORE X. */
 lmi_newops(s,071201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);opnd(s,002130,DLOAD);lmi_dload_op(s,0);next(s);
 store(s,000025);
 /* SIGN BZE TAU. BADX; BMN ABS BADX; DSU BPL XMAX BADX. */
 lmi_newops(s,053165);opnd(s,002074,SIGN);lmi_sign(s,0);right(s);if(bze(s,024302))return LCK_BADX;next(s);
 lmi_newops(s,051440);if(bplbmn(s,024302))return LCK_BADX;right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LCK_ABORTED;next(s);
 lmi_newops(s,051025);opnd(s,000013,DSU);lmi_dsu(s,0);right(s);if(bplbmn(s,024302))return LCK_BADX;next(s);
 return LCK_STORBNDS;
}
uint8_t lck_storbnds(apollo_overlay_state *s){
 agc_word k[2];
 if(!kepzero(k))return LCK_FAILED;
 /* STORBNDS DLOAD BPL TAU. STOREMIN; DLOAD DCOMP XMAX; STODL XMIN KEPZERO;
  * STORE XMAX; GOTO DXCOMP. */
 lmi_newops(s,051145);opnd(s,002074,DLOAD);lmi_dload_op(s,0);right(s);if(bplbmn(s,024152))return LCK_STOREMIN;next(s);
 lmi_newops(s,057545);opnd(s,000013,DLOAD);lmi_dload_op(s,0);right(s);unary(s,COMP,lmi_comp);next(s);
 stodl(s,014015,024007,k);
 store(s,000013);
 lmi_newops(s,077650);go(s,024155);
 return LCK_DXCOMP;
}
uint8_t lck_storemin(apollo_overlay_state *s){
 agc_word k[2];
 if(!kepzero(k))return LCK_FAILED;
 /* STOREMIN DLOAD KEPZERO; STORE XMIN. */
 lmi_newops(s,077745);opnd(s,024007,DLOAD);lmi_dload_op(s,k);next(s);
 store(s,000015);
 return LCK_DXCOMP;
}
uint8_t lck_dxcomp(apollo_overlay_state *s){
 /* DXCOMP DLOAD DMPR TAU. BEE22; ABS; STODL EPSILONT XPREV. */
 lmi_newops(s,057345);opnd(s,002074,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,011046,DMPR);lmi_dmpr(s,BEE22);next(s);
 lmi_newops(s,077646);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LCK_ABORTED;next(s);
 stodl(s,016177,001553,0);
 /* XDIFF BDSU X; STORE DELX. */
 lmi_newops(s,077621);opnd(s,000025,BDSU);lmi_bdsu(s,0);next(s);
 store(s,002643);
 return LCK_KEPLOOP;
}
uint8_t lck_keploop(apollo_overlay_state *s){
 /* KEPLOOP DLOAD DSQ X; NORM PUSH X1; DMP SRR* ALPHA 0 -6,1. */
 lmi_newops(s,063545);opnd(s,000025,DLOAD);lmi_dload_op(s,0);right(s);unary(s,DSQ,lmi_dsq);next(s);
 lmi_newops(s,041501);opnd(s,000047,TSLC);lmi_tslc(s);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,053605);opnd(s,000011,DMP);lmi_dmp(s,0);right(s);gshift(s,021573);next(s);
 /* STCALL XI DELTIME. */
 lmi_store_code(s,034031);lmi_15bitadr(s,024421,CALLITA);lmi_call(s);branch_taken(s);
 return LCK_DELTIME;
}
uint8_t lck_deltime(apollo_overlay_state *s,p66_bias_io *io){
 /* DELTIME EXIT; TC POLY (S(XI) series); TC INTPRET; STODL S(XI) XI. */
 lmi_newops(s,077776);lmi_exit(s);
 ai_tc(s,02423);W[5]=07222;lmi_poly(s,SXI);if(W[5]!=02450u)return LCK_FAILED;
 ai_tc(s,02451);W[5]=06037;lmi_intpret(s,io);TRACE(s);
 stodl(s,014033,000031,0);
 return LCK_DELTIME2;
}
uint8_t lck_deltime2(apollo_overlay_state *s,p66_bias_io *io){
 /* EXIT; TC POLY (C(XI) series); TC INTPRET. */
 lmi_newops(s,077776);lmi_exit(s);
 ai_tc(s,02455);W[5]=07222;lmi_poly(s,CXI);if(W[5]!=02502u)return LCK_FAILED;
 ai_tc(s,02503);W[5]=06037;lmi_intpret(s,io);TRACE(s);
 return LCK_DELTAIL;
}
/* RTB TPMODE (bank 10, 03633), as the left order of RTB PDDL. */
static void rtb_tpmode(apollo_overlay_state *s){
 lmi_misc(s,021633,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);lmi_tpmode(s);
}
uint8_t lck_deltail(apollo_overlay_state *s){
 /* DMP SRR* 0D 0 -5,1; STORE XSQC(XI); DMP SL1 KEPC1; RTB PDDL TPMODE
  * (PDDL pushes up). */
 lmi_newops(s,053605);opnd(s,000001,DMP);lmi_dmp(s,0);right(s);gshift(s,021574);next(s);
 store(s,000035);
 lmi_newops(s,072405);opnd(s,000043,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,065234);rtb_tpmode(s);right(s);opnd(s,053605,PDDL);lmi_pddl_op(s,0);next(s);
 /* DMP SRR* S(XI) 0 -5,1; DMP SL1 KEPC2; RTB PDDL TPMODE R1. */
 lmi_newops(s,053605);opnd(s,000033,DMP);lmi_dmp(s,0);right(s);gshift(s,021574);next(s);
 lmi_newops(s,072405);opnd(s,000045,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,065234);rtb_tpmode(s);right(s);opnd(s,000041,PDDL);lmi_pddl_op(s,0);next(s);
 /* SR TAD 6 (TAD pushes up); NORM DMP X1 X; SR* TAD 0 -3,1 (pushes up). */
 lmi_newops(s,076261);lmi_direct(s,020607,GSHIFT);lmi_gshift(s);right(s);opnd(s,041301,TAD);lmi_tad(s,0);next(s);
 lmi_newops(s,041301);opnd(s,000047,TSLC);lmi_tslc(s);right(s);opnd(s,000025,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,076257);gshift(s,020576);right(s);opnd(s,057232,TAD);lmi_tad(s,0);next(s);
 /* SL4R DMPR 1/ROOTMU; STORE T; RVQ. */
 lmi_newops(s,057232);shorts(s);right(s);opnd(s,000023,DMPR);lmi_dmpr(s,0);next(s);
 store(s,000037);
 rvq(s);
 return LCK_KEPRET;
}
uint8_t lck_kepret(apollo_overlay_state *s){
 /* BOV BDSU TIMEOVFL TAU.; STORE DELT. */
 lmi_newops(s,044200);
 switch(bov(s,024311)){case 0:right(s);break;case 1:return LCK_TIMEOVFL;default:return LCK_FAILED;}
 opnd(s,002074,BDSU);lmi_bdsu(s,0);next(s);
 store(s,002645);
 /* ABS BDSU EPSILONT; BPL DLOAD KEPCONVG T. */
 lmi_newops(s,044246);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LCK_ABORTED;right(s);opnd(s,002177,BDSU);lmi_bdsu(s,0);next(s);
 lmi_newops(s,071244);if(bplbmn(s,024333))return LCK_KEPCONVG;
 right(s);opnd(s,000037,DLOAD);lmi_dload_op(s,0);next(s);
 /* DSU NORM TC X1; PDDL NORM DELX X2; XSU,1 DMP X2 DELT. */
 lmi_newops(s,060225);opnd(s,001551,DSU);lmi_dsu(s,0);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,060325);opnd(s,002643,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,000050,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,041260);lmi_misc(s,000047,XSU);lmi_xsu(s);right(s);opnd(s,002645,DMP);lmi_dmp(s,0);next(s);
 /* SLR* DDV 1,1 (DDV pushes up); SR1 PUSH. */
 lmi_newops(s,056257);gshift(s,021202);right(s);opnd(s,041542,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,041542);shorts(s);right(s);unary(s,PUSH,lmi_push);next(s);
 /* BPL DLOAD POSDELX X; STORE XMAX; BDSU DSU XMIN (DSU pushes up). */
 lmi_newops(s,071244);if(bplbmn(s,024246))return LCK_POSDELX;
 right(s);opnd(s,000025,DLOAD);lmi_dload_op(s,0);next(s);
 store(s,000013);
 lmi_newops(s,045221);opnd(s,000015,BDSU);lmi_bdsu(s,0);right(s);opnd(s,051000,DSU);lmi_dsu(s,0);next(s);
 /* BOV BPL NDXCHNGE NDXCHNGE; DLOAD GOTO 0D NEWDELX. */
 lmi_newops(s,051000);
 switch(bov(s,024240)){case 0:right(s);break;case 1:return LCK_NDXCHNGE;default:return LCK_FAILED;}
 if(bplbmn(s,024240))return LCK_NDXCHNGE;
 next(s);
 lmi_newops(s,052145);opnd(s,000001,DLOAD);lmi_dload_op(s,0);right(s);go(s,024260);
 return LCK_NEWDELX;
}
uint8_t lck_ndxchnge(apollo_overlay_state *s){
 /* NDXCHNGE DLOAD DSU XMIN X; DMPR GOTO DP9/10 NEWDELX. */
 lmi_newops(s,045345);opnd(s,000015,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000025,DSU);lmi_dsu(s,0);next(s);
 dp910_newdelx(s);return LCK_NEWDELX;
}
uint8_t lck_posdelx(apollo_overlay_state *s){
 /* POSDELX DLOAD X; STORE XMIN; BDSU DSU XMAX (DSU pushes up). */
 lmi_newops(s,077745);opnd(s,000025,DLOAD);lmi_dload_op(s,0);next(s);
 store(s,000015);
 lmi_newops(s,045221);opnd(s,000013,BDSU);lmi_bdsu(s,0);right(s);opnd(s,050000,DSU);lmi_dsu(s,0);next(s);
 /* BOV BMN PDXCHNGE PDXCHNGE; DLOAD 0D. */
 lmi_newops(s,050000);
 switch(bov(s,024274)){case 0:right(s);break;case 1:return LCK_PDXCHNGE;default:return LCK_FAILED;}
 if(bplbmn(s,024274))return LCK_PDXCHNGE;
 next(s);
 lmi_newops(s,077745);opnd(s,000001,DLOAD);lmi_dload_op(s,0);next(s);
 return LCK_NEWDELX;
}
uint8_t lck_pdxchnge(apollo_overlay_state *s){
 /* PDXCHNGE DLOAD DSU XMAX X; DMPR GOTO DP9/10 NEWDELX. */
 lmi_newops(s,045345);opnd(s,000013,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000025,DSU);lmi_dsu(s,0);next(s);
 dp910_newdelx(s);return LCK_NEWDELX;
}
uint8_t lck_newdelx(apollo_overlay_state *s){
 /* NEWDELX STORE DELX; BZE DAD KEPCONVG X; STODL X T; STORE TC. */
 store(s,002643);
 lmi_newops(s,043254);if(bze(s,024333))return LCK_KEPCONVG;
 right(s);opnd(s,000025,DAD);lmi_dad(s,0);next(s);
 stodl(s,014025,000037,0);
 store(s,001551);
 return LCK_BRNCHCTR;
}
uint8_t lck_brnchctr(apollo_overlay_state *s){
 /* BRNCHCTR RTB BHIZ CHECKCTR KEPCONVG: CHECKCTR CS ONE; INDEX FIXLOC,
  * AD ITERCTR; INDEX FIXLOC, TS ITERCTR; TS MPAC; TC DANZIG. */
 lmi_newops(s,046034);lmi_misc(s,024644,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);
 if(W[5]!=02644u)return LCK_FAILED;
 {/* The instruction words (12,2644-2652) give the addresses: CS ONE reads
   * the fixed-fixed constant at 04753 (bank 2, 0753); INDEX FIXLOC adds
   * C(FIXLOC) to AD/TS ITERCTR. */
  agc_word one;unsigned ctr;
  if(!fixed_pair((uint16_t)(2u*1024u+(044753&01777u)),&one,1))return LCK_FAILED;
  ctr=(unsigned)(W[050120&01777u]+(060026&01777u));
  ai_csf(s,one);ai_ad(s,ctr);ctr=(unsigned)(W[050120&01777u]+(054026&01777u));(void)ai_ts(s,ctr);(void)ai_ts(s,054154&01777u);
  ai_tc(s,02653);W[5]=06061;
 }
 right(s);lmi_misc(s,024333,RTBBHIZ);(void)ai_ccs(s,020);
 if(lmi_bhiz(s)){branch_taken(s);return LCK_KEPCONVG;}
 next(s);
 /* GOTO KEPLOOP. */
 lmi_newops(s,077650);go(s,024166);
 return LCK_KEPLOOP;
}
uint8_t lck_badx(apollo_overlay_state *s){
 /* BADX DLOAD SR1 XMAX; SIGN TAU.; STORE X; GOTO STORBNDS. */
 lmi_newops(s,070545);opnd(s,000013,DLOAD);lmi_dload_op(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,077765);opnd(s,002074,SIGN);lmi_sign(s,0);next(s);
 store(s,000025);
 lmi_newops(s,077650);go(s,024140);
 return LCK_STORBNDS;
}
uint8_t lck_timeovfl(apollo_overlay_state *s){
 /* TIMEOVFL DLOAD BMN X NEGTOVFL; STORE XMAX. */
 lmi_newops(s,050145);opnd(s,000025,DLOAD);lmi_dload_op(s,0);right(s);if(bplbmn(s,024330))return LCK_NEGTOVFL;next(s);
 store(s,000013);
 return LCK_CMNTOVFL;
}
uint8_t lck_negtovfl(apollo_overlay_state *s){
 /* NEGTOVFL STORE XMIN; GOTO CMNTOVFL. */
 store(s,000015);
 lmi_newops(s,077650);go(s,024315);
 return LCK_CMNTOVFL;
}
uint8_t lck_cmntovfl(apollo_overlay_state *s){
 /* CMNTOVFL DLOAD SR1 DELX; STORE DELX; BZE BDSU KEPRTN X. */
 lmi_newops(s,070545);opnd(s,002643,DLOAD);lmi_dload_op(s,0);right(s);shorts(s);next(s);
 store(s,002643);
 lmi_newops(s,044254);lmi_misc(s,002112,BZEGOTO);
 if(lmi_bzegoto(s)){lmi_goto_any(s);TRACE(s);return LCK_RETURNED;}
 right(s);opnd(s,000025,BDSU);lmi_bdsu(s,0);next(s);
 /* STODL X TC; STORE T; GOTO BRNCHCTR. */
 stodl(s,014025,001551,0);
 store(s,000037);
 lmi_newops(s,077650);go(s,024267);
 return LCK_BRNCHCTR;
}
uint8_t lck_kepconvg(apollo_overlay_state *s){
 /* KEPCONVG DLOAD SR4R R1; DSU VXSC XSQC(XI) URRECT; VSL1 PDDL X. */
 lmi_newops(s,044545);opnd(s,000041,DLOAD);lmi_dload_op(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,074225);opnd(s,000035,DSU);lmi_dsu(s,0);right(s);opnd(s,002647,VXSC);lmi_vxsc(s,0);next(s);
 lmi_newops(s,065372);shorts(s);right(s);opnd(s,000025,PDDL);lmi_pddl_op(s,0);next(s);
 /* DSQ NORM X1; DMPR DMPR 1/ROOTMU X; DMP SRR* S(XI) 0 -7,1; BDSU T. */
 lmi_newops(s,060316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,057275);opnd(s,000023,DMPR);lmi_dmpr(s,0);right(s);opnd(s,000025,DMPR);lmi_dmpr(s,0);next(s);
 lmi_newops(s,053605);opnd(s,000033,DMP);lmi_dmp(s,0);right(s);gshift(s,021572);next(s);
 lmi_newops(s,077621);opnd(s,000037,BDSU);lmi_bdsu(s,0);next(s);
 /* SL1 VXSC VRECT; VSL1 VAD (pushes up); VSL4; STORE RCV. */
 lmi_newops(s,074352);shorts(s);right(s);opnd(s,001511,VXSC);lmi_vxsc(s,0);next(s);
 lmi_newops(s,053372);shorts(s);right(s);opnd(s,077712,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,077712);shorts(s);next(s);
 store(s,001535);
 /* ABVAL NORM X2; STODL RCNORM XI; DMPR DSU S(XI) D1/128. */
 lmi_newops(s,060246);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LCK_ABORTED;right(s);opnd(s,000050,TSLC);lmi_tslc(s);next(s);
 stodl(s,014043,000031,0);
 lmi_newops(s,045275);opnd(s,000033,DMPR);lmi_dmpr(s,0);right(s);opnd(s,011033,DSU);lmi_dsu(s,D1_128);next(s);
 /* DMP SL1R ROOTMU; DMP SLR* X 0 -3,2; DDV VXSC RCNORM URRECT. */
 lmi_newops(s,076405);opnd(s,000021,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,053605);opnd(s,000025,DMP);lmi_dmp(s,0);right(s);gshift(s,056601);next(s);
 lmi_newops(s,074271);opnd(s,000043,DDV);lmi_ddv(s,0);right(s);opnd(s,002647,VXSC);lmi_vxsc(s,0);next(s);
 /* VSL1 PDDL XSQC(XI); SLR* DDV 0 -4,2 RCNORM; BDSU VXSC D1/256 VRECT. */
 lmi_newops(s,065372);shorts(s);right(s);opnd(s,000035,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,056257);gshift(s,056602);right(s);opnd(s,000043,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,074221);opnd(s,011047,BDSU);lmi_bdsu(s,D1_256);right(s);opnd(s,001511,VXSC);lmi_vxsc(s,0);next(s);
 /* VAD VSL8 (VAD pushes up); STADR; STODL VCV T; STODL TC X; STORE XPREV. */
 lmi_newops(s,042455);opnd(s,077626,VAD);lmi_vad(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,077626);lmi_unary(s,STADR);lmi_stadr(s,062234);lmi_dodload(s);lmi_diradres(s,000037,DLOAD);lmi_dload_op(s,0);next(s);
 stodl(s,015551,000025,0);
 store(s,001553);
 /* GOTO KEPRTN (an erasable CADR). */
 lmi_newops(s,077650);lmi_misc(s,002112,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);TRACE(s);
 return LCK_RETURNED;
}

uint8_t lm_conic_kepler_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LCK_ENTRY:return lck_entry(s);
 case LCK_1REV:return lck_1rev(s);
 case LCK_STOREMAX:return lck_storemax(s);
 case LCK_PERIODCH:return lck_periodch(s);
 case LCK_MODDONE:return lck_moddone(s);
 case LCK_STORBNDS:return lck_storbnds(s);
 case LCK_STOREMIN:return lck_storemin(s);
 case LCK_DXCOMP:return lck_dxcomp(s);
 case LCK_KEPLOOP:return lck_keploop(s);
 case LCK_DELTIME:return lck_deltime(s,io);
 case LCK_DELTIME2:return lck_deltime2(s,io);
 case LCK_DELTAIL:return lck_deltail(s);
 case LCK_KEPRET:return lck_kepret(s);
 case LCK_NDXCHNGE:return lck_ndxchnge(s);
 case LCK_POSDELX:return lck_posdelx(s);
 case LCK_PDXCHNGE:return lck_pdxchnge(s);
 case LCK_NEWDELX:return lck_newdelx(s);
 case LCK_BRNCHCTR:return lck_brnchctr(s);
 case LCK_BADX:return lck_badx(s);
 case LCK_TIMEOVFL:return lck_timeovfl(s);
 case LCK_NEGTOVFL:return lck_negtovfl(s);
 case LCK_CMNTOVFL:return lck_cmntovfl(s);
 case LCK_KEPCONVG:return lck_kepconvg(s);
 default:return LCK_FAILED;
 }
}

/* The actual entry: KEPPREP (ORBITAL_INTEGRATION, bank 11) after STQ
 * KEPRTN and AXC,1 10 or 2 (BON MOONFLAG), GOTO KEPLERN. KEPRTN (E4,1512)
 * holds the return of one of KEPPREP's three callers. */
#define KEPRTN 02112u
static uint8_t admitted(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120];agc_word x1;
 if(W[5]!=06070u||W[4]!=024000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(024000u|(W[3]>>8))||W[0165]!=W[6]||W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 x1=W[vac+046u];
 /* KEPRTN: the returns KEPPREP's STQ stores for its three callers (CALL at
  * 13,3250; STCALL at 11,2514 and 11,3010). */
 return W[0164]==02023u&&(x1==077775u||x1==077765u)&&(W[KEPRTN]==027252u||W[KEPRTN]==022516u||W[KEPRTN]==023012u);
}
uint8_t lm_conic_kepler(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase=LCK_ENTRY;unsigned guard;
 if(!s||!io||!admitted(s,io))return 0;
 /* PERIODCH and the iteration (at most 20 passes through CHECKCTR, plus
  * the overflow halvings) bound the chain; the guard only stops a runaway. */
 for(guard=0;guard<4000u&&phase<LCK_RETURNED;++guard)phase=lm_conic_kepler_step(s,io,phase);
 return phase==LCK_RETURNED?1u:phase==LCK_ABORTED?2u:0u;
}
