/* SPDX-License-Identifier: MIT */
/* Comanche055 P76 (P76.agc, bank 13 02071-02230) as routines of the
 * control-transfer ABI; see include/cm_p76_routines.h and docs/CM-P76.md.
 * Every interpretive pair is compiled with the shared bodies built for
 * Comanche055 (src/cm_interp_steps.c, src/cm_interp_orders.c): its original
 * order and operand words appear below only as the literal bookkeeping
 * arguments of those paths, and each runtime branch follows the outcome they
 * return. VXM reads its matrix from the work area at 12D. V06N84, V06N33,
 * ZERO and NEGONE are literals that the source map checks against the fresh
 * rope. Each basic instruction is one ai_* register effect on the image, in
 * source order. */
#include "cm_p76_routines.h"
#include "cm_interp.h"
#include "agc_image_ops.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 COMPMAT (UNIT, VXV,
 * VXM), group 2 the rest. Host builds register all. */
#ifndef CM_P76_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (CM_P76_C64_GROUP==1)
#define HAS_G2 (CM_P76_C64_GROUP==2)
#endif
#if defined __clang__ && defined CM_P76_C64_GROUP
/* A C64 group build compiles only its routines, so helpers that only the
 * other group's routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries (Comanche055). */
enum {VLOAD=06454,DLOAD=06021,SLOAD=06450,VAD=06720,VXV=07427,VXM=07306,GSHIFT=07570,SSP=06567,UNIT=03023,COMP=07637,
 RVQ=03274,CALLITA=02534,RTBBHIZ=02474,SWITCH=02543};
/* Erasable (unswitched, and the EB-window address as the source names it,
 * EBANK=TIG). */
#define MARKCTR 01126u
#define VHFCNT 01125u
#define MRKBUF2 01734u
/* Fixed constants (13,2220-2221 and the fixed-fixed ZERO and NEGONE). */
#define K_V06N84 01524u
#define K_V06N33 01441u
#define K_ZERO 0u
#define K_NEGONE 077776u
/* Destinations outside the routines. */
#define UPFLAG 05546u
#define PHASCHNG 05412u
#define BANKCALL 04662u
#define GOTOPOOH 04106u
#define INTPRET 06006u
#define FB13 026000u

static void next(apollo_overlay_state *s){cmi_danzig_next(s);}
static void right(apollo_overlay_state *s){cmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)cmi_address_pushup(s,word,target);else cmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){cmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);cmi_vload(s,0);}
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 cmi_store_code(s,code);cmi_dodload(s);cmi_diradres(s,word,DLOAD);cmi_dload_op(s,0);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 cmi_store_code(s,code);cmi_dovload(s);cmi_diradres(s,word,VLOAD);cmi_vload(s,0);next(s);
}
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){cmi_store_code(s,code);cmi_15bitadr(s,word,CALLITA);cmi_call(s);cmi_goto(s);}
static void call(apollo_overlay_state *s,agc_word word){cmi_misc(s,word,CALLITA);cmi_call(s);cmi_goto(s);}
static void gshift(apollo_overlay_state *s,agc_word word){cmi_indexed(s,word,GSHIFT);cmi_gshift(s);}
static void shorts(apollo_overlay_state *s){cmi_short_dispatch(s);if(W[5]==02017u)cmi_shortt(s);else cmi_shortv(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){cmi_misc(s,word,SWITCH);return cmi_switch(s,io);}
static uint8_t unit(apollo_overlay_state *s){cmi_unary(s,UNIT);return cmi_unit(s);}
static void exit_order(apollo_overlay_state *s){cmi_newops(s,077776);cmi_exit(s);}
/* VXM of the 18-word matrix the address formation names in the work area:
 * 0 when it would run past the erasable image. */
static uint8_t vxm(apollo_overlay_state *s,agc_word word){
 agc_word k[18];unsigned i;
 opnd(s,word,VXM);if(W[0116]>2048u-18u)return 0;
 for(i=0;i<18u;++i)k[i]=W[W[0116]+i];
 cmi_vxm(s,k);return 1;
}

/* Canonical image at NEWOPS with LOC `loc` in FB 13, coherent banks with
 * BANKSET = BB, INTBIT15 clear (bank 13 is below 20), EDOP, ZERO and NEWJOB
 * zero, the VAC one of the five with PUSHLOC no further than 32D into it,
 * MODE one of the three, IRQ 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06037u||W[4]!=FB13||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS, at POODOO, or after EXIT at
 * the basic word that follows: the trampoline continues there or stops
 * outside the registered routines. */

#if HAS_G2
/* DLOAD TIG; STCALL TDEC1 OTHPREC (after the INTPRET at 2107). */
INTERP(p76_tig,02110u)
 cmi_newops(s,077745);opnd(s,003413,DLOAD);cmi_dload_op(s,0);next(s);
 stcall(s,034041,027105);
 return EXEC_OK;
}
#endif
#if HAS_G1
/* COMPMAT VLOAD UNIT RATT; VCOMP; STORE 24D; VXV UNIT VATT; STORE 18D; VXV
 * UNIT 24D; STOVL 12D DELVOV; VXM VSL1 12D; VAD VATT; STORE 6; CALL
 * INTSTALL (where OTHPREC returns). */
