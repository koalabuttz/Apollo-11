/* SPDX-License-Identifier: MIT */
/* Native Luminary099 time of free fall (Luminary099/TIME_OF_FREE_FALL.agc,
 * bank 27 words 03360-03772): TFFCONIC/TFFCONMU, TFFRP/RA, CALCTPER and
 * CALCTFF. Every interpretive pair is compiled: its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * the lm_interp_steps and lm_interp_orders paths, and each runtime branch
 * (BZE, BPL/BMN, BOV, the TFFSW switch orders) follows the outcome those
 * paths return. The bank-27 constants are not in the certified fixed-data
 * tables and appear as the literal assembled words; TFF1/4 and TFFZEROS
 * (bank 23) are read from the certified table. */
#include "lm_time_free_fall.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "rtb_agreement.h"
#ifdef LM_TIME_FREE_FALL_TRACE
void lm_time_free_fall_trace(const apollo_overlay_state *);
#define TRACE(s) lm_time_free_fall_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#include "agc_image_w.h"
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,SIGN=07655,VXSC=07401,DLOAD=06052,PDDL=06523,PDVL=06557,TSLC=07616,DDV=07577,BDDV=07603,
 GSHIFT=07621,DOT=07331,VXV=07460,DSU=07005,BDSU=07062,DAD=06775,DMP=07572,
 SQRT=03207,DSQ=03174,COMP=07670,UNIT=03023,ABVAL=03176,VSQ=03245,STADR=06354,RVQ=03274,PUSH=03247,
 RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,SWITCH=02516,BOVB=02457};

/* Bank 27 constants (27,3754-3771) and the T(X) table after TC POLY (27,3736). */
static const agc_word PI16[2]={06220,037553},LIM22[2]={037777,037700},DP22[2]={0,0100},
 DP2M3[2]={04000,0},DP2M4[2]={02000,0};
/* NEARONE (27,3770): also ASCENT_GUIDANCE's DLOAD operand. */
const agc_word lm_time_free_fall_nearone[2]={037777,037777};
#define NEARONE lm_time_free_fall_nearone
/* R300K (27,3766), assembled in this file but referenced by no Luminary099
 * code; kept for the source map and for any later caller. */
const agc_word lm_time_free_fall_r300k[2]={0305,011205};
static const agc_word TX[13]={04,012525,012525,071463,057703,04423,017645,074604,043667,01626,037256,077404,052071};

/* TFF1/4 (HIDP1/4, 23,2511) and TFFZEROS (HI6ZEROS, 23,2521). */
static uint8_t bank23(uint16_t sub,agc_word k[2]){
 return apollo_fixed_data_read(LM,(uint16_t)(023u*1024u+sub),&k[0])&&apollo_fixed_data_read(LM,(uint16_t)(023u*1024u+sub+1u),&k[1]);
}

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
/* BPL/BMN with the branch word; 1 when taken (at the target's NEWOPS). */
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
/* TCDANZIG = ENDT(X) (27,3753), entered by RTB with FBANK 27: TC DANZIG. */
void lm_time_free_fall_tcdanzig(apollo_overlay_state *s){ai_tc(s,03754);W[5]=06061;}
/* BOVB TCDANZIG (operand word 057753): 0 at DANZIG, 2 out of scope. The
 * same order in MEASUREMENT_INCORPORATION, ORBITAL_INTEGRATION and P20-P25
 * reaches DANZIG the same way. */
uint8_t lm_time_free_fall_bovb_tcdanzig(apollo_overlay_state *s){
 uint8_t r;
 lmi_misc(s,057753,BOVB);r=lmi_bov(s);
 if(r==LMI_BOV_NONE)return 0;
 if(r!=LMI_BOV_RTB)return 2;
 lmi_rtb(s);if(W[5]!=03753u)return 2;
 lm_time_free_fall_tcdanzig(s);return 0;
}
/* RTB T(X): TC POLY with the table, then TC DANZIG. */
static uint8_t rtb_tx(apollo_overlay_state *s){
 lmi_misc(s,057735,RTBBHIZ);
 if(ai_ccs(s,020)!=0u)return 0;
 lmi_rtb(s);if(W[5]!=03735u)return 0;
 ai_tc(s,03736);W[5]=07222;lmi_poly(s,TX);if(W[5]!=03753u)return 0;
 lm_time_free_fall_tcdanzig(s);return 1;
}

