/* SPDX-License-Identifier: MIT */
/* Comanche055 GROUND_TRACKING_DETERMINATION_PROGRAM (P21, bank 37
 * 02001-02154) as routines of the control-transfer ABI; see
 * include/cm_p21_routines.h and docs/CM-P21.md. Every interpretive pair is
 * compiled with the shared bodies built for Comanche055
 * (src/cm_interp_steps.c, src/cm_interp_orders.c): its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * those paths, and each runtime branch follows the outcome they return. ASIN
 * is dispatched to ARCSIN (00,3607), outside these routines. V06N43, V6N34,
 * K.01, 600SEC, ONE and BIT2 are literals that the source map checks
 * against the fresh rope. Each basic instruction is one ai_* register effect
 * on the image, in source order. */
#include "cm_p21_routines.h"
#include "cm_interp.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
/* The routines a C64 build registers (C64 RAM): group 1 TDEC1 through
 * P21VSAVE (INTEGRV, DDV, ASIN), group 2 the rest. Host builds register
 * all. Unregistered routines go unused there. */
#ifndef CM_P21_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (CM_P21_C64_GROUP==1)
#define HAS_G2 (CM_P21_C64_GROUP==2)
#endif
#if defined __clang__ && defined CM_P21_C64_GROUP
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries (Comanche055). */
enum {VLOAD=06454,DLOAD=06021,SLOAD=06450,DAD=06744,DMP=07052,DDV=07546,DOT=07300,GSHIFT=07570,UNIT=03023,
 ABVAL=03201,ARCSIN=03607,SXA=02411,CALLITA=02534,BZEGOTO=02514,RTBBHIZ=02474,SWITCH=02543};
/* Erasable (unswitched). */
#define OPTION2 01132u
/* Fixed constants (fixed-fixed ONE and BIT2, 37,2152, 37,2151, the K.01
 * pair at 37,2153 and the 600SEC pair at 37,2145). */
#define K_ONE 01u
#define K_BIT2 02u
#define K_V6N34 01442u
#define K_V06N43 01453u
#define K_K01_HI 000243u
#define K_K01_LO 032703u
#define K_600SEC_HI 03u
#define K_600SEC_LO 025140u
/* P21ONENN (37,2147, OCT 00001,00000) is read by TPI_SEARCH, not P21. */
#define K_P21ONENN_HI 01u
#define K_P21ONENN_LO 0u
/* Destinations outside the routines. */
#define BANKCALL 04662u
#define GOTOPOOH 04106u
#define INTPRET 06006u
#define FB37 076000u

static void next(apollo_overlay_state *s){cmi_danzig_next(s);}
static void right(apollo_overlay_state *s){cmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)cmi_address_pushup(s,word,target);else cmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){cmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);cmi_vload(s,0);}
static void dload(apollo_overlay_state *s,agc_word word){opnd(s,word,DLOAD);cmi_dload_op(s,0);}
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
static void sload(apollo_overlay_state *s,agc_word word){cmi_direct(s,word,SLOAD);cmi_sload(s);}
static uint8_t bze(apollo_overlay_state *s,agc_word word){cmi_misc(s,word,BZEGOTO);return cmi_bze(s);}
static const agc_word k01[2]={K_K01_HI,K_K01_LO},sec600[2]={K_600SEC_HI,K_600SEC_LO};

/* Canonical image at NEWOPS with LOC `loc` in FB 37, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 32D into it, MODE one of the three, IRQ
 * 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06037u||W[4]!=FB37||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS, at ARCSIN, at POODOO, after
 * RTB at the basic routine it names, or after EXIT at the basic word that
 * follows: the trampoline continues there or stops outside the registered
 * routines. */

/* DLOAD DSPTEM1; STCALL TDEC1 INTSTALL (after the INTPRET at 2017). */
INTERP(p21_tdec1,02020u)
 cmi_newops(s,077745);dload(s,001046);next(s);
 stcall(s,034041,027442);
 return EXEC_OK;
}
/* CLEAR CLEAR DIM0FLAG INTYPFLG; CALL INTEGRV. */
static uint8_t p21_integrv_body(apollo_overlay_state *s,p66_bias_io *io){
 cmi_newops(s,043014);if(sw(s,io,001676))return 0;right(s);if(sw(s,io,001673))return 0;next(s);
 cmi_newops(s,077624);call(s,027162);
 return 1;
}
/* BON SET P21FLAG P21CONT VINTFLAG; SLOAD SR1 OPTION2; BHIZ CLEAR +2
 * VINTFLAG; on into CLEAR CLEAR and CALL INTEGRV (where INTSTALL
 * returns). A set P21FLAG ends at P21CONT's NEWOPS, a zero OPTION2 at the
 * NEWOPS two words on. */
