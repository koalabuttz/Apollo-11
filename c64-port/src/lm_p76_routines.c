/* SPDX-License-Identifier: MIT */
/* Luminary099 P76 (P76.agc, bank 13 02207-02350) as routines of the
 * control-transfer ABI; see include/lm_p76_routines.h and docs/LM-P76.md.
 * Every interpretive pair is compiled with the shared bodies
 * (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * those paths, and each runtime branch follows the outcome they return. VXM
 * runs the compiled matrix phases on the work-area matrix at 12D
 * (src/lm_work_matrix.c). Each basic instruction is one ai_* register effect
 * on the image, in source order. */
#include "lm_p76_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "lm_work_matrix.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 COMPMAT (UNIT, VXV,
 * VXM), group 2 the rest. Host builds register all. */
#ifndef LM_P76_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (LM_P76_C64_GROUP==1)
#define HAS_G2 (LM_P76_C64_GROUP==2)
#endif
#if defined __clang__ && defined LM_P76_C64_GROUP
/* A C64 group build compiles only its routines, so helpers that only the
 * other group's routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,VAD=06751,VXV=07460,GSHIFT=07621,SSP=06620,UNIT=03023,COMP=07670,RVQ=03274,
 AXT=02344,CALLITA=02507,SWITCH=02516};
/* Erasable (EB-window address as the source names it, EBANK=TIG). */
#define MARKCTR 01462u
/* Fixed constants (13,2336-2337 and the fixed-fixed ZERO). */
#define K_V06N84 01524u
#define K_V06N33 01441u
#define K_ZERO 0u
/* Destinations outside the routines. */
#define UPFLAG 05504u
#define PHASCHNG 05353u
#define BANKCALL 04616u
#define GOTOPOOH 06001u
#define INTPRET 06037u

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);lmi_dload_op(s,0);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static uint8_t unit(apollo_overlay_state *s){lmi_unary(s,UNIT);return lmi_unit(s);}
static void exit_order(apollo_overlay_state *s){lmi_newops(s,077776);lmi_exit(s);}

/* Canonical image at NEWOPS with LOC `loc` in FB 13, coherent banks with
 * BANKSET = BB, INTBIT15 clear (bank 13 is below 20), EDOP, ZERO and NEWJOB
 * zero, the VAC one of the five with PUSHLOC no further than 32D into it,
 * MODE one of the three, IRQ 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=026000u||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS, at POODOO1, or after EXIT at
 * the basic word that follows: the trampoline continues there or stops
 * outside the registered routines. */

#if HAS_G2
/* VLOAD DELVLVC; STORE DELVOV; EXIT. */
INTERP(p76_save,02212u)
 lmi_newops(s,077775);vload(s,003434);next(s);
 store(s,002223);
 exit_order(s);
 return EXEC_OK;
}
/* DLOAD TIG; STCALL TDEC1 OTHPREC. */
INTERP(p76_tig,02233u)
 lmi_newops(s,077745);opnd(s,003442,DLOAD);lmi_dload_op(s,0);next(s);
 stcall(s,034041,027043);
 return EXEC_OK;
}
#endif
#if HAS_G1
/* COMPMAT VLOAD UNIT RATT; VCOMP; STORE 24D; VXV UNIT VATT; STORE 18D; VXV
 * UNIT 24D; STOVL 12D DELVOV; VXM VSL1 12D; VAD VATT; STORE 6; CALL
 * INTSTALL (where OTHPREC returns). */
INTERP(compmat,02237u)
 lmi_newops(s,053575);vload(s,000001);right(s);if(unit(s))return EXEC_OK;next(s);
 lmi_newops(s,077676);lmi_unary(s,COMP);lmi_comp(s);next(s);
 store(s,000031);
 lmi_newops(s,053435);opnd(s,000007,VXV);lmi_vxv(s,0);right(s);if(unit(s))return EXEC_OK;next(s);
 store(s,000023);
 lmi_newops(s,053435);opnd(s,000031,VXV);lmi_vxv(s,0);right(s);if(unit(s))return EXEC_OK;next(s);
 stovl(s,024015,002223);
 lmi_newops(s,076505);if(!lm_work_matrix(s,000015,1))return EXEC_INVALID_ACTION;right(s);shorts(s);next(s);
 lmi_newops(s,077655);opnd(s,000007,VAD);lmi_vad(s,0);next(s);
 store(s,000007);
 lmi_newops(s,077624);call(s,027414);
 return EXEC_OK;
}
#endif
#if HAS_G2
/* CALL P76SUB1 (where INTSTALL returns). */
INTERP(call_sub1,02261u)
 lmi_newops(s,077624);call(s,026340);
 return EXEC_OK;
}
/* VLOAD VSR* 6 0,2; STOVL VCV RATT; VSR* 0,2; STODL RCV TIG; STORE TET;
 * CLEAR DLOAD INTYPFLG TETTHIS; INTOTHIS STCALL TDEC1 INTEGRVS (where
 * P76SUB1 returns). */