INTERP(compmat,02114u)
 cmi_newops(s,053575);vload(s,000001);right(s);if(unit(s))return EXEC_OK;next(s);
 cmi_newops(s,077676);cmi_unary(s,COMP);cmi_comp(s);next(s);
 store(s,000031);
 cmi_newops(s,053435);opnd(s,000007,VXV);cmi_vxv(s,0);right(s);if(unit(s))return EXEC_OK;next(s);
 store(s,000023);
 cmi_newops(s,053435);opnd(s,000031,VXV);cmi_vxv(s,0);right(s);if(unit(s))return EXEC_OK;next(s);
 stovl(s,024015,003540);
 cmi_newops(s,076505);if(!vxm(s,000015))return EXEC_INVALID_ACTION;right(s);shorts(s);next(s);
 cmi_newops(s,077655);opnd(s,000007,VAD);cmi_vad(s,0);next(s);
 store(s,000007);
 cmi_newops(s,077624);call(s,027442);
 return EXEC_OK;
}
#endif
#if HAS_G2
/* CALL P76SUB1 (where INTSTALL returns). */
INTERP(call_sub1,02136u)
 cmi_newops(s,077624);call(s,026222);
 return EXEC_OK;
}
/* VLOAD VSR* 6 0,2; STOVL VCV RATT; VSR* 0,2; STODL RCV TIG; STORE TET;
 * CLEAR DLOAD INTYPFLG TETTHIS; INTOTHIS STCALL TDEC1 INTEGRVS (where
 * P76SUB1 returns). */
INTERP(sub1_ret,02140u)
 cmi_newops(s,053775);vload(s,000007);right(s);gshift(s,057176);next(s);
 stovl(s,025543,000001);
 cmi_newops(s,077657);gshift(s,057176);next(s);
 stodl(s,015535,003413);
 store(s,001517);
 cmi_newops(s,071214);if(sw(s,io,001673))return EXEC_INVALID_ACTION;right(s);opnd(s,001571,DLOAD);cmi_dload_op(s,0);next(s);
 stcall(s,034041,027135);
 return EXEC_OK;
}
/* CALL INTSTALL (where INTEGRVS returns). */
INTERP(integrvs_ret,02157u)
 cmi_newops(s,077624);call(s,027442);
 return EXEC_OK;
}
/* CALL P76SUB1 (where INTSTALL returns again). */
INTERP(call_sub1_2,02161u)
 cmi_newops(s,077624);call(s,026222);
 return EXEC_OK;
}
/* VLOAD RATT1; STORE RRECT; STODL RCV TAT; STOVL TET VATT1; CALL MINIRECT
 * (where P76SUB1 returns again). */
INTERP(sub1_ret2,02163u)
 cmi_newops(s,077775);vload(s,000017);next(s);
 store(s,001503);
 stodl(s,015535,000015);
 stovl(s,025517,000025);
 cmi_newops(s,077624);call(s,023435);
 return EXEC_OK;
}
/* EXIT (where MINIRECT returns). */
INTERP(minirect_ret,02174u)
 exit_order(s);
 return EXEC_OK;
}
/* CALL ATOPOTH (after REINTFLG's INTPRET). */
INTERP(call_atopoth,02202u)
 cmi_newops(s,077624);call(s,026760);
 return EXEC_OK;
}
/* SSP EXIT QPRET OUT (where ATOPOTH returns). */
INTERP(atopoth_ret,02204u)
 cmi_newops(s,077531);cmi_direct(s,000053,SSP);cmi_ssp(s,026211);right(s);cmi_exit(s);
 return EXEC_OK;
}
/* OUT EXIT. */
INTERP(out,02211u)
 exit_order(s);
 return EXEC_OK;
}
/* P76SUB1 CLEAR SLOAD MOONFLAG X2; BHIZ SET +2 MOONFLAG, MOONFLAG set
 * unless X2 is zero; RVQ. */
INTERP(p76sub1,02222u)
 cmi_newops(s,067214);if(sw(s,io,000263))return EXEC_INVALID_ACTION;right(s);cmi_direct(s,000050,SLOAD);cmi_sload(s);next(s);
 cmi_newops(s,043030);cmi_misc(s,026230,RTBBHIZ);(void)ai_ccs(s,020);
 if(cmi_bhiz(s))cmi_goto(s);
 else{right(s);if(sw(s,io,000063))return EXEC_INVALID_ACTION;next(s);}
 cmi_newops(s,077616);cmi_unary(s,RVQ);cmi_rvq(s);
 return EXEC_OK;
}
#endif
static const agc_routine_entry interpretive[]={
#if HAS_G2
 {CM_P76_B13(02110),p76_tig},
#endif
#if HAS_G1
 {CM_P76_B13(02114),compmat},
#endif
#if HAS_G2
 {CM_P76_B13(02136),call_sub1},{CM_P76_B13(02140),sub1_ret},
 {CM_P76_B13(02157),integrvs_ret},{CM_P76_B13(02161),call_sub1_2},{CM_P76_B13(02163),sub1_ret2},{CM_P76_B13(02174),minirect_ret},
 {CM_P76_B13(02202),call_atopoth},{CM_P76_B13(02204),atopoth_ret},{CM_P76_B13(02211),out},{CM_P76_B13(02222),p76sub1},
#endif
};
const agc_routine_table cm_p76_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

