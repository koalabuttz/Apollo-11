/* SPDX-License-Identifier: MIT */
/* Luminary099 IMU_PERFORMANCE_TEST_2 and IMU_PERFORMANCE_TESTS_4 as native
 * routines of the control-transfer ABI; see include/lm_imu_performance.h
 * and docs/LM-IMU-PERFORMANCE.md. Basic code: each instruction is one ai_*
 * effect on the image in source order, under a case label at its address
 * (INDEX is the explicit address offset); a TC or TCF within the file is a
 * goto, one to another file ends the run with Q and Z set. Interpretive
 * code: each order-pair word is a case of its own, running the compiled
 * lm_interp_steps/lm_interp_orders paths with the pair's words as literal
 * arguments; branches follow the outcomes those paths return. */
#include "lm_imu_performance.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "sband_trig.h"
#define W s->words

/* Erasable (EB 5 window where the source's EBANK is POSITON or XSM). */
#define A_ 0u
#define L_ 1u
#define Q_ 2u
#define EB_ 3u
#define CDUX 032u
#define PIPAX 037u
#define NEWJOB 067u
#define FLAGWRD3 077u
#define MPAC 0154u
#define FIXLOC 0120u
#define LOC 0164u
#define PUSHLOC 0166u
#define BANKSET 0165u
#define EDOP 023u
#define THETAD 0321u
#define DELVX 0324u
#define DSPTEM1 01045u
#define DSPTEM2 01050u
#define ERVECTOR 01404u
#define LENGTHOT 01412u
#define LOSVEC 01413u
#define NDXCTR 01414u
#define PIPINDEX 01415u
#define POSITON 01416u
#define QPLACE 01417u
#define QPLACES 01420u
#define SOUTHDR 01421u
#define TORQNDX 01440u
#define DRIFTT 01442u
#define DATAPL 01472u
#define DRIFTO 01502u
#define DRIFTI 01504u
#define ALTIM 01530u
#define ALTIMS 01531u
#define RESULTCT 01537u
#define GEOCOMPS 01562u
#define ERCOMP 01563u
#define ZERONDX 01571u
#define SECXT1 01572u      /* 1SECXT */
#define PERFDLAY 01574u
#define OVFLOWCK 01576u
#define XNB 01664u
/* Fixed constants (checked against the rope by
 * tools/lm_imu_performance_source.py). */
#define K_ZERO 0u
#define K_ONE 1u
#define K_TWO 2u
#define K_FIVE 5u
#define K_BIT5 020u
#define K_BIT10 01000u
#define K_BIT14 020000u
#define K_HALF 020000u
#define K_PRIO20 020000u
#define K_POSMAX 037777u
#define K_DEC17 021u       /* ND1 */
#define K_DEC58 072u
#define K_3990DEC 07626u
#define K_VN0641 01451u
#define K_VB06N98 01542u
#define K_XNBADR 01664u
#define K_ESTICADR 076500u
#define K_77DECML 0115u
#define K_ALXXXZ 01443u
#define K_OCT1601 01601u
#define K_OGCPL 02737u
#define K_EBANK5 02400u
#define K_EBANK7 03400u
/* Other files. */
#define X_BANKCALL 04616u
#define X_FINETIME 04102u
#define X_FINDVAC 05105u
#define X_CHANG1 05122u
#define X_CHANG2 05126u
#define X_JOBSLEEP 05133u
#define X_JOBWAKE 05137u
#define X_ENDOFJOB 05155u
#define X_TWIDDLE 05173u
#define X_TASKOVER 05261u
#define X_LONGCALL 05277u
#define X_NEWMODEX 05311u
#define X_NEWMODEA 05314u
#define X_ENDEXT 05472u
#define X_DOWNFLAG 05516u
#define X_ALARM 05567u
#define X_CCSHOLE 05705u
#define X_VARALARM 05744u
#define X_ATIGINC 01400u
#define NEWOPS 06070u
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {DLOAD=06052,SLOAD=06501,PDDL=06523,DAD=06775,DSU=07005,BDSU=07062,DMP=07572,DMPR=07574,DDV=07577,
 VAD=06751,VXSC=07401,MXV=07334,VXM=07337,GSHIFT=07621,SETPD=07613,STADR=06354,VLOAD=06505,
 COMP=07670,PUSH=03247,VDEF=03232,COSINE=03517,SINE=03530,RVQ=03274,
 AXT=02344,LXA=02354,LXC=02360,SXA=02364,XAD=02400,INCR=02406,TIX=02415,
 RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,CALLITA=02507};

/* Bank-37 data the interpretive operands name (OMEG/MS at 02000-02001; DEC585
 * through GEORGEK at 03055-03112) and DPPOSMAX (bank 23, 02527-02530). */
static const agc_word OMEGMS[2]={07623u,026552u};
static const agc_word B37DATA[036]={06200u,0u,0u,0u,0u,0u,0u,1u,4u,2u,0220u,077776u,035730u,035u,010317u,017550u,
 0115u,01443u,04133u,02265u,057223u,066451u,05427u,012577u,077567u,044202u,024276u,014066u,023073u,011773u};
static const agc_word DPPOSMAX[2]={037777u,037777u};

enum {GO=0,STOP=1,REFUSE=2};
#define RUN_LIMIT 4000u

/* The fixed operand words ADDRWD names after address formation (n words),
 * or 0 for an erasable operand. Returns 0 for a fixed address outside the
 * data above. */
static int operand(const apollo_overlay_state *s,unsigned n,const agc_word **k){
 unsigned a=W[0116],bank=W[4]>>10;
 *k=0;
 if(a<02000u)return 1;
 if(a>=04000u)return 0;
 if(bank==037u&&a>=03055u&&a+n<=03113u){*k=B37DATA+(a-03055u);return 1;}
 if(bank==037u&&a==02000u&&n<=2u){*k=OMEGMS;return 1;}
 if(bank==023u&&a==02527u&&n<=2u){*k=DPPOSMAX;return 1;}
 return 0;
}
#define K(n) do{if(!operand(s,(n),&k))return REFUSE;}while(0)

/* ---- Interpretive helpers -------------------------------------------- */
/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS; positive goes
 * to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS NEWJOB; TCF CHANG2). */
static int next(apollo_overlay_state *s){
 agc_word j=W[NEWJOB];
 if(j==0u){lmi_danzig_next(s);return GO;}
 if(j>037777u)return REFUSE;
 ai_ca(s,BANKSET);(void)ai_ts(s,6);(void)ai_ccs(s,EDOP);(void)ai_ccs(s,NEWJOB);W[5]=X_CHANG2;return STOP;
}
#define NEXT return next(s)
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static int store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);return next(s);}
static int stodl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dodload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,DLOAD);else lmi_diradres(s,word,DLOAD);
 K(2);lmi_dload_op(s,k);return next(s);
}
static int stovl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);K(6);lmi_vload(s,k);return next(s);
}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
static int trig(apollo_overlay_state *s,uint8_t cosine){lmi_unary(s,cosine?COSINE:SINE);return sband_trig(s,APOLLO_FIXED_LM,cosine)?GO:REFUSE;}
#define TRIG(c) do{if(trig(s,(c)))return REFUSE;}while(0)
/* SLOAD: ZL; INDEX ADDRWD, CA 0; SLOAD2 DXCH MPAC; CAF ZERO; TS MPAC +2;
 * NEWMODE TS MODE (the fixed word when ADDRWD names one). */