INTERP(p21_flags,02024u)
 cmi_newops(s,043014);
 if(sw(s,io,001303)){cmi_swbranch(s,076044);return EXEC_OK;}
 right(s);if(sw(s,io,001474))return EXEC_INVALID_ACTION;next(s);
 cmi_newops(s,070535);sload(s,001133);right(s);shorts(s);next(s);
 cmi_newops(s,043030);cmi_misc(s,076035,RTBBHIZ);(void)ai_ccs(s,020);
 if(cmi_bhiz(s)){cmi_goto(s);return EXEC_OK;}
 right(s);if(sw(s,io,001674))return EXEC_INVALID_ACTION;next(s);
 if(!p21_integrv_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
INTERP(p21_integrv,02035u)
 if(!p21_integrv_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
/* GOTO P21VSAVE (where INTEGRV returns). */
INTERP(integrv_ret,02042u)
 cmi_newops(s,077650);cmi_misc(s,076065,BZEGOTO);cmi_goto_order(s);
 return EXEC_OK;
}
/* P21CONT VLOAD P21BASER; STOVL RCV P21BASEV; STODL VCV P21TIME; STORE
 * TET; CLEAR CLEAR DIM0FLAG MOONFLAG; SLOAD BZE P21ORIG +3; SET MOONFLAG
 * unless P21ORIG is zero; CALL INTEGRVS. */
INTERP(p21cont,02044u)
 cmi_newops(s,077775);vload(s,003605);next(s);
 stovl(s,025535,003613);
 stodl(s,015543,002316);
 store(s,001517);
 cmi_newops(s,043014);if(sw(s,io,001676))return EXEC_INVALID_ACTION;right(s);if(sw(s,io,000263))return EXEC_INVALID_ACTION;next(s);
 cmi_newops(s,053135);sload(s,003604);right(s);
 if(!bze(s,076063)){next(s);cmi_newops(s,077614);if(sw(s,io,000063))return EXEC_INVALID_ACTION;next(s);}
 cmi_newops(s,077624);call(s,027135);
 return EXEC_OK;
}
/* P21VSAVE DLOAD TAT; STOVL P21TIME RATT1; STOVL P21BASER VATT1; STORE
 * P21BASEV; ABVAL SL* 0,2; STOVL P21VEL RATT; UNIT DOT VATT; DDV ASIN
 * P21VEL, ASIN dispatched to ARCSIN (where INTEGRVS returns). */
INTERP(p21vsave,02065u)
 cmi_newops(s,077745);dload(s,000015);next(s);
 stovl(s,026316,000017);
 stovl(s,027605,000025);
 store(s,003613);
 cmi_newops(s,053646);cmi_unary(s,ABVAL);if(cmi_abvalabs(s))return EXEC_OK;right(s);gshift(s,057576);next(s);
 stovl(s,027623,000001);
 cmi_newops(s,050256);cmi_unary(s,UNIT);if(cmi_unit(s))return EXEC_OK;right(s);opnd(s,000007,DOT);cmi_dot(s,0);next(s);
 cmi_newops(s,067471);opnd(s,003623,DDV);cmi_ddv(s,0);right(s);cmi_unary(s,ARCSIN);
 return EXEC_OK;
}
/* P21DSP CLEAR SLOAD LUNAFLAG X2; BZE SET +2 LUNAFLAG, LUNAFLAG set unless
 * X2 is zero; VLOAD RATT; STODL ALPHAV TAT; CLEAR CALL ERADFLAG LAT-LONG. */
static uint8_t p21dsp_body(apollo_overlay_state *s,p66_bias_io *io){
 cmi_newops(s,067214);if(sw(s,io,001663))return 0;right(s);sload(s,000050);next(s);
 cmi_newops(s,043054);
 if(!bze(s,076116)){right(s);if(sw(s,io,001463))return 0;next(s);}
 cmi_newops(s,077775);vload(s,000001);next(s);
 stodl(s,016152,000015);
 cmi_newops(s,045014);if(sw(s,io,000662))return 0;right(s);call(s,026370);
 return 1;
}
/* STORE P21GAM; SXA,2 SET P21ORIG P21FLAG; on into P21DSP (where ARCSIN
 * returns). */
