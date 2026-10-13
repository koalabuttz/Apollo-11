/* SPDX-License-Identifier: MIT */
/* Native Luminary099 time-radius and apsides subroutines
 * (Luminary099/CONIC_SUBROUTINES.agc): TIMERAD, TERMNVEC, CIRCULAR, BADR2
 * (bank 12 03547-03673) and APSIDES, GETECC, INFINAPO (12,3674-3741).
 * PARAM/GEOM, GETX, COMMNOUT, COGAOVFL, ABTCONIC and DELTIME are the
 * time-theta milestone's steps (src/lm_conic_timethet.c). Every interpretive
 * pair is compiled: its original order and operand words appear below only
 * as the literal bookkeeping arguments of the lm_interp_steps and
 * lm_interp_orders paths, and each runtime branch follows the outcome those
 * paths return. LODPHALF, KEPZERO and LDPOSMAX (bank 12) are read from the
 * certified fixed-data table; the CONICS1 constants and DP1/4 appear as
 * literal words. */
#include "lm_conic_timerad.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#ifdef LM_CONIC_TIMERAD_TRACE
void lm_conic_timerad_trace(const apollo_overlay_state *);
#define TRACE(s) lm_conic_timerad_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,SIGN=07655,VXSC=07401,DLOAD=06052,PDDL=06523,TSLC=07616,DDV=07577,
 GSHIFT=07621,DOT=07331,VXV=07460,DSU=07005,BDSU=07062,DAD=06775,DMP=07572,VAD=06751,VSU=06747,
 SETPD=07613,SQRT=03207,DSQ=03174,UNIT=03023,PUSH=03247,CALLITA=02507,BZEGOTO=02467,BPLBMN=02474,
 SWITCH=02516,BOVB=02457};

/* CONICS1 constants (bank 4, 03030-03063). */
static const agc_word D1_4[2]={010000,0},D1_8[2]={04000,0},D1_16[2]={02000,0},D1_32[2]={01000,0},
 D1_64[2]={0400,0};
/* DP1/4 (ORBITAL_INTEGRATION, 12,3755). */
static const agc_word DP1_4[2]={010000,0};

/* Certified fixed words of bank 12 at 12,addr. */
static uint8_t bank12(uint16_t addr,agc_word k[2]){
 return apollo_fixed_data_read(LM,(uint16_t)(012u*1024u+addr-02000u),&k[0])&&
        apollo_fixed_data_read(LM,(uint16_t)(012u*1024u+addr-02000u+1u),&k[1]);
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
/* STODL/STOVL: the store, then the load of the given word. */
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word,const agc_word *k){
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);lmi_dload_op(s,k);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
static void branch_taken(apollo_overlay_state *s){lmi_goto(s);TRACE(s);}
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
/* STQ SETPD (return word) 0; BOV +1; VLOAD PDVL RVEC VVEC; CALL PARAM. */
static uint8_t head(apollo_overlay_state *s,agc_word plus1){
 lmi_newops(s,040220);lmi_misc(s,002710,CALLITA);lmi_ita(s);right(s);lmi_direct(s,000001,SETPD);lmi_setpd(s);setpd_next(s);
 lmi_newops(s,077600);
 switch(bov(s,plus1)){case 0:next(s);break;case 1:break;default:return LTT_FAILED;}
 lmi_newops(s,063375);opnd(s,002655,VLOAD);lmi_vload(s,0);right(s);opnd(s,002744,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,077624);call(s,011064);
 return LTT_PARAM;
}