static int sload(apollo_overlay_state *s,agc_word word){const agc_word *k;
 lmi_direct(s,word,SLOAD);K(1);
 if(!k){lmi_sload(s);return GO;}
 W[1]=0;ai_caf(s,k[0]);ai_dxch(s,MPAC);ai_caf(s,0);(void)ai_ts(s,MPAC+2u);(void)ai_ts(s,0163);W[5]=06061;return GO;
}
static void misc(apollo_overlay_state *s,agc_word word,agc_word target){lmi_misc(s,word,target);}
static int rtb(apollo_overlay_state *s,agc_word word){misc(s,word,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);return GO;}
static int call(apollo_overlay_state *s,agc_word word){misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;}
/* BPL/BMN: 1 (GOTO taken, at the target's NEWOPS) or 0 (DANZIG). */
static int bplbmn(apollo_overlay_state *s,agc_word word){misc(s,word,BPLBMN);if(!lmi_bplbmn(s))return 0;lmi_goto(s);return 1;}
/* TIX: 1 when it branches (at the target's NEWOPS). */
static int tix(apollo_overlay_state *s,agc_word word){misc(s,word,TIX);if(!lmi_tix(s))return 0;lmi_goto(s);return 1;}
static int rvq(apollo_overlay_state *s){
 if(!(W[(W[FIXLOC]+052u)&03777u]&074000u))return REFUSE;
 lmi_unary(s,RVQ);lmi_rvq(s);return GO;
}
static int exit_(apollo_overlay_state *s){lmi_newops(s,077776);lmi_exit(s);return GO;}