INTERP(sub1_ret,02263u)
 lmi_newops(s,053775);vload(s,000007);right(s);gshift(s,057176);next(s);
 stovl(s,025543,000001);
 lmi_newops(s,077657);gshift(s,057176);next(s);
 stodl(s,015535,003442);
 store(s,001517);
 lmi_newops(s,071214);if(sw(s,io,001673))return EXEC_INVALID_ACTION;right(s);opnd(s,001643,DLOAD);lmi_dload_op(s,0);next(s);
 stcall(s,034041,027107);
 return EXEC_OK;
}
/* CALL INTSTALL (where INTEGRVS returns). */
INTERP(integrvs_ret,02302u)
 lmi_newops(s,077624);call(s,027414);
 return EXEC_OK;
}
/* VLOAD RATT1; STORE RRECT; STODL RCV TAT; STOVL TET VATT1; CALL MINIRECT
 * (where INTSTALL returns again). */
INTERP(intstall_ret,02304u)
 lmi_newops(s,077775);vload(s,000017);next(s);
 store(s,001503);
 stodl(s,015535,000015);
 stovl(s,025517,000025);
 lmi_newops(s,077624);call(s,023455);
 return EXEC_OK;
}
/* EXIT (where MINIRECT returns). */
INTERP(minirect_ret,02315u)
 exit_order(s);
 return EXEC_OK;
}
/* CALL ATOPOTH (after REINTFLG's INTPRET). */
INTERP(call_atopoth,02323u)
 lmi_newops(s,077624);call(s,026661);
 return EXEC_OK;
}
/* SSP EXIT QPRET OUT (where ATOPOTH returns). */
INTERP(atopoth_ret,02325u)
 lmi_newops(s,077531);lmi_direct(s,000053,SSP);lmi_ssp(s,026332);right(s);lmi_exit(s);
 return EXEC_OK;
}
/* OUT EXIT. */
INTERP(out,02332u)
 exit_order(s);
 return EXEC_OK;
}
/* P76SUB1 AXT,2 SET 2 MOONFLAG; BON AXT,2 CMOONFLG QPRET 0; CLEAR RVQ
 * MOONFLAG. BON's branch names erasable QPRET, so SWBRANCH's GOTO goes
 * through GOTOERS to the CADR there. */
INTERP(p76sub1,02340u)
 lmi_newops(s,043174);lmi_misc(s,000002,AXT);lmi_axt(s);right(s);if(sw(s,io,000063))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,077014);
 if(sw(s,io,004303)){ai_ca(s,0165u);(void)ai_ts(s,4u);ai_caf(s,000052);(void)ai_ts(s,0117u);lmi_goto_any(s);return EXEC_OK;}
 right(s);lmi_misc(s,000000,AXT);lmi_axt(s);next(s);
 lmi_newops(s,043414);if(sw(s,io,000263))return EXEC_INVALID_ACTION;right(s);lmi_unary(s,RVQ);lmi_rvq(s);
 return EXEC_OK;
}
#endif
static const agc_routine_entry interpretive[]={
#if HAS_G2
 {LM_P76_B13(02212),p76_save},{LM_P76_B13(02233),p76_tig},
#endif
#if HAS_G1
 {LM_P76_B13(02237),compmat},
#endif
#if HAS_G2
 {LM_P76_B13(02261),call_sub1},{LM_P76_B13(02263),sub1_ret},{LM_P76_B13(02302),integrvs_ret},
 {LM_P76_B13(02304),intstall_ret},{LM_P76_B13(02315),minirect_ret},{LM_P76_B13(02323),call_atopoth},
 {LM_P76_B13(02325),atopoth_ret},{LM_P76_B13(02332),out},{LM_P76_B13(02340),p76sub1},
#endif
};
const agc_routine_table lm_p76_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