uint8_t ltr_timerad(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* TIMERAD STQ SETPD RTNTR 0; BOV +1; VLOAD PDVL RVEC VVEC; CALL PARAM. */
 return head(s,025554);
}
uint8_t ltr_afterparam(apollo_overlay_state *s,p66_bias_io *io){
 /* BOV DLOAD COGAOVFL D1/32; DSU DMP R1A P; SQRT DMP COGA; SL4 VXSC U2. */
 lmi_newops(s,071200);
 switch(bov(s,024764)){case 0:break;case 1:return LTT_COGAOVFL;default:return LTT_FAILED;}
 right(s);opnd(s,011043,DLOAD);lmi_dload_op(s,D1_32);next(s);
 lmi_newops(s,041225);opnd(s,002742,DSU);lmi_dsu(s,0);right(s);opnd(s,002740,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,041366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;right(s);opnd(s,002766,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,074212);shorts(s);right(s);opnd(s,002712,VXSC);lmi_vxsc(s,0);next(s);
 /* PDDL DSU D1/64 R1A; VXSC VSU UR1 (VSU pushes up); VSL4 UNIT; BOV CIRCULAR. */
 lmi_newops(s,045325);opnd(s,011035,PDDL);lmi_pddl_op(s,D1_64);right(s);opnd(s,002742,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,052361);opnd(s,002722,VXSC);lmi_vxsc(s,0);right(s);opnd(s,053512,VSU);lmi_vsu(s,0);next(s);
 lmi_newops(s,053512);shorts(s);right(s);lmi_unary(s,UNIT);if(lmi_unit(s))return LTT_ABORTED;next(s);
 lmi_newops(s,077600);
 switch(bov(s,025660)){case 0:next(s);break;case 1:return LTR_CIRCULAR;default:return LTT_FAILED;}
 /* PDDL NORM RDESIRED X1; PDDL DMP R1 P; SL* DDV 0,1 (DDV pushes up);
  * DSU DDV D1/16 36D; STORE COSF. */
 lmi_newops(s,060325);opnd(s,002756,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,041325);opnd(s,000041,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,002740,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,056257);gshift(s,020201);right(s);opnd(s,056225,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,056225);opnd(s,011041,DSU);lmi_dsu(s,D1_16);right(s);opnd(s,000045,DDV);lmi_ddv(s,0);next(s);
 store(s,000031);
 /* BOV DSQ BADR2; BDSU BMN D1/4 BADR2; SQRT SIGN SGNRDOT; CLEAR APSESW. */
 lmi_newops(s,063400);
 switch(bov(s,025664)){case 0:break;case 1:return LTR_BADR2;default:return LTT_FAILED;}
 right(s);unary(s,DSQ,lmi_dsq);next(s);
 lmi_newops(s,050021);opnd(s,011037,BDSU);lmi_bdsu(s,D1_4);right(s);
 if(bplbmn(s,025664))return LTR_BADR2;
 next(s);
 lmi_newops(s,075366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;right(s);opnd(s,002755,SIGN);lmi_sign(s,0);next(s);
 lmi_newops(s,077614);if(sw(s,io,004272))return LTT_FAILED;next(s);
 return LTR_TERMNVEC;
}
uint8_t ltr_termnvec(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* TERMNVEC VXSC VSL1 UN; VXV PDVL 0D (PDVL pushes up); VXSC VAD COSF (VAD
  * pushes up); VSL1 PUSH; DOT DDV UR1 DP1/4; SR1 BOV +1. */
 lmi_newops(s,076561);opnd(s,002674,VXSC);lmi_vxsc(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,063235);opnd(s,000001,VXV);lmi_vxv(s,0);right(s);opnd(s,053361,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,053361);opnd(s,000031,VXSC);lmi_vxsc(s,0);right(s);opnd(s,041572,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,041572);shorts(s);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,056241);opnd(s,002722,DOT);lmi_dot(s,0);right(s);opnd(s,025756,DDV);lmi_ddv(s,DP1_4);next(s);
 lmi_newops(s,040142);shorts(s);right(s);
 switch(bov(s,025644)){case 0:next(s);break;case 1:break;default:return LTT_FAILED;}
 /* STOVL CSTH UR1; VXV VSL1 (VXV pushes up); DOT SL1 UN; STODL SNTH P;
  * CALL GETX. */
 stovl(s,026732,002722);
 lmi_newops(s,076435);opnd(s,072441,VXV);lmi_vxv(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,072441);opnd(s,002674,DOT);lmi_dot(s,0);right(s);shorts(s);next(s);
 stodl(s,016730,002740,0);
 lmi_newops(s,077624);call(s,024767);
 return LTT_GETX;
}
uint8_t ltr_soln(apollo_overlay_state *s,p66_bias_io *io){
 /* CLRGO SOLNSW COMMNOUT. */
 lmi_newops(s,077614);if(!sw(s,io,002634))return LTT_FAILED;lmi_swbranch(s,024747);TRACE(s);
 return LTT_COMMNOUT;
}
uint8_t ltr_circular(apollo_overlay_state *s,p66_bias_io *io){
 /* CIRCULAR SETPD SETGO 0 SOLNSW ABTCONIC. */
 lmi_newops(s,043001);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);
 if(!sw(s,io,002434))return LTT_FAILED;
 lmi_swbranch(s,025742);TRACE(s);
 return LTT_ABTCONIC;
}
uint8_t ltr_badr2(apollo_overlay_state *s,p66_bias_io *io){
 agc_word half[2],zero[2];
 /* BADR2 DLOAD SIGN LODPHALF COSF; STODL COSF KEPZERO; SETGO APSESW TERMNVEC. */
 if(!bank12(02004,half)||!bank12(02006,zero))return LTT_FAILED;
 lmi_newops(s,075345);opnd(s,024005,DLOAD);lmi_dload_op(s,half);right(s);opnd(s,000031,SIGN);lmi_sign(s,0);next(s);
 stodl(s,014031,024007,zero);
 lmi_newops(s,077614);if(!sw(s,io,004032))return LTT_FAILED;lmi_swbranch(s,025630);TRACE(s);
 return LTR_TERMNVEC;
}
uint8_t ltr_apsides(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* APSIDES STQ SETPD RTNAPSE 0D; BOV +1; VLOAD PDVL RVEC VVEC; CALL PARAM. */
 return head(s,025701);
}
uint8_t ltr_getecc(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* BOV GETECC (either way to GETECC). */
 lmi_newops(s,077600);
 switch(bov(s,025710)){case 0:next(s);break;case 1:break;default:return LTT_FAILED;}
 /* GETECC DMP SL4 R1A; BDSU SQRT D1/64; STORE ECC; DAD PDDL D1/8 R1;
  * DMP SL1 P; DDV (pushes up); PDDL NORM R1A X1; PDDL SL* R1 0 -5,1. */
 lmi_newops(s,042405);opnd(s,002742,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,075421);opnd(s,011035,BDSU);lmi_bdsu(s,D1_64);right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTT_ABORTED;next(s);
 store(s,002752);
 lmi_newops(s,065215);opnd(s,011031,DAD);lmi_dad(s,D1_8);right(s);opnd(s,000041,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,072405);opnd(s,002740,DMP);lmi_dmp(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,077671);opnd(s,060325,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,060325);opnd(s,002742,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 lmi_newops(s,053725);opnd(s,000041,PDDL);lmi_pddl_op(s,0);right(s);gshift(s,020174);next(s);
 /* DDV DSU (both push up); BOV BMN INFINAPO INFINAPO; GOTO RTNAPSE. */
 lmi_newops(s,045271);opnd(s,050000,DDV);lmi_ddv(s,0);right(s);opnd(s,050000,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,050000);
 switch(bov(s,025737)){case 0:break;case 1:return LTR_INFINAPO;default:return LTT_FAILED;}
 right(s);
 if(bplbmn(s,025737))return LTR_INFINAPO;
 next(s);
 lmi_newops(s,077650);go_erasable(s,002710);
 return LTT_RETURNED;
}
uint8_t ltr_infinapo(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[2];
 (void)io;
 /* INFINAPO DLOAD GOTO LDPOSMAX RTNAPSE. */
 if(!bank12(02017,k))return LTT_FAILED;
 lmi_newops(s,052145);opnd(s,024020,DLOAD);lmi_dload_op(s,k);right(s);go_erasable(s,002710);
 return LTT_RETURNED;
}

uint8_t ltr_remap(const apollo_overlay_state *s,uint8_t next){
 switch(next){
 case LTT_AFTERPARAM:return W[0164]==03561u?LTR_AFTERPARAM:W[0164]==03706u?LTR_GETECC:LTT_FAILED;
 case LTT_COMMNOUT:return W[0164]==03655u?LTR_SOLN:LTT_FAILED;
 case LTT_ENTRY:case LTT_HAVENORM:case LTT_NEWSTATE:case LTT_FINAL:return LTT_FAILED;
 default:return next;
 }
}
uint8_t lm_conic_timerad_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LTR_TIMERAD:return ltr_timerad(s,io);
 case LTR_AFTERPARAM:return ltr_afterparam(s,io);
 case LTR_TERMNVEC:return ltr_termnvec(s,io);
 case LTR_SOLN:return ltr_soln(s,io);
 case LTR_CIRCULAR:return ltr_circular(s,io);
 case LTR_BADR2:return ltr_badr2(s,io);
 case LTR_APSIDES:return ltr_apsides(s,io);
 case LTR_GETECC:return ltr_getecc(s,io);
 case LTR_INFINAPO:return ltr_infinapo(s,io);
 case LTT_ENTRY:case LTT_HAVENORM:case LTT_NEWSTATE:case LTT_FINAL:return LTT_FAILED;
 default:
  if(phase>=LTT_RETURNED)return LTT_FAILED;
  return ltr_remap(s,lm_conic_timethet_step(s,io,phase));
 }
}