/* One order-pair word at LOC (Z at NEWOPS). */
static int interpretive(apollo_overlay_state *s,lm_ipt_io *io){
 const agc_word *k;
 (void)io;
 switch(W[LOC]){
 /* LATAZCHK DLOAD SL2 LATITUDE; STODL DSPTEM1 +1 AZIMUTH; RTB EXIT 1STO2S. */
 case 02021:lmi_newops(s,062545);opnd(s,002403,DLOAD);K(2);lmi_dload_op(s,k);right(s);shorts(s);NEXT;
 case 02023:return stodl(s,015047,002401);
 case 02025:lmi_newops(s,077434);return rtb(s,021614);
 /* SLOAD RTB DSPTEM1 CDULOGIC; STORE AZIMUTH; SLOAD SR2 DSPTEM1 +1; STORE
  * LATITUDE; COS DCOMP; SL1; STODL WANGI LATITUDE; SIN SL1; STODL WANGO
  * AZIMUTH; PUSH SIN; STORE YNB +2; STODL ZNB +4 (pushdown); COS; STORE
  * YNB +4; DCOMP; POSGMBL STCALL ZNB +2 CALCGA; EXIT. */
 case 02040:lmi_newops(s,047135);if(sload(s,001046))return REFUSE;right(s);return rtb(s,021576);
 case 02043:return store(s,002401);
 case 02044:lmi_newops(s,060535);if(sload(s,001047))return REFUSE;right(s);shorts(s);NEXT;
 case 02046:return store(s,002403);
 case 02047:lmi_newops(s,057546);TRIG(1);right(s);unary(s,COMP,lmi_comp);NEXT;
 case 02050:lmi_newops(s,077752);shorts(s);NEXT;
 case 02051:return stodl(s,016437,002403);
 case 02053:lmi_newops(s,072556);TRIG(0);right(s);shorts(s);NEXT;
 case 02054:return stodl(s,016435,002401);
 case 02056:lmi_newops(s,073406);unary(s,PUSH,lmi_push);right(s);TRIG(0);NEXT;
 case 02057:return store(s,002675);
 case 02060:return stodl(s,016705,077746);
 case 02061:lmi_newops(s,077746);TRIG(1);NEXT;
 case 02062:return store(s,002677);
 case 02063:lmi_newops(s,077676);unary(s,COMP,lmi_comp);NEXT;
 case 02064:lmi_store_code(s,036703);lmi_15bitadr(s,047353,CALLITA);lmi_call(s);lmi_goto(s);return GO;
 case 02066:return exit_(s);
 /* DLOAD DSU DATAPL +6 DATAPL +2; BPL CALL AINGOTN OVERFFIX. */
 case 02211:lmi_newops(s,045345);opnd(s,002501,DLOAD);K(2);lmi_dload_op(s,k);right(s);opnd(s,002475,DSU);K(2);lmi_dsu(s,k);NEXT;
 case 02214:lmi_newops(s,045044);if(bplbmn(s,076217))return GO;right(s);return call(s,076275);
 /* AINGOTN PDDL DDV DATAPL +4 (pushdown); DMPR RTB DEC585 SGNAGREE;
  * STORE DSPTEM2; EXIT. */
 case 02217:lmi_newops(s,056325);opnd(s,002477,PDDL);K(2);lmi_pddl_op(s,k);right(s);opnd(s,047075,DDV);lmi_ddv(s,0);NEXT;
 case 02221:lmi_newops(s,047075);opnd(s,037056,DMPR);K(2);lmi_dmpr(s,k);right(s);return rtb(s,021612);
 case 02224:return store(s,001051);
 case 02225:return exit_(s);
 /* OVERFFIX DAD DAD DPPOSMAX ONEDPP; RVQ. */
 case 02275:lmi_newops(s,043215);opnd(s,006530,DAD);K(2);lmi_dad(s,k);right(s);opnd(s,037064,DAD);K(2);lmi_dad(s,k);NEXT;
 case 02300:lmi_newops(s,077616);return rvq(s);
 /* ERTHRVSE DLOAD PDDL SCHZEROS LATITUDE; COS DCOMP; PDDL SIN LATITUDE;
  * VDEF VXSC OMEG/MS; STORE ERVECTOR; RTB LOADTIME; STOVL TMARK SCHZEROS;
  * STORE ERCOMP; RVQ. */
 case 02401:lmi_newops(s,065345);opnd(s,037057,DLOAD);K(2);lmi_dload_op(s,k);right(s);opnd(s,002403,PDDL);K(2);lmi_pddl_op(s,k);NEXT;
 case 02404:lmi_newops(s,057546);TRIG(1);right(s);unary(s,COMP,lmi_comp);NEXT;
 case 02405:lmi_newops(s,073525);opnd(s,002403,PDDL);K(2);lmi_pddl_op(s,k);right(s);TRIG(0);NEXT;
 case 02407:lmi_newops(s,074266);unary(s,VDEF,lmi_vdef);right(s);opnd(s,036001,VXSC);K(2);lmi_vxsc(s,k);NEXT;
 case 02411:return store(s,002405);
 case 02412:lmi_newops(s,077634);return rtb(s,021573);
 case 02414:return stovl(s,026433,037057);
 case 02416:return store(s,002564);
 case 02417:lmi_newops(s,077616);return rvq(s);
 /* EARTHR ITA RTB S2 LOADTIME; STORE TEMPTIME; DSU BPL TMARK ERTHR; CALL
  * OVERFFIX; ERTHR SL VXSC 9D ERVECTOR; MXV VAD XSM ERCOMP; STODL ERCOMP
  * TEMPTIME; STORE TMARK; AXT,1 RTB ECADR ERCOMP PULSEIMU; GOTO S2. */
 case 02420:lmi_newops(s,047020);misc(s,000051,CALLITA);lmi_ita(s);right(s);return rtb(s,021573);
 case 02423:return store(s,002431);
 case 02424:lmi_newops(s,051025);opnd(s,002433,DSU);K(2);lmi_dsu(s,k);right(s);if(bplbmn(s,076431))return GO;NEXT;
 case 02427:lmi_newops(s,077624);return call(s,076275);
 case 02431:lmi_newops(s,074261);opnd(s,020212,GSHIFT);lmi_gshift(s);right(s);opnd(s,002405,VXSC);K(2);lmi_vxsc(s,k);NEXT;
 case 02434:lmi_newops(s,053321);opnd(s,002643,MXV);K(18);lmi_mxv(s,k);right(s);opnd(s,002564,VAD);K(6);lmi_vad(s,k);NEXT;
 case 02437:return stodl(s,016564,002431);
 case 02441:return store(s,002433);
 case 02442:lmi_newops(s,047170);misc(s,002563,AXT);lmi_axt(s);right(s);return rtb(s,021705);
 case 02445:lmi_newops(s,077650);misc(s,000051,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);return GO;
 /* EARTHR* (after its TC INTPRET) CALL EARTHR; EXIT. */
 case 02452:lmi_newops(s,077624);return call(s,076420);
 case 02454:return exit_(s);
 /* ESTIMS SLOAD SCHZEROS; STOVL GCOMPSW -1 INTVAL +2; STOVL ALX1S
  * SCHZEROS; STORE DELVX; STORE GCOMP; SLOAD TORQNDX; DCOMP BMN VERTSKIP;
  * CALL ERTHRVSE; VERTSKIP EXIT. */
 case 02516:lmi_newops(s,077735);if(sload(s,037057))return REFUSE;NEXT;
 case 02520:return stovl(s,025477,037070);
 case 02522:return stovl(s,026445,037057);
 case 02524:return store(s,000325);
 case 02525:return store(s,001472);
 case 02526:lmi_newops(s,077735);if(sload(s,002441))return REFUSE;NEXT;
 case 02530:lmi_newops(s,050076);unary(s,COMP,lmi_comp);right(s);if(bplbmn(s,076534))return GO;NEXT;
 case 02532:lmi_newops(s,077624);return call(s,076401);
 case 02534:return exit_(s);
 /* NORMLOP DLOAD INTVAL; STOVL S1 DELVX; VXM VSL1 XSM; DLOAD DCOMP MPAC
  * +3; STODL DPIPAY MPAC +5; STORE DPIPAZ; SETPD AXT,1 0 8D; SLOAD DCOMP
  * GEOCOMPS; BMN PERFERAS. */
 case 02605:lmi_newops(s,077745);opnd(s,037066,DLOAD);K(2);lmi_dload_op(s,k);NEXT;
 case 02607:return stovl(s,024051,000325);
 case 02611:lmi_newops(s,076505);opnd(s,002643,VXM);K(18);lmi_vxm(s,k);right(s);shorts(s);NEXT;
 case 02613:lmi_newops(s,057545);opnd(s,000160,DLOAD);K(2);lmi_dload_op(s,k);right(s);unary(s,COMP,lmi_comp);NEXT;
 case 02615:return stodl(s,016523,000162);
 case 02617:return store(s,002527);
 case 02620:lmi_newops(s,076001);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);misc(s,000010,AXT);lmi_axt(s);NEXT;
 case 02623:lmi_newops(s,057535);if(sload(s,002563))return REFUSE;right(s);unary(s,COMP,lmi_comp);NEXT;
 case 02625:lmi_newops(s,077640);if(bplbmn(s,076772))return GO;NEXT;
 /* ALCGKK SLOAD BMN ALTIMS ALFLT3; ALKCG AXT,2 LXA,1 12D ALX1S; ALKCG2
  * DLOAD* INCR,1 ALFDK +144D,1 DEC -2; STORE ALDK +10D,2; TIX,2 SXA,1
  * ALKCG2 ALX1S. */
 case 02627:lmi_newops(s,050135);if(sload(s,002532))return REFUSE;right(s);if(bplbmn(s,076644))return GO;NEXT;
 case 02632:lmi_newops(s,072174);misc(s,000014,AXT);lmi_axt(s);right(s);misc(s,002444,LXA);lmi_lxa(s);NEXT;
 case 02635:lmi_newops(s,062143);lmi_indexed(s,002243,DLOAD);K(2);lmi_dload_op(s,k);right(s);misc(s,077775,INCR);lmi_incr(s);NEXT;
 case 02640:return store(s,012545);
 case 02641:lmi_newops(s,066104);if(tix(s,076635))return GO;right(s);misc(s,002444,SXA);lmi_sxa(s);NEXT;
 /* ALFLT3 AXT,1 8D; DELMLP DLOAD* DMP DPIPAY +8D,1 PIPASC; SLR BDSU* 9D
  * INTY +8D,1; STORE INTY +8D,1; PDDL DMP* VELSC VLAUN +8D,1; SL2R; DSU
  * STADR (pushdown); STORE DELM +8D,1; STORE DELM +10D,1; TIX,1 AXT,2
  * DELMLP 4. */
 case 02644:lmi_newops(s,077770);misc(s,000010,AXT);lmi_axt(s);NEXT;
 case 02646:lmi_newops(s,041343);lmi_indexed(s,002533,DLOAD);K(2);lmi_dload_op(s,k);right(s);opnd(s,037100,DMP);K(2);lmi_dmp(s,k);NEXT;
 case 02651:lmi_newops(s,043661);opnd(s,021212,GSHIFT);lmi_gshift(s);right(s);lmi_indexed(s,002501,BDSU);K(2);lmi_bdsu(s,k);NEXT;
 case 02654:return store(s,006501);
 case 02655:lmi_newops(s,040725);opnd(s,037102,PDDL);K(2);lmi_pddl_op(s,k);right(s);lmi_indexed(s,002521,DMP);K(2);lmi_dmp(s,k);NEXT;
 case 02660:lmi_newops(s,077732);shorts(s);NEXT;
 case 02661:lmi_newops(s,045425);opnd(s,071216,DSU);K(2);lmi_dsu(s,k);right(s);lmi_unary(s,STADR);lmi_stadr(s,071216);NEXT;
 case 02663:return store(s,006563);
 case 02664:lmi_newops(s,077100);if(tix(s,076646))return GO;right(s);misc(s,000004,AXT);lmi_axt(s);NEXT;
 /* ALILP DLOAD* DMPR* ALK +4,2 ALDK +4,2; STORE ALK +4,2; TIX,2 AXT,2
  * ALILP 8D. */
 case 02667:lmi_newops(s,056743);lmi_indexed(s,075324,DLOAD);K(2);lmi_dload_op(s,k);right(s);lmi_indexed(s,075240,DMPR);K(2);lmi_dmpr(s,k);NEXT;
 case 02672:return store(s,012453);
 case 02673:lmi_newops(s,077104);if(tix(s,076667))return GO;right(s);misc(s,000010,AXT);lmi_axt(s);NEXT;
 /* ALKLP LXC,1 SXA,1 CMPX1 CMPX1; DLOAD* DMPR* ALK +1,1 DELM +8D,2; DAD*
  * INTY +8D,2; STORE INTY +8D,2; DLOAD* DAD* ALK +12D,2 ALDK +12D,2; STORE
  * ALK +12D,2; DMPR* DAD* DELM +8D,2 INTY +16D,2; STORE INTY +16D,2;
  * DLOAD* DMP* ALSK +1,1 DELM +8D,2; SL1R DAD* VLAUN +8D,2; STORE VLAUN
  * +8D,2; TIX,2 AXT,1 ALKLP 8D. */
 case 02676:lmi_newops(s,066140);misc(s,002445,LXC);lmi_lxc(s);right(s);misc(s,002445,SXA);lmi_sxa(s);NEXT;
 case 02701:lmi_newops(s,056743);lmi_indexed(s,002450,DLOAD);K(2);lmi_dload_op(s,k);right(s);lmi_indexed(s,075216,DMPR);K(2);lmi_dmpr(s,k);NEXT;
 case 02704:lmi_newops(s,077613);lmi_indexed(s,075276,DAD);K(2);lmi_dad(s,k);NEXT;
 case 02706:return store(s,012501);
 case 02707:lmi_newops(s,042743);lmi_indexed(s,075314,DLOAD);K(2);lmi_dload_op(s,k);right(s);lmi_indexed(s,075230,DAD);K(2);lmi_dad(s,k);NEXT;
 case 02712:return store(s,012463);
 case 02713:lmi_newops(s,042673);lmi_indexed(s,075216,DMPR);K(2);lmi_dmpr(s,k);right(s);lmi_indexed(s,075266,DAD);K(2);lmi_dad(s,k);NEXT;
 case 02716:return store(s,012511);
 case 02717:lmi_newops(s,040743);lmi_indexed(s,037105,DLOAD);K(2);lmi_dload_op(s,k);right(s);lmi_indexed(s,075216,DMP);K(2);lmi_dmp(s,k);NEXT;
 case 02722:lmi_newops(s,042772);shorts(s);right(s);lmi_indexed(s,075256,DAD);K(2);lmi_dad(s,k);NEXT;
 case 02724:return store(s,012521);
 case 02725:lmi_newops(s,076104);if(tix(s,076676))return GO;right(s);misc(s,000010,AXT);lmi_axt(s);NEXT;
 /* LOOSE DLOAD* PDDL* ACCWD +8D,1 VLAUN +8D,1; PDDL* VDEF POSNV +8D,1;
  * MXV VSL1 TRANSM1; DLOAD MPAC; STORE POSNV +8D,1; DLOAD MPAC +3; STORE
  * VLAUN +8D,1; DLOAD MPAC +5; STORE ACCWD +8D,1; TIX,1 LOOSE; AXT,2 AXT,1
  * 6 2. */
 case 02730:lmi_newops(s,064743);lmi_indexed(s,002523,DLOAD);K(2);lmi_dload_op(s,k);right(s);lmi_indexed(s,002521,PDDL);K(2);lmi_pddl_op(s,k);NEXT;
 case 02733:lmi_newops(s,055523);lmi_indexed(s,002531,PDDL);K(2);lmi_pddl_op(s,k);right(s);unary(s,VDEF,lmi_vdef);NEXT;
 case 02735:lmi_newops(s,076521);opnd(s,002001,MXV);K(18);lmi_mxv(s,k);right(s);shorts(s);NEXT;
 case 02737:lmi_newops(s,077745);opnd(s,000155,DLOAD);K(2);lmi_dload_op(s,k);NEXT;
 case 02741:return store(s,006531);
 case 02742:lmi_newops(s,077745);opnd(s,000160,DLOAD);K(2);lmi_dload_op(s,k);NEXT;
 case 02744:return store(s,006521);
 case 02745:lmi_newops(s,077745);opnd(s,000162,DLOAD);K(2);lmi_dload_op(s,k);NEXT;
 case 02747:return store(s,006523);
 case 02750:lmi_newops(s,077700);if(tix(s,076730))return GO;NEXT;
 case 02752:lmi_newops(s,076174);misc(s,000006,AXT);lmi_axt(s);right(s);misc(s,000002,AXT);lmi_axt(s);NEXT;
 /* BOOP DLOAD* DMPR ANGX +2,1 GEORGEJ; SR2R; PUSH SIN; SL3R XAD,1 X1;
  * STORE 16D,2; DLOAD (pushdown); COS; STORE 22D,2; TIX,2 BOOP;
  * PERFERAS EXIT. */
 case 02755:lmi_newops(s,057343);lmi_indexed(s,002503,DLOAD);K(2);lmi_dload_op(s,k);right(s);opnd(s,037110,DMPR);K(2);lmi_dmpr(s,k);NEXT;
 case 02760:lmi_newops(s,077722);shorts(s);NEXT;
 case 02761:lmi_newops(s,073406);unary(s,PUSH,lmi_push);right(s);TRIG(0);NEXT;
 case 02762:lmi_newops(s,056072);shorts(s);right(s);misc(s,000046,XAD);lmi_xad(s);NEXT;
 case 02764:return store(s,010021);
 case 02765:lmi_newops(s,077745);opnd(s,077746,DLOAD);K(2);lmi_dload_op(s,k);NEXT;
 case 02766:lmi_newops(s,077746);TRIG(1);NEXT;
 case 02767:return store(s,010027);
 case 02770:lmi_newops(s,077704);if(tix(s,076755))return GO;NEXT;
 case 02772:return exit_(s);
 /* SETUPER1 DLOAD PDDL ANGZ ANGY; PDDL VDEF ANGX; VCOMP VXSC GEORGEJ; MXV
  * VSR1 XSM; STORE OGC; EXIT. */
 case 03006:lmi_newops(s,065345);opnd(s,002473,DLOAD);K(2);lmi_dload_op(s,k);right(s);opnd(s,002477,PDDL);K(2);lmi_pddl_op(s,k);NEXT;
 case 03011:lmi_newops(s,055525);opnd(s,002501,PDDL);K(2);lmi_pddl_op(s,k);right(s);unary(s,VDEF,lmi_vdef);NEXT;
 case 03013:lmi_newops(s,074276);unary(s,COMP,lmi_comp);right(s);opnd(s,037110,VXSC);K(2);lmi_vxsc(s,k);NEXT;
 case 03015:lmi_newops(s,074521);opnd(s,002643,MXV);K(18);lmi_mxv(s,k);right(s);shorts(s);NEXT;
 case 03017:return store(s,002740);
 case 03020:return exit_(s);
 /* GEOSTRT4 (after its TC INTPRET) CALL ERTHRVSE; EXIT. */
 case 03030:lmi_newops(s,077624);return call(s,076401);
 case 03032:return exit_(s);
 default:return REFUSE;
 }
}