#if HAS_G2
static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 13 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=FB13||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* CAF V06N84; TC BANKCALL (CADR GOFLASH), and V06N84 +1's. */
static void flash84(apollo_overlay_state *s){ai_caf(s,K_V06N84);tc(s,02075u,BANKCALL);}
static void flash33(apollo_overlay_state *s){ai_caf(s,K_V06N33);tc(s,02103u,BANKCALL);}
/* ENDP76 CAF ZERO; TS MARKCTR; TS VHFCNT; CAF NEGONE; TS MRKBUF2; TCF
 * GOTOPOOH. */
static void endp76(apollo_overlay_state *s){
 ai_caf(s,K_ZERO);(void)ai_ts(s,MARKCTR);(void)ai_ts(s,VHFCNT);ai_caf(s,K_NEGONE);(void)ai_ts(s,MRKBUF2);W[5]=GOTOPOOH;
}
/* P76 TC UPFLAG (ADRES TRACKFLG). */
ROUTINE(p76){ENTRY(02071u);tc(s,02072u,UPFLAG);return EXEC_OK;}
/* (where UPFLAG returns) CAF V06N84; TC BANKCALL; its returns TCF ENDP76,
 * TC +2 (on to CAF V06N84 +1; TC BANKCALL), TC -5. */
ROUTINE(r_flash84){ENTRY(02073u);flash84(s);return EXEC_OK;}
ROUTINE(f84_terminate){ENTRY(02076u);W[5]=02212u;endp76(s);return EXEC_OK;}
ROUTINE(f84_proceed){ENTRY(02077u);tc(s,02100u,02101u);flash33(s);return EXEC_OK;}
ROUTINE(f84_recycle){ENTRY(02100u);tc(s,02101u,02073u);flash84(s);return EXEC_OK;}
ROUTINE(r_flash33){ENTRY(02101u);flash33(s);return EXEC_OK;}
/* GOFLASH's returns: TCF ENDP76, TC +2 (on to TC INTPRET), TC -5. */
ROUTINE(f33_terminate){ENTRY(02104u);W[5]=02212u;endp76(s);return EXEC_OK;}
ROUTINE(f33_proceed){ENTRY(02105u);tc(s,02106u,02107u);tc(s,02110u,INTPRET);return EXEC_OK;}
ROUTINE(f33_recycle){ENTRY(02106u);tc(s,02107u,02101u);flash33(s);return EXEC_OK;}
ROUTINE(r_intpret){ENTRY(02107u);tc(s,02110u,INTPRET);return EXEC_OK;}
/* After MINIRECT's EXIT: TC PHASCHNG (OCT 04024); TC UPFLAG (REINTFLG); TC
 * INTPRET. */
ROUTINE(phaschng){ENTRY(02175u);tc(s,02176u,PHASCHNG);return EXEC_OK;}
ROUTINE(phaschng_ret){ENTRY(02177u);tc(s,02200u,UPFLAG);return EXEC_OK;}
ROUTINE(reintflg_ret){ENTRY(02201u);tc(s,02202u,INTPRET);return EXEC_OK;}
/* After ATOPOTH's EXIT: TC BANKCALL (CADR INTWAKE1). */
ROUTINE(intwake){ENTRY(02207u);tc(s,02210u,BANKCALL);return EXEC_OK;}
ROUTINE(r_endp76){ENTRY(02212u);endp76(s);return EXEC_OK;}
#endif
static const agc_routine_entry basic[]={
#if HAS_G2
 {CM_P76_B13(02071),p76},{CM_P76_B13(02073),r_flash84},{CM_P76_B13(02076),f84_terminate},{CM_P76_B13(02077),f84_proceed},
 {CM_P76_B13(02100),f84_recycle},{CM_P76_B13(02101),r_flash33},{CM_P76_B13(02104),f33_terminate},{CM_P76_B13(02105),f33_proceed},
 {CM_P76_B13(02106),f33_recycle},{CM_P76_B13(02107),r_intpret},{CM_P76_B13(02175),phaschng},{CM_P76_B13(02177),phaschng_ret},
 {CM_P76_B13(02201),reintflg_ret},{CM_P76_B13(02207),intwake},{CM_P76_B13(02212),r_endp76},
#else
 {0,0}
#endif
};
const agc_routine_table cm_p76_basic_routines={basic,(uint16_t)(HAS_G2?sizeof basic/sizeof *basic:0u)};