uint8_t ltf_entry(apollo_overlay_state *s){
 /* TFFCONIC STORE TFF/RTMU. */
 store(s,000037);return LTF_CONMU;
}
uint8_t ltf_conmu(apollo_overlay_state *s){
 agc_word k[2];
 /* TFFCONMU VLOAD UNIT RONE. */
 lmi_newops(s,053575);opnd(s,002207,VLOAD);lmi_vload(s,0);right(s);lmi_unary(s,UNIT);if(lmi_unit(s))return LTF_ABORTED;next(s);
 /* PDDL 36D; STORE RMAG1; NORM X1. */
 lmi_newops(s,077725);opnd(s,000045,PDDL);lmi_pddl_op(s,0);next(s);
 store(s,000015);
 lmi_newops(s,077701);opnd(s,000047,TSLC);lmi_tslc(s);next(s);
 /* STOVL NRMAG VONE; VXSC TFF/RTMU; STORE VONE'. */
 stovl(s,024041,002215);
 lmi_newops(s,077761);opnd(s,000037,VXSC);lmi_vxsc(s,0);next(s);
 store(s,002170);
 /* VXSC VXV NRMAG (VXV pushes up); VSL1 VSQ. */
 lmi_newops(s,047361);opnd(s,000041,VXSC);lmi_vxsc(s,0);right(s);opnd(s,047572,VXV);lmi_vxv(s,0);next(s);
 lmi_newops(s,047572);shorts(s);right(s);unary(s,VSQ,lmi_vsq);next(s);
 /* STODL TFFNP TFF1/4; DDV PDVL NRMAG VONE'; VSQ DCOMP; STORE TFFVSQ. */
 if(!bank23(0511,k))return LTF_FAILED;
 stodl(s,014035,006512,k);
 lmi_newops(s,063271);opnd(s,000041,DDV);lmi_ddv(s,0);right(s);opnd(s,002170,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,057436);unary(s,VSQ,lmi_vsq);right(s);unary(s,COMP,lmi_comp);next(s);
 store(s,000025);
 /* SR* DAD 0 -6,1 (DAD pushes up); STADR STORE TFFALFA. */
 lmi_newops(s,043257);gshift(s,020573);right(s);opnd(s,077626,DAD);lmi_dad(s,0);next(s);
 lmi_newops(s,077626);lmi_unary(s,STADR);lmi_stadr(s,077744);next(s);
 /* SL* PUSH 0 -6,1; ABS SQRT; NORM X2; STORE TFFRTALF. */
 lmi_newops(s,041457);gshift(s,020173);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,075446);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTF_ABORTED;right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTF_ABORTED;next(s);
 lmi_newops(s,077701);opnd(s,000050,TSLC);lmi_tslc(s);next(s);
 store(s,000031);
 return LTF_CONMU_END;
}
uint8_t ltf_conmu_end(apollo_overlay_state *s){
 agc_word k[2];
 /* DSQ SIGN (pushes up); BZE BDDV +2 TFF1/4. */
 lmi_newops(s,075316);unary(s,DSQ,lmi_dsq);right(s);opnd(s,055254,SIGN);lmi_sign(s,0);next(s);
 lmi_newops(s,055254);lmi_misc(s,057425,BZEGOTO);
 if(!lmi_bze(s)){
  if(!bank23(0511,k))return LTF_FAILED;
  right(s);opnd(s,006512,BDDV);lmi_bddv(s,k);next(s);
 }else TRACE(s);
 /* +2 STORE TFF1/ALF; DUMPCNIC RVQ. */
 store(s,000027);rvq(s);return LTF_RETURNED;
}
uint8_t ltf_rpra(apollo_overlay_state *s){
 agc_word k[2];
 if(!bank23(0511,k))return LTF_FAILED;
 /* TFFRP/RA DLOAD DMP TFFALFA TFFNP; SR* DCOMP 0 -8D,1. */
 lmi_newops(s,041345);opnd(s,000033,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000035,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,057457);gshift(s,020571);right(s);unary(s,COMP,lmi_comp);next(s);
 /* DAD ABS DP2(-4); SQRT DAD TFF1/4; PUSH BDDV TFFNP. */
 lmi_newops(s,051415);opnd(s,017765,DAD);lmi_dad(s,DP2M4);right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTF_ABORTED;next(s);
 lmi_newops(s,043366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTF_ABORTED;right(s);opnd(s,006512,DAD);lmi_dad(s,k);next(s);
 lmi_newops(s,055206);unary(s,PUSH,lmi_push);right(s);opnd(s,000035,BDDV);lmi_bddv(s,0);next(s);
 /* SR* SR* 0,1 0 -7,1; STODL RPER (pushes up). */
 lmi_newops(s,053657);gshift(s,020601);right(s);gshift(s,020572);next(s);
 stodl(s,014017,041005,0);
 /* DMP BOVB TFF1/ALF TCDANZIG. */
 lmi_newops(s,041005);opnd(s,000027,DMP);lmi_dmp(s,0);right(s);if(lm_time_free_fall_bovb_tcdanzig(s))return LTF_FAILED;next(s);
 /* BZE SL* MAXRA 0 -5,2. */
 lmi_newops(s,053654);lmi_misc(s,057461,BZEGOTO);
 if(lmi_bze(s)){TRACE(s);return LTF_MAXRA;}
 right(s);gshift(s,057603);next(s);
 /* SL* BOV 0,2 MAXRA. */
 lmi_newops(s,040057);gshift(s,057576);right(s);
 switch(bov(s,057461)){case 0:next(s);break;case 1:return LTF_MAXRA;default:return LTF_FAILED;}
 /* BPL +3. */
 lmi_newops(s,077644);if(bplbmn(s,057463))return LTF_RAPO;
 next(s);return LTF_MAXRA;
}
uint8_t ltf_maxra(apollo_overlay_state *s){
 /* MAXRA DLOAD NEARONE. */
 lmi_newops(s,077745);opnd(s,017771,DLOAD);lmi_dload_op(s,NEARONE);next(s);
 return LTF_RAPO;
}
uint8_t ltf_rapo(apollo_overlay_state *s){
 /* +3 STORE RAPO; DUMPRPRA RVQ. */
 store(s,000021);rvq(s);return LTF_RETURNED;
}
uint8_t ltf_calctper(apollo_overlay_state *s,p66_bias_io *io){
 /* CALCTPER SETGO TFFSW +3. */
 lmi_newops(s,077614);lmi_misc(s,003436,SWITCH);
 if(!lmi_switch(s,io))return LTF_FAILED;
 lmi_swbranch(s,057472);TRACE(s);return LTF_TERMS;
}
uint8_t ltf_calctff(apollo_overlay_state *s,p66_bias_io *io){
 /* CALCTFF CLEAR TFFSW. */
 lmi_newops(s,077614);lmi_misc(s,003676,SWITCH);
 if(lmi_switch(s,io))return LTF_FAILED;
 next(s);return LTF_TERMS;
}
uint8_t ltf_terms(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[2];
 if(!bank23(0511,k))return LTF_FAILED;
 /* +3 STORE RTERM; SL* 0,1; STORE NRTERM. */
 store(s,000023);
 lmi_newops(s,077657);gshift(s,020201);next(s);
 store(s,000021);
 /* DMP BDSU TFFALFA TFF1/4; PUSH DMP NRTERM; PDDL SR* TFFNP 0 -6,1. */
 lmi_newops(s,044205);opnd(s,000033,DMP);lmi_dmp(s,0);right(s);opnd(s,006512,BDSU);lmi_bdsu(s,k);next(s);
 lmi_newops(s,041206);unary(s,PUSH,lmi_push);right(s);opnd(s,000021,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,053725);opnd(s,000035,PDDL);lmi_pddl_op(s,0);right(s);gshift(s,020573);next(s);
 /* DCOMP DAD (pushes up); SR* 0,1. */
 lmi_newops(s,043276);unary(s,COMP,lmi_comp);right(s);opnd(s,077657,DAD);lmi_dad(s,0);next(s);
 lmi_newops(s,077657);gshift(s,020601);next(s);
 /* BOFF DLOAD TFFSW +2 TFFZEROS. */
 lmi_newops(s,071214);lmi_misc(s,003756,SWITCH);
 if(lmi_switch(s,io)){lmi_swbranch(s,057515);TRACE(s);return LTF_QTERM;}
 if(!bank23(0521,k))return LTF_FAILED;
 right(s);opnd(s,006522,DLOAD);lmi_dload_op(s,k);next(s);
 return LTF_QTERM;
}
uint8_t ltf_qterm(apollo_overlay_state *s){
 /* +2 BMN SQRT MAXTFF1; DCOMP BOVB TCDANZIG. */
 lmi_newops(s,075440);if(bplbmn(s,057650))return LTF_MAXTFF1;
 right(s);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTF_ABORTED;next(s);
 lmi_newops(s,041076);unary(s,COMP,lmi_comp);right(s);if(lm_time_free_fall_bovb_tcdanzig(s))return LTF_FAILED;next(s);
 /* STOVL TFFTEM VONE'; DOT SL3 RONE; STORE TFFQ1. */
 stovl(s,024045,002170);
 lmi_newops(s,052441);opnd(s,002207,DOT);lmi_dot(s,0);right(s);shorts(s);next(s);
 store(s,000017);
 return LTF_XTERM;
}
uint8_t ltf_xterm(apollo_overlay_state *s){
 /* BMN BDSU INBOUND TFFTEM; STODL TFFX TFFALFA; DMP BDSU NRMAG (pushes up). */
 lmi_newops(s,044240);if(bplbmn(s,057550))return LTF_INBOUND;
 right(s);opnd(s,000045,BDSU);lmi_bdsu(s,0);next(s);
 stodl(s,014043,000033,0);
 lmi_newops(s,044205);opnd(s,000041,DMP);lmi_dmp(s,0);right(s);opnd(s,051406,BDSU);lmi_bdsu(s,0);next(s);
 return LTF_SAVEDEN;
}
uint8_t ltf_saveden(apollo_overlay_state *s){
 /* SAVEDEN PUSH ABS; DAD BOV LIM(-22) TFFXTEST. */
 lmi_newops(s,051406);unary(s,PUSH,lmi_push);right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTF_ABORTED;next(s);
 lmi_newops(s,040015);opnd(s,017757,DAD);lmi_dad(s,LIM22);right(s);
 switch(bov(s,057561)){case 0:next(s);return LTF_FALLBACK;case 1:return LTF_XTEST;default:return LTF_FAILED;}
}
uint8_t ltf_fallback(apollo_overlay_state *s){
 agc_word k[2];
 if(!bank23(0521,k))return LTF_FAILED;
 /* DLOAD PDDL TFFZEROS (PDDL pushes up); DLOAD DCOMP TFFALFA. */
 lmi_newops(s,065345);opnd(s,006522,DLOAD);lmi_dload_op(s,k);right(s);opnd(s,057545,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,057545);opnd(s,000033,DLOAD);lmi_dload_op(s,0);right(s);unary(s,COMP,lmi_comp);next(s);
 /* BMN DLOAD TFFEL1 (DLOAD pushes up); DUMPTFF1 RVQ. */
 lmi_newops(s,071240);if(bplbmn(s,057655))return LTF_TFFEL1;
 right(s);opnd(s,077616,DLOAD);lmi_dload_op(s,0);next(s);
 rvq(s);return LTF_RETURNED;
}
uint8_t ltf_inbound(apollo_overlay_state *s){
 /* INBOUND DLOAD (pushes up); DLOAD DSU RTERM RMAG1; STODL TFFX TFFTEM. */
 lmi_newops(s,077745);opnd(s,045345,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,045345);opnd(s,000023,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000015,DSU);lmi_dsu(s,0);next(s);
 stodl(s,014043,000045,0);
 /* DAD GOTO TFFQ1 SAVEDEN. */
 lmi_newops(s,052015);opnd(s,000017,DAD);lmi_dad(s,0);right(s);lmi_misc(s,057535,BZEGOTO);lmi_goto_order(s);TRACE(s);
 return LTF_SAVEDEN;
}
uint8_t ltf_xtest(apollo_overlay_state *s){
 /* TFFXTEST DAD PDDL DP(-22) TFFX; DMP SR* TFFRTALF 0 -3,2; DDV (pushes up). */
 lmi_newops(s,065215);opnd(s,017761,DAD);lmi_dad(s,DP22);right(s);opnd(s,000043,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,053605);opnd(s,000031,DMP);lmi_dmp(s,0);right(s);gshift(s,057201);next(s);
 lmi_newops(s,077671);opnd(s,040145,DDV);lmi_ddv(s,0);next(s);
 /* DLOAD BOV TFFX TFFELL. */
 lmi_newops(s,040145);opnd(s,000043,DLOAD);lmi_dload_op(s,0);right(s);
 switch(bov(s,057653)){case 0:next(s);return LTF_SERIES;case 1:return LTF_TFFELL;default:return LTF_FAILED;}
}
uint8_t ltf_series(apollo_overlay_state *s){
 /* DDV STADR (DDV pushes up) STORE TFFTEM; PUSH DSQ; PUSH DMP TFFNP. */
 lmi_newops(s,045471);opnd(s,077732,DDV);lmi_ddv(s,0);right(s);lmi_unary(s,STADR);lmi_stadr(s,077732);next(s);
 lmi_newops(s,063406);unary(s,PUSH,lmi_push);right(s);unary(s,DSQ,lmi_dsq);next(s);
 lmi_newops(s,041206);unary(s,PUSH,lmi_push);right(s);opnd(s,000035,DMP);lmi_dmp(s,0);next(s);
 /* SL SIGN 5 TFFTEM; STODL TFFTEM (pushes up). */
 lmi_newops(s,075261);opnd(s,020206,GSHIFT);lmi_gshift(s);right(s);opnd(s,000045,SIGN);lmi_sign(s,0);next(s);
 stodl(s,014045,041206,0);
 /* PUSH DMP TFFALFA; SL* 0,1; STORE TFFX. */
 lmi_newops(s,041206);unary(s,PUSH,lmi_push);right(s);opnd(s,000033,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,077657);gshift(s,020201);next(s);
 store(s,000043);
 return LTF_POLY;
}
uint8_t ltf_poly(apollo_overlay_state *s){
 /* RTB DMP T(X) (DMP pushes up); SR2 BDSU RTERM; DAD DMP RMAG1 (DMP pushes up). */
 lmi_newops(s,041234);if(!rtb_tx(s))return LTF_FAILED;right(s);opnd(s,044302,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,044302);shorts(s);right(s);opnd(s,000023,BDSU);lmi_bdsu(s,0);next(s);
 lmi_newops(s,041215);opnd(s,000015,DAD);lmi_dad(s,0);right(s);opnd(s,051042,DMP);lmi_dmp(s,0);next(s);
 /* SR3 BPL ENDTFF; PUSH SIGN TFFQ1. */
 lmi_newops(s,051042);shorts(s);right(s);if(bplbmn(s,057641))return LTF_ENDTFF;next(s);
 lmi_newops(s,075206);unary(s,PUSH,lmi_push);right(s);opnd(s,000017,SIGN);lmi_sign(s,0);next(s);
 /* BPL DLOAD NEGTFF TFF1/ALF; DCOMP BPL NEGTFF; DCOMP. */
 lmi_newops(s,071244);if(bplbmn(s,057645))return LTF_NEGTFF;
 right(s);opnd(s,000027,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,051076);unary(s,COMP,lmi_comp);right(s);if(bplbmn(s,057645))return LTF_NEGTFF;next(s);
 lmi_newops(s,077676);unary(s,COMP,lmi_comp);next(s);
 return LTF_SCALE;
}
uint8_t ltf_scale(apollo_overlay_state *s){
 /* DMP DDV PI/16 TFFRTALF; SL* SL* 0 -4,2 0 -4,2; SL* DAD 0,2 (DAD pushes up). */
 lmi_newops(s,056205);opnd(s,017755,DMP);lmi_dmp(s,PI16);right(s);opnd(s,000031,DDV);lmi_ddv(s,0);next(s);
 lmi_newops(s,053657);gshift(s,057602);right(s);gshift(s,057602);next(s);
 lmi_newops(s,043257);gshift(s,057576);right(s);opnd(s,040005,DAD);lmi_dad(s,0);next(s);
 return LTF_ENDTFF;
}
uint8_t ltf_endtff(apollo_overlay_state *s){
 /* ENDTFF DMP BOV TFF/RTMU MAXTFF; DUMPTFF2 RVQ. */
 lmi_newops(s,040005);opnd(s,000037,DMP);lmi_dmp(s,0);right(s);
 switch(bov(s,057651)){case 0:next(s);break;case 1:return LTF_MAXTFF;default:return LTF_FAILED;}
 rvq(s);return LTF_RETURNED;
}
uint8_t ltf_negtff(apollo_overlay_state *s){
 /* NEGTFF DLOAD (pushes up); GOTO ENDTFF. */
 lmi_newops(s,077745);opnd(s,077650,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,077650);lmi_misc(s,057641,BZEGOTO);lmi_goto_order(s);TRACE(s);
 return LTF_ENDTFF;
}
uint8_t ltf_maxtff1(apollo_overlay_state *s){
 /* MAXTFF1 DLOAD (pushes up). */
 lmi_newops(s,077745);opnd(s,043545,DLOAD);lmi_dload_op(s,0);next(s);
 return LTF_MAXTFF;
}
uint8_t ltf_maxtff(apollo_overlay_state *s){
 /* MAXTFF DLOAD RVQ NEARONE. */
 lmi_newops(s,043545);opnd(s,017771,DLOAD);lmi_dload_op(s,NEARONE);right(s);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);
 return LTF_RETURNED;
}
uint8_t ltf_tffell(apollo_overlay_state *s){
 /* TFFELL SL2; BDDV PUSH (BDDV pushes up). */
 lmi_newops(s,077712);shorts(s);next(s);
 lmi_newops(s,041465);opnd(s,045345,BDDV);lmi_bddv(s,0);right(s);unary(s,PUSH,lmi_push);next(s);
 return LTF_TFFEL1;
}
uint8_t ltf_tffel1(apollo_overlay_state *s,p66_bias_io *io){
 /* TFFEL1 DLOAD DSU TFFTEM TFFQ1; STODL TFFDELQ (pushes up); STADR STORE TFFTEM. */
 lmi_newops(s,045345);opnd(s,000045,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000017,DSU);lmi_dsu(s,0);next(s);
 stodl(s,014013,077626,0);
 lmi_newops(s,077626);lmi_unary(s,STADR);lmi_stadr(s,077732);next(s);
 /* DMP SL* TFF1/ALF 0,2; PUSH DMP TFFTEM. */
 lmi_newops(s,053605);opnd(s,000027,DMP);lmi_dmp(s,0);right(s);gshift(s,057576);next(s);
 lmi_newops(s,041206);unary(s,PUSH,lmi_push);right(s);opnd(s,000045,DMP);lmi_dmp(s,0);next(s);
 /* SL* BOVB 0,2 SIGNMPAC; STORE TFFX. */
 lmi_newops(s,041057);gshift(s,057576);right(s);
 lmi_misc(s,021712,BOVB);
 switch(lmi_bov(s)){
 case LMI_BOV_NONE:break;
 case LMI_BOV_RTB:lmi_rtb(s);if(W[5]!=03712u||rtb_agreement(s,io,RTB_LM,RTB_SIGN_MPAC)!=RTB_OK||W[5]!=06061u)return LTF_FAILED;break;
 default:return LTF_FAILED;
 }
 next(s);
 store(s,000043);
 return LTF_ELLIPSE;
}
uint8_t ltf_ellipse(apollo_overlay_state *s){
 agc_word k[2];
 /* RTB DMP T(X) TFFX; SR3 DSU DP2(-3); DMP PUSH (DMP pushes up). */
 lmi_newops(s,041234);if(!rtb_tx(s))return LTF_FAILED;right(s);opnd(s,000043,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,045242);shorts(s);right(s);opnd(s,017763,DSU);lmi_dsu(s,DP2M3);next(s);
 lmi_newops(s,041405);opnd(s,041345,DMP);lmi_dmp(s,0);right(s);unary(s,PUSH,lmi_push);next(s);
 /* DLOAD DMP TFFTEM RMAG1; SL2 DAD TFFQ1; STODL TFFTEM TFFNP. */
 lmi_newops(s,041345);opnd(s,000045,DLOAD);lmi_dload_op(s,0);right(s);opnd(s,000015,DMP);lmi_dmp(s,0);next(s);
 lmi_newops(s,043312);shorts(s);right(s);opnd(s,000017,DAD);lmi_dad(s,0);next(s);
 stodl(s,014045,000035,0);
 /* DMP SL* TFF1/ALF 1,2; SIGN SL* TFFTEM 0,2; STODL TFFTEM TFF1/ALF. */
 lmi_newops(s,053605);opnd(s,000027,DMP);lmi_dmp(s,0);right(s);gshift(s,057575);next(s);
 lmi_newops(s,053765);opnd(s,000045,SIGN);lmi_sign(s,0);right(s);gshift(s,057576);next(s);
 stodl(s,014045,000027,0);
 /* SQRT DMP PI/16; DAD (pushes up); SL* DSU 0 -1,2 TFFDELQ. */
 lmi_newops(s,041366);lmi_unary(s,SQRT);if(lmi_sqrt(s))return LTF_ABORTED;right(s);opnd(s,017755,DMP);lmi_dmp(s,PI16);next(s);
 lmi_newops(s,077615);opnd(s,045257,DAD);lmi_dad(s,0);next(s);
 lmi_newops(s,045257);gshift(s,057577);right(s);opnd(s,000013,DSU);lmi_dsu(s,0);next(s);
 /* DMP SL* TFF1/ALF 0 -3,2; SL* GOTO 0 -4,2 ENDTFF. */
 lmi_newops(s,053605);opnd(s,000027,DMP);lmi_dmp(s,0);right(s);gshift(s,057601);next(s);
 lmi_newops(s,052057);gshift(s,057602);right(s);lmi_misc(s,057641,BZEGOTO);lmi_goto_order(s);TRACE(s);
 (void)k;return LTF_ENDTFF;
}