/* ---- Basic helpers ------------------------------------------------------ */
static int out(apollo_overlay_state *s,agc_word q,agc_word z){ai_tc(s,q);W[5]=z;return STOP;}
/* INDEX K: the index added to an erasable address a, refused when the sum
 * leaves 0..01777 (the instruction would change). -0 indexes as +0. */
static int ix(const apollo_overlay_state *s,unsigned k,unsigned a,unsigned *ea){
 agc_word v=ai_word(s,k);if(v==077777u)v=0;
 if(v>01777u-a)return 0;
 *ea=a+v;return 1;
}
#define IX(k,a) do{if(!ix(s,(k),(a),&ea))return REFUSE;}while(0)
/* TC to an erasable word E that holds a TC to fixed memory: Q = E +1 and
 * Z = that address. TC Q runs the word Q held before the TC (Q ends at 3). */
static int tc_erasable(apollo_overlay_state *s,unsigned e){
 agc_word v=e==Q_?W[Q_]:ai_word(s,e);
 if(v<02000u||v>07777u)return REFUSE;
 /* A TC into FINETIME, SWCALL or the interpreter: no caller of the file
  * stores one in its return words. */
 if((v>=04102u&&v<=04123u)||(v>=04621u&&v<=04630u)||(v>=06037u&&v<=07707u))return REFUSE;
 ai_tc(s,(agc_word)(e+1u));W[5]=v;return GO;
}
static void intpret(apollo_overlay_state *s,lm_ipt_io *io,agc_word ret){ai_tc(s,ret);W[5]=06037;lmi_intpret(s,&io->control);}
static void read_ch(apollo_overlay_state *s,agc_word v){ai_caf(s,v);}
/* A case label (and goto label) at each instruction address; straight-line
 * code falls through from one instruction to the next, as the source does. */
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#define AT(a) case a: L##a: __attribute__((unused))
#define GOTO(a) goto L##a
#define TC(r,a) do{ai_tc(s,(r));GOTO(a);}while(0)