/* The actual entries: P32/P72 CSI's STCALL RDESIRED TIMERAD (return
 * 35,2106, RVSW set) and PERIAPO's STCALL XXXALT APSIDES (return 23,2333);
 * NEWOPS at the routine with QPRET the return, X1 -2 or -10 (RTX1). */
static uint8_t admitted(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120];agc_word x1,q;
 if(W[5]!=06070u||W[4]!=024000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(024000u|(W[3]>>8))||W[0165]!=W[6]||W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 x1=W[vac+046u];q=W[vac+052u];
 if(x1!=077775u&&x1!=077765u)return 0;
 if(W[0164]==03547u)return q==072106u&&(W[0103]&0400u)?LTR_TIMERAD:0u;
 if(W[0164]==03674u)return q==046333u?LTR_APSIDES:0u;
 return 0;
}
uint8_t lm_conic_timerad(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;unsigned guard;
 if(!s||!io||!(phase=admitted(s,io)))return 0;
 /* GETX's loops run at most four passes each; the guard only stops a
  * runaway. */
 for(guard=0;guard<200u&&!LM_CONIC_TIMERAD_DONE(phase);++guard)phase=lm_conic_timerad_step(s,io,phase);
 return phase==LTT_RETURNED?1u:phase==LTT_ABORTED?2u:phase==LTT_CONICABORT?3u:0u;
}