INTERP(arcsin_ret,02104u)
 store(s,003625);
 cmi_newops(s,043134);cmi_misc(s,003603,SXA);cmi_sxa(s);right(s);if(sw(s,io,001063))return EXEC_INVALID_ACTION;next(s);
 if(!p21dsp_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
INTERP(p21dsp,02110u)
 if(!p21dsp_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
/* DMP K.01; STORE P21ALT; EXIT (where LAT-LONG returns). */
INTERP(latlong_ret,02125u)
 cmi_newops(s,077605);cmi_direct(s,036154,DMP);cmi_dmp(s,k01);next(s);
 store(s,003621);
 cmi_newops(s,077776);cmi_exit(s);
 return EXEC_OK;
}
/* DLOAD DAD P21TIME 600SEC; STORE DSPTEM1; RTB P21PROG1 (after the INTPRET
 * at 2136). */
INTERP(p21_next,02137u)
 cmi_newops(s,043345);dload(s,002316);right(s);cmi_direct(s,036146,DAD);cmi_dad(s,sec600);next(s);
 store(s,001046);
 cmi_newops(s,077634);cmi_misc(s,076011,RTBBHIZ);(void)ai_ccs(s,020);cmi_rtb(s);
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
#if HAS_G1
 {CM_P21_B37(02020),p21_tdec1},{CM_P21_B37(02024),p21_flags},{CM_P21_B37(02035),p21_integrv},{CM_P21_B37(02042),integrv_ret},
 {CM_P21_B37(02044),p21cont},{CM_P21_B37(02065),p21vsave},
#endif
#if HAS_G2
 {CM_P21_B37(02104),arcsin_ret},{CM_P21_B37(02110),p21dsp},
 {CM_P21_B37(02125),latlong_ret},{CM_P21_B37(02137),p21_next},
#endif
};
const agc_routine_table cm_p21_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 37 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=FB37||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* CAF BIT2; TC BANKCALL (CADR GOPERF4). */
static void perf4(apollo_overlay_state *s){ai_caf(s,K_BIT2);tc(s,02005u,BANKCALL);}
/* P21PROG1 CAF V6N34; TC BANKCALL (CADR GOFLASH). */
static void flash34(apollo_overlay_state *s){ai_caf(s,K_V6N34);tc(s,02013u,BANKCALL);}
/* PROG21 CAF ONE; TS OPTION2; on into the GOPERF4 request. */
ROUTINE(prog21){ENTRY(02001u);ai_caf(s,K_ONE);(void)ai_ts(s,OPTION2);perf4(s);return EXEC_OK;}
ROUTINE(r_perf4){ENTRY(02003u);perf4(s);return EXEC_OK;}
/* GOPERF4's returns: TC GOTOPOOH; TC +2 (P21PROG1); TC -5. */
ROUTINE(perf4_terminate){ENTRY(02006u);tc(s,02007u,GOTOPOOH);return EXEC_OK;}
ROUTINE(perf4_proceed){ENTRY(02007u);tc(s,02010u,02011u);flash34(s);return EXEC_OK;}
ROUTINE(perf4_enter){ENTRY(02010u);tc(s,02011u,02003u);perf4(s);return EXEC_OK;}
ROUTINE(p21prog1){ENTRY(02011u);flash34(s);return EXEC_OK;}
/* GOFLASH's returns: TC GOTOPOOH; TC +2 (TC INTPRET); TC -5. */
ROUTINE(f34_terminate){ENTRY(02014u);tc(s,02015u,GOTOPOOH);return EXEC_OK;}
ROUTINE(f34_proceed){ENTRY(02015u);tc(s,02016u,02017u);tc(s,02020u,INTPRET);return EXEC_OK;}
ROUTINE(f34_recycle){ENTRY(02016u);tc(s,02017u,02011u);flash34(s);return EXEC_OK;}
ROUTINE(r_intpret){ENTRY(02017u);tc(s,02020u,INTPRET);return EXEC_OK;}
/* After the EXIT at 2130: CAF V06N43; TC BANKCALL (CADR GOFLASH); its
 * returns TC GOTOPOOH, TC GOTOPOOH and TC INTPRET. */
ROUTINE(v06n43){ENTRY(02131u);ai_caf(s,K_V06N43);tc(s,02133u,BANKCALL);return EXEC_OK;}
ROUTINE(f43_terminate){ENTRY(02134u);tc(s,02135u,GOTOPOOH);return EXEC_OK;}
ROUTINE(f43_proceed){ENTRY(02135u);tc(s,02136u,GOTOPOOH);return EXEC_OK;}
ROUTINE(f43_recycle){ENTRY(02136u);tc(s,02137u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
#if HAS_G2
 {CM_P21_B37(02001),prog21},{CM_P21_B37(02003),r_perf4},{CM_P21_B37(02006),perf4_terminate},{CM_P21_B37(02007),perf4_proceed},
 {CM_P21_B37(02010),perf4_enter},{CM_P21_B37(02011),p21prog1},{CM_P21_B37(02014),f34_terminate},{CM_P21_B37(02015),f34_proceed},
 {CM_P21_B37(02016),f34_recycle},{CM_P21_B37(02017),r_intpret},{CM_P21_B37(02131),v06n43},{CM_P21_B37(02134),f43_terminate},
 {CM_P21_B37(02135),f43_proceed},{CM_P21_B37(02136),f43_recycle},
#else
 {0,0}
#endif
};
const agc_routine_table cm_p21_basic_routines={basic,(uint16_t)(HAS_G2?sizeof basic/sizeof *basic:0u)};