#if HAS_G2
static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 13 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=026000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* CAF V06N84; TC BANKCALL (CADR GOFLASH), and V06N84 +1's. */
static void flash84(apollo_overlay_state *s){ai_caf(s,K_V06N84);tc(s,02220u,BANKCALL);}
static void flash33(apollo_overlay_state *s){ai_caf(s,K_V06N33);tc(s,02226u,BANKCALL);}
/* ENDP76 CAF ZERO; TS MARKCTR; TCF GOTOPOOH. */
static void endp76(apollo_overlay_state *s){ai_caf(s,K_ZERO);(void)ai_ts(s,MARKCTR);W[5]=GOTOPOOH;}
ROUTINE(p76){ENTRY(02207u);tc(s,02210u,UPFLAG);return EXEC_OK;}
ROUTINE(trackflg_ret){ENTRY(02211u);tc(s,02212u,INTPRET);return EXEC_OK;}
ROUTINE(r_flash84){ENTRY(02216u);flash84(s);return EXEC_OK;}
ROUTINE(f84_terminate){ENTRY(02221u);W[5]=02333u;endp76(s);return EXEC_OK;}
ROUTINE(f84_proceed){ENTRY(02222u);tc(s,02223u,02224u);flash33(s);return EXEC_OK;}
ROUTINE(f84_recycle){ENTRY(02223u);tc(s,02224u,02216u);flash84(s);return EXEC_OK;}
ROUTINE(r_flash33){ENTRY(02224u);flash33(s);return EXEC_OK;}
ROUTINE(f33_terminate){ENTRY(02227u);W[5]=02333u;endp76(s);return EXEC_OK;}
ROUTINE(f33_proceed){ENTRY(02230u);tc(s,02231u,02232u);tc(s,02233u,INTPRET);return EXEC_OK;}
ROUTINE(f33_recycle){ENTRY(02231u);tc(s,02232u,02224u);flash33(s);return EXEC_OK;}
ROUTINE(r_intpret){ENTRY(02232u);tc(s,02233u,INTPRET);return EXEC_OK;}
/* After MINIRECT's EXIT: TC PHASCHNG (OCT 04024); TC UPFLAG (REINTFLG); TC
 * INTPRET. */
ROUTINE(phaschng){ENTRY(02316u);tc(s,02317u,PHASCHNG);return EXEC_OK;}
ROUTINE(phaschng_ret){ENTRY(02320u);tc(s,02321u,UPFLAG);return EXEC_OK;}
ROUTINE(reintflg_ret){ENTRY(02322u);tc(s,02323u,INTPRET);return EXEC_OK;}
/* After ATOPOTH's EXIT: TC BANKCALL (CADR INTWAKE1). */
ROUTINE(intwake){ENTRY(02330u);tc(s,02331u,BANKCALL);return EXEC_OK;}
ROUTINE(r_endp76){ENTRY(02333u);endp76(s);return EXEC_OK;}
#endif
static const agc_routine_entry basic[]={
#if HAS_G2
 {LM_P76_B13(02207),p76},{LM_P76_B13(02211),trackflg_ret},{LM_P76_B13(02216),r_flash84},{LM_P76_B13(02221),f84_terminate},
 {LM_P76_B13(02222),f84_proceed},{LM_P76_B13(02223),f84_recycle},{LM_P76_B13(02224),r_flash33},{LM_P76_B13(02227),f33_terminate},
 {LM_P76_B13(02230),f33_proceed},{LM_P76_B13(02231),f33_recycle},{LM_P76_B13(02232),r_intpret},{LM_P76_B13(02316),phaschng},
 {LM_P76_B13(02320),phaschng_ret},{LM_P76_B13(02322),reintflg_ret},{LM_P76_B13(02330),intwake},{LM_P76_B13(02333),r_endp76},
#else
 {0,0}
#endif
};
const agc_routine_table lm_p76_basic_routines={basic,(uint16_t)(HAS_G2?sizeof basic/sizeof *basic:0u)};
