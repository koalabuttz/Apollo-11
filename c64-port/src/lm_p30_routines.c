/* SPDX-License-Identifier: MIT */
/* Luminary099 P30 and S30.1 (P30_P37.agc, bank 35 02000-02026 and bank 34
 * 02000-02056) as routines of the control-transfer ABI; see
 * include/lm_p30_routines.h and docs/LM-P30.md. Every interpretive pair is
 * compiled with the shared bodies (src/lm_interp_steps.c,
 * src/lm_interp_orders.c): its original order and operand words appear below
 * only as the literal bookkeeping arguments of those paths, and each runtime
 * branch follows the outcome they return. VXM runs the compiled matrix
 * phases on the work-area matrix at 0D (src/lm_work_matrix.c). Each basic
 * instruction is one ai_* register effect on the image, in source order. */
#include "lm_p30_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "lm_work_matrix.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 S30.1's LEMPREC
 * return (UNIT, VXV, VXM, ABVAL), group 2 the rest. Host builds register
 * all. */
#ifndef LM_P30_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (LM_P30_C64_GROUP==1)
#define HAS_G2 (LM_P30_C64_GROUP==2)
#endif
#if defined __clang__ && defined LM_P30_C64_GROUP
/* A C64 group build compiles only its routines, so helpers that only the
 * other group's routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,DLOAD=06052,VAD=06751,VXV=07460,SETPD=07613,UNIT=03023,COMP=07670,PUSH=03247,
 ABVAL=03176,SXA=02364,CALLITA=02507,SWITCH=02516};
/* Fixed constants (35,2025, 35,3727 and 35,2026). */
#define K_V06N33 01441u
#define K_V06N81 01521u
#define K_V06N42 01452u
/* Destinations outside the routines (VNPOOH is lm-p34's). */
#define UPFLAG 05504u
#define DOWNFLAG 05516u
#define VNPOOH 03712u
#define INTPRET 06037u
#define FB34 070000u
#define FB35 072000u

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static void dload(apollo_overlay_state *s,agc_word word){opnd(s,word,DLOAD);lmi_dload_op(s,0);}
static void pdvl(apollo_overlay_state *s,agc_word word){opnd(s,word,PDVL);lmi_pdvl(s,0);}
static void vxv(apollo_overlay_state *s,agc_word word){opnd(s,word,VXV);lmi_vxv(s,0);}
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);lmi_dload_op(s,0);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
/* STCALL code label (fixed CADR) to the callee's NEWOPS. */
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void stq(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_ita(s);}
static void sxa(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,SXA);lmi_sxa(s);}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static void vcomp(apollo_overlay_state *s){lmi_unary(s,COMP);lmi_comp(s);}
/* UNIT and ABVAL: 1 when they end at POODOO1 (alarm 1302). */
static uint8_t unit(apollo_overlay_state *s){lmi_unary(s,UNIT);return lmi_unit(s);}
static uint8_t abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);return lmi_abvalabs(s);}

/* Canonical image at NEWOPS with LOC `loc` in FB `fb` (34 or 35), coherent
 * banks with BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC
 * one of the five with PUSHLOC no further than 32D into it, MODE one of the
 * three, IRQ 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word fb,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=fb||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,fb,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(fb),(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS, at POODOO1, or after EXIT at
 * the basic word that follows: the trampoline continues there or stops
 * outside the registered routines. */

#if HAS_G2
/* S30.1 STQ DLOAD QTEMP TIG; STCALL TDEC1 LEMPREC. */
INTERP(s301,FB34,02000u)
 lmi_newops(s,071220);stq(s,003632);right(s);dload(s,003442);next(s);
 stcall(s,034041,027057);
 return EXEC_OK;
}
#endif
#if HAS_G1
/* VLOAD SXA,2 RATT RTX2; STORE RTIG; UNIT VCOMP; STOVL DELVSIN VATT; STORE
 * VTIG; VXV UNIT RTIG; SETPD SXA,1 0 RTX1; PUSH VXV DELVSIN; VSL1 PDVL
 * (pushes up); PDVL PDVL DELVSIN DELVSLV; VXM VSL1 0; STORE DELVSIN; ABVAL;
 * STOVL DELVSAB RTIG; PDVL VAD VTIG DELVSIN; CALL PERIAPO1 (where LEMPREC
 * returns). */
INTERP(lemprec_ret,FB34,02005u)
 lmi_newops(s,067175);vload(s,000001);right(s);sxa(s,002777);next(s);
 store(s,003642);
 lmi_newops(s,057456);if(unit(s))return EXEC_OK;right(s);vcomp(s);next(s);
 stovl(s,027656,000007);
 store(s,003650);
 lmi_newops(s,053435);vxv(s,003642);right(s);if(unit(s))return EXEC_OK;next(s);
 lmi_newops(s,066001);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);sxa(s,002776);next(s);
 lmi_newops(s,047206);push(s);right(s);vxv(s,003656);next(s);
 lmi_newops(s,063372);shorts(s);right(s);pdvl(s,063315);next(s);
 lmi_newops(s,063315);pdvl(s,003656);right(s);pdvl(s,003434);next(s);
 lmi_newops(s,076505);if(!lm_work_matrix(s,000001,1))return EXEC_INVALID_ACTION;right(s);shorts(s);next(s);
 store(s,003656);
 lmi_newops(s,077646);if(abval(s))return EXEC_OK;next(s);
 stovl(s,027664,003642);
 lmi_newops(s,053315);pdvl(s,003650);right(s);opnd(s,003656,VAD);lmi_vad(s,0);next(s);
 lmi_newops(s,077624);call(s,046316);
 return EXEC_OK;
}
#endif
#if HAS_G2
/* CALL SHIFTR1 (where PERIAPO1 returns); CALL MAXCHK (where SHIFTR1
 * returns). */