uint8_t lm_time_free_fall_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LTF_ENTRY:return ltf_entry(s);
 case LTF_CONMU:return ltf_conmu(s);
 case LTF_CONMU_END:return ltf_conmu_end(s);
 case LTF_RPRA:return ltf_rpra(s);
 case LTF_MAXRA:return ltf_maxra(s);
 case LTF_RAPO:return ltf_rapo(s);
 case LTF_CALCTPER:return ltf_calctper(s,io);
 case LTF_CALCTFF:return ltf_calctff(s,io);
 case LTF_TERMS:return ltf_terms(s,io);
 case LTF_QTERM:return ltf_qterm(s);
 case LTF_XTERM:return ltf_xterm(s);
 case LTF_SAVEDEN:return ltf_saveden(s);
 case LTF_FALLBACK:return ltf_fallback(s);
 case LTF_INBOUND:return ltf_inbound(s);
 case LTF_XTEST:return ltf_xtest(s);
 case LTF_SERIES:return ltf_series(s);
 case LTF_POLY:return ltf_poly(s);
 case LTF_SCALE:return ltf_scale(s);
 case LTF_ENDTFF:return ltf_endtff(s);
 case LTF_NEGTFF:return ltf_negtff(s);
 case LTF_MAXTFF1:return ltf_maxtff1(s);
 case LTF_MAXTFF:return ltf_maxtff(s);
 case LTF_TFFELL:return ltf_tffell(s);
 case LTF_TFFEL1:return ltf_tffel1(s,io);
 case LTF_ELLIPSE:return ltf_ellipse(s);
 default:return LTF_FAILED;
 }
}