/* Basic code at Z (bank 37, or FINETIME in fixed-fixed). */
static int basic(apollo_overlay_state *s,lm_ipt_io *io){
 unsigned ea;uint8_t c;
 switch(W[5]){
 /* REDO TC NEWMODEX (MM 07). */
 AT(02002);return out(s,02003,X_NEWMODEX);
 /* GEOIMUTT TC IMUZERR; IMUBACK zero NDXCTR, TORQNDX, OVFLOWCK; NBPOSPL
  * zero XNB with ZEROING; HALF to XNB; GUESS TC INTPRET. */
 AT(02004);TC(02005,02325);
 AT(02005);ai_caf(s,K_ZERO);
 AT(02006);(void)ai_ts(s,NDXCTR);
 AT(02007);(void)ai_ts(s,TORQNDX);
 AT(02010);(void)ai_ts(s,TORQNDX+1u);
 AT(02011);(void)ai_ts(s,OVFLOWCK);
 AT(02012);ai_caf(s,K_DEC17);
 AT(02013);(void)ai_ts(s,ZERONDX);
 AT(02014);ai_caf(s,K_XNBADR);
 AT(02015);TC(02016,02367);
 AT(02016);ai_caf(s,K_HALF);
 AT(02017);(void)ai_ts(s,XNB);
 AT(02020);intpret(s,io,02021);return GO;
 /* After 1STO2S: XCH MPAC; TS DSPTEM1; CAF VN0641; TC BANKCALL GOFLASH;
  * TC ENDTEST1 / TC +2 / TC -5; TC INTPRET. */
 AT(02027);ai_xch(s,MPAC);
 AT(02030);(void)ai_ts(s,DSPTEM1);
 AT(02031);ai_caf(s,K_VN0641);
 AT(02032);return out(s,02033,X_BANKCALL);
 AT(02034);TC(02035,02270);
 AT(02035);TC(02036,02037);
 AT(02036);TC(02037,02031);
 AT(02037);intpret(s,io,02040);return GO;
 /* After CALCGA's IMUCOARS: gimbal lock test, DOWNFLAG GLOKFAIL. */
 AT(02067);return out(s,02070,X_BANKCALL);
 AT(02071);ai_caf(s,K_BIT14);
 AT(02072);ai_mask(s,FLAGWRD3);
 AT(02073);if(ai_bzf(s))GOTO(02076);
 AT(02075);ai_incr(s,NDXCTR);
 AT(02076);return out(s,02077,X_DOWNFLAG);
 AT(02100);TC(02101,02315);
 /* CCS NDXCTR: TC PIPACHK / TC FINIMUDD / EXTEND DCA PERFDLAY / the DCA
  * word unextended (CA PERFDLAY +1); TC LONGCALL 2CADR GOESTIMS. */
 AT(02101);c=ai_ccs(s,NDXCTR);
  if(c==1u)GOTO(02103);
  if(c==2u)GOTO(02104);
  if(c==3u){ai_ca(s,PERFDLAY+1u);return out(s,02107,X_LONGCALL);}
 AT(02102);TC(02103,02126);
 AT(02103);TC(02104,02320);
 AT(02104);ai_dca(s,PERFDLAY);return out(s,02107,X_LONGCALL);
 AT(02111);ai_caf(s,K_ESTICADR);
 AT(02112);return out(s,02113,X_JOBSLEEP);
 AT(02113);ai_caf(s,K_ESTICADR);
 AT(02114);return out(s,02115,X_JOBWAKE);
 AT(02115);return out(s,02116,X_TASKOVER);
 /* TORQUE. */
 AT(02117);ai_caf(s,K_ZERO);
 AT(02120);(void)ai_ts(s,DSPTEM2);
 AT(02121);ai_ca(s,DRIFTI);
 AT(02122);(void)ai_ts(s,DSPTEM2+1u);
 AT(02123);IX(POSITON,SOUTHDR-1u);(void)ai_ts(s,ea);
 AT(02125);TC(02126,02457);
 /* PIPACHK INDEX NDXCTR; TC +1 (TC 02130 + NDXCTR). */
 AT(02126);{agc_word v=ai_word(s,NDXCTR);if(v==077777u)v=0;if(v>03777u-02130u)return REFUSE;ai_tc(s,02130);W[5]=(agc_word)(02130u+v);return GO;}
 AT(02130);TC(02131,02447);
 AT(02131);ai_caf(s,K_DEC17);
 AT(02132);(void)ai_ts(s,DATAPL+4u);
 AT(02133);ai_caf(s,K_DEC58);
 AT(02134);(void)ai_ts(s,LENGTHOT);
 AT(02135);ai_caf(s,K_ONE);
 AT(02136);(void)ai_ts(s,RESULTCT);
 AT(02137);ai_caf(s,K_ZERO);
 AT(02140);IX(PIPINDEX,PIPAX);(void)ai_ts(s,ea);
 AT(02142);(void)ai_ts(s,DATAPL);
 AT(02143);TC(02144,02332);
 AT(02144);io->control.interrupt_enabled=0;
 AT(02145);ai_caf(s,K_TWO);
 AT(02146);return out(s,02147,X_TWIDDLE);
 AT(02150);return out(s,02151,X_ENDOFJOB);
 /* PIPATASK. */
 AT(02151);ai_dim(s,LENGTHOT);
 AT(02153);ai_ca(s,LENGTHOT);
 AT(02154);if(ai_bzmf(s))GOTO(02161);
 AT(02156);ai_caf(s,K_BIT10);
 AT(02157);return out(s,02160,X_TWIDDLE);
 AT(02161);ai_caf(s,K_PRIO20);
 AT(02162);return out(s,02163,X_FINDVAC);
 AT(02165);return out(s,02166,X_TASKOVER);
 /* PIPJOBB INDEX NDXCTR; TC +1. */
 AT(02166);{agc_word v=ai_word(s,NDXCTR);if(v==077777u)v=0;if(v>03777u-02170u)return REFUSE;ai_tc(s,02170);W[5]=(agc_word)(02170u+v);return GO;}
 AT(02170);TC(02171,02447);
 AT(02171);ai_ca(s,LENGTHOT);
 AT(02172);if(ai_bzmf(s))GOTO(02175);
 AT(02174);return out(s,02175,X_ENDOFJOB);
 AT(02175);ai_caf(s,K_FIVE);
 AT(02176);(void)ai_ts(s,RESULTCT);
 AT(02177);TC(02200,02332);
 AT(02200);c=ai_ccs(s,DATAPL+1u);
  if(c==1u)GOTO(02202);
  if(c==2u)GOTO(02203);
  if(c==3u)GOTO(02204);
 AT(02201);TC(02202,02205);
 AT(02202);return out(s,02203,X_CCSHOLE);
 AT(02203);ai_cs(s,DATAPL+4u);
 AT(02204);(void)ai_ts(s,DATAPL+4u);
 AT(02205);ai_dcs(s,DATAPL);
 AT(02207);ai_das(s,DATAPL+4u);
 AT(02210);intpret(s,io,02211);return GO;
 /* After SGNAGREE's EXIT: CCS NDXCTR; TC COAALIGN / TC SHOW / VERTDRFT. */
 AT(02226);c=ai_ccs(s,NDXCTR);
  if(c==1u)GOTO(02230);
  if(c==2u)GOTO(02231);
  if(c==3u)GOTO(02232);
 AT(02227);TC(02230,02301);
 AT(02230);TC(02231,02457);
 AT(02231);ai_caf(s,K_3990DEC);
 AT(02232);(void)ai_ts(s,LENGTHOT);
 AT(02233);IX(POSITON,SOUTHDR-2u);ai_cs(s,ea);
 AT(02235);(void)ai_ts(s,DRIFTT);
 AT(02236);c=ai_ccs(s,PIPINDEX);
  if(c==1u)GOTO(02240);
  if(c==2u)GOTO(02241);
  if(c==3u)GOTO(02242);
 AT(02237);GOTO(02245);
 AT(02240);ai_csf(s,K_BIT5);
 AT(02241);ai_ads(s,ERCOMP+2u);
 AT(02242);ai_caf(s,K_BIT5);
 AT(02243);ai_ads(s,ERCOMP+4u);
 AT(02244);GOTO(02251);
 AT(02245);ai_csf(s,K_BIT5);
 AT(02246);ai_ads(s,ERCOMP+2u);
 AT(02247);ai_caf(s,K_BIT5);
 AT(02250);ai_ads(s,ERCOMP);
 AT(02251);TC(02252,02447);
 AT(02252);ai_caf(s,K_ZERO);
 AT(02253);(void)ai_ts(s,ERVECTOR);
 AT(02254);(void)ai_ts(s,ERVECTOR+1u);
 AT(02255);ai_caf(s,K_POSMAX);
 AT(02256);(void)ai_ts(s,TORQNDX);
 AT(02257);(void)ai_ts(s,TORQNDX+1u);
 AT(02260);ai_ca(s,CDUX);
 AT(02261);(void)ai_ts(s,LOSVEC);
 AT(02262);TC(02263,02500);
 AT(02263);ai_ca(s,DRIFTO);
 AT(02264);(void)ai_ts(s,DSPTEM2+1u);
 AT(02265);ai_caf(s,K_ZERO);
 AT(02266);(void)ai_ts(s,DSPTEM2);
 AT(02267);TC(02270,02457);
 /* ENDTEST1 DOWNFLAG IMUSE; CS ZERO; TC NEWMODEA; TC ENDEXT. */
 AT(02270);return out(s,02271,X_DOWNFLAG);
 AT(02272);ai_csf(s,K_ZERO);
 AT(02273);return out(s,02274,X_NEWMODEA);
 AT(02274);return out(s,02275,X_ENDEXT);
 /* COAALIGN, ALIGNCOA, IMUSLLLG, FINIMUDD, IMUZERR. */
 AT(02301);ai_qxch(s,ZERONDX);
 AT(02303);ai_caf(s,K_ZERO);
 AT(02304);(void)ai_ts(s,THETAD);
 AT(02305);(void)ai_ts(s,THETAD+1u);
 AT(02306);(void)ai_ts(s,THETAD+2u);
 AT(02307);return out(s,02310,X_BANKCALL);
 AT(02311);return out(s,02312,X_BANKCALL);
 AT(02313);TC(02314,03047);
 AT(02314);return tc_erasable(s,ZERONDX);
 AT(02315);ai_qxch(s,ZERONDX);
 AT(02317);TC(02320,02311);
 AT(02320);ai_qxch(s,ZERONDX);
 AT(02322);return out(s,02323,X_BANKCALL);
 AT(02324);TC(02325,02311);
 AT(02325);ai_qxch(s,ZERONDX);
 AT(02327);return out(s,02330,X_BANKCALL);
 AT(02331);TC(02332,02311);
 /* CHECKG: catch a PIPA pulse. CHECKG1 spins while PIPAX is unchanged and
  * no job is waiting, which only an interrupt ends. */
 AT(02332);ai_qxch(s,QPLACE);
 AT(02334);TC(02335,02342);
 AT(02335);io->control.interrupt_enabled=1;
 AT(02336);ai_ca(s,NEWJOB);
 AT(02337);if(ai_bzmf(s))GOTO(02346);
 AT(02341);return out(s,02342,X_CHANG1);
 AT(02342);io->control.interrupt_enabled=0;
 AT(02343);IX(PIPINDEX,PIPAX);ai_cs(s,ea);
 AT(02345);(void)ai_ts(s,ZERONDX);
 AT(02346);io->control.interrupt_enabled=0;
 AT(02347);IX(PIPINDEX,PIPAX);ai_ca(s,ea);
 AT(02351);ai_ad(s,ZERONDX);
 AT(02352);if(ai_bzf(s)){
  /* Back to CHECKG1: with NEWJOB not positive the same state recurs. */
  agc_word j=ai_word(s,NEWJOB);
  if(j==0u||j==077777u||j>037777u)return REFUSE;
  GOTO(02335);
 }
 AT(02354);IX(PIPINDEX,PIPAX);ai_ca(s,ea);
 AT(02356);IX(RESULTCT,DATAPL);(void)ai_ts(s,ea);
 AT(02360);TC(02361,04102);
 AT(02361);IX(RESULTCT,DATAPL+1u);(void)ai_ts(s,ea);
 AT(02363);IX(RESULTCT,DATAPL+2u);ai_lxch(s,ea);
 AT(02365);io->control.interrupt_enabled=1;
 AT(02366);return tc_erasable(s,QPLACE);
 /* ZEROING (A = count, L or A = first address): clear C(ZERONDX) +1 words. */
 AT(02367);(void)ai_ts(s,L_);
 AT(02370);GOTO(02372);
 AT(02371);(void)ai_ts(s,ZERONDX);
 AT(02372);ai_caf(s,K_ZERO);
 AT(02373);IX(L_,0u);if(ea<020u)return REFUSE;(void)ai_ts(s,ea);
 AT(02375);ai_incr(s,L_);
 AT(02376);c=ai_ccs(s,ZERONDX);
  if(c==1u)GOTO(02400);
  if(c>=2u)return REFUSE;
 AT(02377);GOTO(02371);
 AT(02400);return tc_erasable(s,Q_);
 /* EARTHR* (QPLACES); after its EXIT: TC IMUSLLLG; TC QPLACES. */
 AT(02447);ai_qxch(s,QPLACES);
 AT(02451);intpret(s,io,02452);return GO;
 AT(02455);TC(02456,02315);
 AT(02456);return tc_erasable(s,QPLACES);
 /* SHOW, SHOW1: flash V06 N98 until V33. */
 AT(02457);ai_qxch(s,QPLACE);
 AT(02461);ai_ca(s,POSITON);
 AT(02462);(void)ai_ts(s,DSPTEM2+2u);
 AT(02463);ai_caf(s,K_VB06N98);
 AT(02464);return out(s,02465,X_BANKCALL);
 AT(02466);TC(02467,02270);
 AT(02467);return tc_erasable(s,QPLACE);
 AT(02470);GOTO(02461);
 /* ESTIMS. */
 AT(02500);io->control.interrupt_enabled=0;
 AT(02501);ai_ca(s,SECXT1);
 AT(02502);return out(s,02503,X_TWIDDLE);
 AT(02504);ai_caf(s,K_ZERO);
 AT(02505);(void)ai_ts(s,PIPAX);
 AT(02506);(void)ai_ts(s,PIPAX+1u);
 AT(02507);(void)ai_ts(s,PIPAX+2u);
 AT(02510);io->control.interrupt_enabled=1;
 AT(02511);ai_caf(s,K_77DECML);
 AT(02512);(void)ai_ts(s,ZERONDX);
 AT(02513);ai_caf(s,K_ALXXXZ);
 AT(02514);TC(02515,02367);
 AT(02515);intpret(s,io,02516);return GO;
 AT(02535);TC(02536,03035);
 /* ALLOOP. */
 AT(02536);ai_ca(s,OVFLOWCK);
 AT(02537);if(ai_bzf(s))GOTO(02542);
 AT(02541);return out(s,02542,X_TASKOVER);
 AT(02542);c=ai_ccs(s,ALTIM);
  if(c==1u)GOTO(02544);
  if(c==2u)GOTO(02545);
  if(c==3u)GOTO(02546);
 AT(02543);ai_ca(s,A_);
 AT(02544);(void)ai_ts(s,ALTIMS);
 AT(02545);ai_cs(s,A_);
 AT(02546);(void)ai_ts(s,ALTIM);
 AT(02547);ai_csf(s,K_ONE);
 AT(02550);ai_ad(s,GEOCOMPS);
 AT(02551);if(ai_bzf(s))GOTO(02556);
 AT(02553);ai_ca(s,LENGTHOT);
 AT(02554);if(ai_bzmf(s))GOTO(02562);
 AT(02556);ai_ca(s,SECXT1);
 AT(02557);return out(s,02560,X_TWIDDLE);
 AT(02561);ai_caf(s,K_ZERO);
 AT(02562);ai_xch(s,PIPAX);
 AT(02563);(void)ai_ts(s,DELVX);
 AT(02564);ai_caf(s,K_ZERO);
 AT(02565);ai_xch(s,PIPAX+1u);
 AT(02566);(void)ai_ts(s,DELVX+2u);
 AT(02567);ai_caf(s,K_ZERO);
 AT(02570);ai_xch(s,PIPAX+2u);
 AT(02571);(void)ai_ts(s,DELVX+4u);
 AT(02572);ai_caf(s,K_PRIO20);
 AT(02573);return out(s,02574,X_FINDVAC);
 AT(02576);return out(s,02577,X_TASKOVER);
 /* ALFLT: CCS GEOCOMPS; TC +2 / TC NORMLOP / TC BANKCALL 1/PIPA. */
 AT(02577);c=ai_ccs(s,GEOCOMPS);
  if(c==1u)GOTO(02601);
  if(c==2u)GOTO(02602);
  if(c==3u)return REFUSE;
 AT(02600);TC(02601,02602);
 AT(02601);TC(02602,02604);
 AT(02602);return out(s,02603,X_BANKCALL);
 AT(02604);intpret(s,io,02605);return GO;
 /* PERFERAS (after its EXIT): EBANK 7; TC ATIGINC. */
 AT(02773);ai_caf(s,K_EBANK7);
 AT(02774);(void)ai_ts(s,EB_);
 AT(02775);return out(s,02776,X_ATIGINC);
 AT(02776);c=ai_ccs(s,LENGTHOT);
  if(c==1u)GOTO(03000);
  if(c==2u)GOTO(03001);
  if(c==3u)GOTO(03002);
 AT(02777);TC(03000,03034);
 AT(03000);c=ai_ccs(s,TORQNDX);
  if(c==1u)GOTO(03002);
  if(c==2u)GOTO(03003);
  if(c==3u)GOTO(03004);
 AT(03001);GOTO(03003);
 AT(03002);TC(03003,03005);
 AT(03003);ai_ca(s,CDUX);
 AT(03004);(void)ai_ts(s,LOSVEC+1u);
 AT(03005);intpret(s,io,03006);return GO;
 AT(03021);ai_caf(s,K_OGCPL);
 AT(03022);return out(s,03023,X_BANKCALL);
 AT(03024);TC(03025,02315);
 /* GEOSTRT4: CCS TORQNDX; TC VALMIS / TC INTPRET. */
 AT(03025);c=ai_ccs(s,TORQNDX);
  if(c>=2u)return REFUSE;
  if(c==1u)GOTO(03027);
 AT(03026);TC(03027,02263);
 AT(03027);intpret(s,io,03030);return GO;
 AT(03033);TC(03034,02117);
 /* SLEEPIE TS LENGTHOT; CCS TORQNDX; TC EARTHR* / TC ENDOFJOB /
  * SOMEERRR. */
 AT(03034);(void)ai_ts(s,LENGTHOT);
 AT(03035);c=ai_ccs(s,TORQNDX);
  if(c==1u)GOTO(03037);
  if(c==2u)GOTO(03040);
  if(c==3u)GOTO(03041);
 AT(03036);TC(03037,02447);
 AT(03037);return out(s,03040,X_ENDOFJOB);
 AT(03040);ai_caf(s,K_EBANK5);
 AT(03041);(void)ai_ts(s,EB_);
 AT(03042);ai_caf(s,K_ONE);
 AT(03043);(void)ai_ts(s,OVFLOWCK);
 AT(03044);return out(s,03045,X_ALARM);
 AT(03046);TC(03047,02270);
 AT(03047);ai_caf(s,K_OCT1601);
 AT(03050);return out(s,03051,X_VARALARM);
 AT(03051);return out(s,03052,X_DOWNFLAG);
 AT(03053);return out(s,03054,X_ENDOFJOB);
 /* FINETIME (fixed-fixed; returns with interrupts inhibited). */
 AT(04102);io->control.interrupt_enabled=0;
 AT(04103);read_ch(s,io->loscalar);
 AT(04105);(void)ai_ts(s,L_);
 AT(04106);ai_caf(s,(agc_word)((((W[A_]>>1)&040000u)|(W[A_]&037777u))^io->loscalar));
 AT(04110);if(ai_bzf(s))GOTO(04115);
 AT(04112);read_ch(s,io->loscalar);
 AT(04114);(void)ai_ts(s,L_);
 AT(04115);ai_csf(s,K_POSMAX);
 AT(04116);ai_ad(s,L_);
 AT(04117);if(ai_bzf(s))return REFUSE;   /* FINETIME +1 again: the scaler is frozen */
 AT(04121);read_ch(s,io->hiscalar);
 AT(04123);return tc_erasable(s,Q_);
 default:return REFUSE;
 }
}