INTERP(periapo_ret,FB34,02043u)
 lmi_newops(s,077624);call(s,046426);
 return EXEC_OK;
}
INTERP(shiftr1_ret,FB34,02045u)
 lmi_newops(s,077624);call(s,045636);
 return EXEC_OK;
}
/* STODL HPER 4D; CALL SHIFTR1 (where MAXCHK returns). */
INTERP(maxchk_ret,FB34,02047u)
 stodl(s,016321,000005);
 lmi_newops(s,077624);call(s,046426);
 return EXEC_OK;
}
/* CALL MAXCHK (where SHIFTR1 returns). */
INTERP(shiftr1_ret2,FB34,02053u)
 lmi_newops(s,077624);call(s,045636);
 return EXEC_OK;
}
/* STCALL HAPO QTEMP: to the CADR in QTEMP (where MAXCHK returns). */
INTERP(maxchk_ret2,FB34,02055u)
 lmi_store_code(s,036317);lmi_15bitadr(s,003632,CALLITA);lmi_call(s);lmi_goto_any(s);
 return EXEC_OK;
}
/* CALL S30.1 (after P30's INTPRET). */
INTERP(call_s301,FB35,02013u)
 lmi_newops(s,077624);call(s,070000);
 return EXEC_OK;
}
/* SET EXIT UPDATFLG (where S30.1 returns). */
INTERP(s301_ret,FB35,02015u)
 lmi_newops(s,077414);if(sw(s,io,000470))return EXEC_INVALID_ACTION;right(s);lmi_exit(s);
 return EXEC_OK;
}
/* SETGO XDELVFLG REVN1645 (after PARAM30's INTPRET). */
INTERP(setgo,FB35,02022u)
 lmi_newops(s,077614);if(!sw(s,io,001027))return EXEC_INVALID_ACTION;lmi_swbranch(s,072540);
 return EXEC_OK;
}
#endif
static const agc_routine_entry interpretive[]={
#if HAS_G2
 {LM_P30_B34(02000),s301},
#endif
#if HAS_G1
 {LM_P30_B34(02005),lemprec_ret},
#endif
#if HAS_G2
 {LM_P30_B34(02043),periapo_ret},{LM_P30_B34(02045),shiftr1_ret},{LM_P30_B34(02047),maxchk_ret},
 {LM_P30_B34(02053),shiftr1_ret2},{LM_P30_B34(02055),maxchk_ret2},
 {LM_P30_B35(02013),call_s301},{LM_P30_B35(02015),s301_ret},{LM_P30_B35(02022),setgo},
#endif
};
const agc_routine_table lm_p30_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 35 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=FB35||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* P30 TC UPFLAG (ADRES UPDATFLG); TC UPFLAG (ADRES TRACKFLG). */
ROUTINE(p30){ENTRY(02000u);tc(s,02001u,UPFLAG);return EXEC_OK;}
ROUTINE(updatflg_ret){ENTRY(02002u);tc(s,02003u,UPFLAG);return EXEC_OK;}
/* P30N33 CAF V06N33; TC VNPOOH (where UPFLAG returns). */
ROUTINE(p30n33){ENTRY(02004u);ai_caf(s,K_V06N33);tc(s,02006u,VNPOOH);return EXEC_OK;}
/* CAF V06N81; TC VNPOOH (where VNPOOH returns). */
ROUTINE(v06n81){ENTRY(02006u);ai_caf(s,K_V06N81);tc(s,02010u,VNPOOH);return EXEC_OK;}
/* TC DOWNFLAG (ADRES UPDATFLG) (where VNPOOH returns); TC INTPRET. */
ROUTINE(downflag){ENTRY(02010u);tc(s,02011u,DOWNFLAG);return EXEC_OK;}
ROUTINE(downflag_ret){ENTRY(02012u);tc(s,02013u,INTPRET);return EXEC_OK;}
/* PARAM30 CAF V06N42; TC VNPOOH (after the EXIT at 2016); TC INTPRET. */
ROUTINE(param30){ENTRY(02017u);ai_caf(s,K_V06N42);tc(s,02021u,VNPOOH);return EXEC_OK;}
ROUTINE(param30_ret){ENTRY(02021u);tc(s,02022u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
#if HAS_G2
 {LM_P30_B35(02000),p30},{LM_P30_B35(02002),updatflg_ret},{LM_P30_B35(02004),p30n33},{LM_P30_B35(02006),v06n81},
 {LM_P30_B35(02010),downflag},{LM_P30_B35(02012),downflag_ret},{LM_P30_B35(02017),param30},{LM_P30_B35(02021),param30_ret},
#else
 {0,0}
#endif
};
const agc_routine_table lm_p30_basic_routines={basic,(uint16_t)(HAS_G2?sizeof basic/sizeof *basic:0u)};