/* The actual caller returns (SR30.1, bank 22): after CALL TFFCONMU, CALL
 * TFFRP/RA, DLOAD CALL RPER CALCTPER and DAD CALL RPADTEM CALCTFF. */
/* (TFFCONIC has no Luminary caller: it is admitted with EDOP +0, since
 * EDOP 040000 would send its first DANZIG into CHANG2.) */
static const agc_word returns[4]={045567,045571,045625,045633};
static uint8_t admitted(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120];agc_word q;uint8_t known=0;
 if(W[5]!=06070u||W[4]!=056000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(056000u|(W[3]>>8))||W[0165]!=W[6]||W[0115]!=040000u||W[023]!=(W[0164]==03360u?0u:040000u)||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 q=W[vac+052u];
 for(i=0;i<4u;++i)known|=(uint8_t)(q==returns[i]);
 return known&&(W[0164]==03360u||W[0164]==03361u||W[0164]==03427u||W[0164]==03465u||W[0164]==03470u);
}
uint8_t lm_time_free_fall(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;unsigned guard;
 if(!s||!io||!admitted(s,io))return 0;
 switch(W[0164]){
 case 03360u:phase=LTF_ENTRY;break;
 case 03361u:phase=LTF_CONMU;break;
 case 03427u:phase=LTF_RPRA;break;
 case 03465u:phase=LTF_CALCTPER;break;
 default:phase=LTF_CALCTFF;break;
 }
 /* The routine has no loops; the bound only stops a runaway chain. */
 for(guard=0;guard<64u&&phase<LTF_RETURNED;++guard)phase=lm_time_free_fall_step(s,io,phase);
 return phase==LTF_RETURNED?1u:phase==LTF_ABORTED?2u:0u;
}