/* Is Z (or LOC at NEWOPS) the file's code? */
static int here(const apollo_overlay_state *s,const lm_ipt_io *io,int *interp){
 agc_word z=W[5];uint16_t ph;
 *interp=0;
 if(z==NEWOPS){ph=agc_fixed_physical(W[LOC],W[BANKSET],io->control.superbank);*interp=1;return ph>=037u*1024u+(02021u-02000u)&&ph<=037u*1024u+(03032u-02000u);}
 if(z>=04102u&&z<=04123u)return 1;
 if(z<02000u||z>=04000u)return 0;
 ph=agc_fixed_physical(z,W[4],io->control.superbank);
 return ph>=037u*1024u+(02002u-02000u)&&ph<=037u*1024u+(03053u-02000u);
}
/* FIXLOC one of the five VAC areas, PUSHLOC inside it, no order code left
 * over in EDOP (every NEWOPS the file reaches follows a whole pair or a
 * store code). */
static int work_area(const apollo_overlay_state *s){
 agc_word f=W[FIXLOC],p=W[PUSHLOC];
 if(W[EDOP])return 0;
 if(f!=0401u&&f!=0455u&&f!=0531u&&f!=0605u&&f!=0661u)return 0;
 return p>=f&&p<=f+042u;
}
static enum exec_status steps(apollo_overlay_state *s,lm_ipt_io *io){
 unsigned n;int st,interp;
 for(n=0;n<RUN_LIMIT;++n){
  if(!here(s,io,&interp))return n?EXEC_OK:EXEC_OUT_OF_SCOPE;
  /* NEWOPS reads the order word under FB, which DANZIG set from BANKSET. */
  if(interp&&(((W[4]^W[BANKSET])&076000u)||!work_area(s)))return EXEC_OUT_OF_SCOPE;
  st=interp?interpretive(s,io):basic(s,io);
  if(st==REFUSE)return EXEC_OUT_OF_SCOPE;
  if(st==STOP)return EXEC_OK;
 }
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status run(apollo_overlay_state *s,void *v){
 lm_ipt_io *io=v,entry;enum exec_status st;
 if(!s||!io||!io->undo||io->undo==s)return EXEC_OUT_OF_SCOPE;
 *io->undo=*s;entry=*io;
 st=steps(s,io);
 if(st!=EXEC_OK){*s=*entry.undo;*io=entry;}
 return st;
}
static enum exec_status routine(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return run(s,io);}
#define B37(a) (uint16_t)(037u*1024u+((a)-02000u))
#define RT(a) {B37(a),routine}
#define FFK(a) (uint16_t)(2048u+((a)-04000u))
#define FF(a) {FFK(a),routine}
static const agc_routine_entry basic_entries[IPT_BASIC_ROUTINES]={
 FF(04112),RT(02002),RT(02004),RT(02034),RT(02035),RT(02036),RT(02071),RT(02100),RT(02111),RT(02113),
 RT(02115),RT(02150),RT(02151),RT(02161),RT(02165),RT(02166),RT(02272),RT(02274),RT(02311),RT(02313),
 RT(02314),RT(02324),RT(02331),RT(02342),RT(02354),RT(02466),RT(02467),RT(02470),RT(02500),RT(02504),
 RT(02536),RT(02561),RT(02576),RT(02577),RT(02604),RT(02776),RT(03024),RT(03046),RT(03051),RT(03053)};
static const agc_routine_entry interp_entries[IPT_INTERPRETIVE_ROUTINES]={
 RT(02021),RT(02040),RT(02043),RT(02066),RT(02211),RT(02224),RT(02414),RT(02423),RT(02445),RT(02452),
 RT(02516),RT(02605),RT(03006),RT(03030)};
const agc_routine_table lm_imu_performance_routines={basic_entries,IPT_BASIC_ROUTINES};
const agc_routine_table lm_imu_performance_interpretive={interp_entries,IPT_INTERPRETIVE_ROUTINES};
const uint16_t ipt_routine_keys[IPT_ROUTINES]={
 FFK(04112),B37(02002),B37(02004),B37(02034),B37(02035),B37(02036),B37(02071),B37(02100),B37(02111),B37(02113),
 B37(02115),B37(02150),B37(02151),B37(02161),B37(02165),B37(02166),B37(02272),B37(02274),B37(02311),B37(02313),
 B37(02314),B37(02324),B37(02331),B37(02342),B37(02354),B37(02466),B37(02467),B37(02470),B37(02500),B37(02504),
 B37(02536),B37(02561),B37(02576),B37(02577),B37(02604),B37(02776),B37(03024),B37(03046),B37(03051),B37(03053),
 B37(02021),B37(02040),B37(02043),B37(02066),B37(02211),B37(02224),B37(02414),B37(02423),B37(02445),B37(02452),
 B37(02516),B37(02605),B37(03006),B37(03030)};
