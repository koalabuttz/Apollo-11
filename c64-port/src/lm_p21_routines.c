/* SPDX-License-Identifier: MIT */
/* Luminary099 GROUND_TRACKING_DETERMINATION_PROGRAM (P21, bank 24
 * 03505-03667) as routines of the control-transfer ABI; see
 * include/lm_p21_routines.h and docs/LM-P21.md. Every interpretive pair is
 * compiled with the shared bodies (src/lm_interp_steps.c,
 * src/lm_interp_orders.c): its original order and operand words appear
 * below only as the literal bookkeeping arguments of those paths, and each
 * runtime branch follows the outcome they return. ASIN is dispatched to
 * ARCSIN (00,3610), outside these routines. V06N43, V6N34, K.01, 600SEC,
 * ONE and BIT2 are literals that the source map checks against the fresh
 * rope. Each basic instruction is one ai_* register effect on the image, in
 * source order. */
#include "lm_p21_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 TDEC1 through
 * P21VSAVE (INTEGRV, DDV, ASIN), group 2 the rest. Host builds register
 * all. Unregistered routines go unused there. */
#ifndef LM_P21_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (LM_P21_C64_GROUP==1)
#define HAS_G2 (LM_P21_C64_GROUP==2)
#endif
#if defined __clang__ && defined LM_P21_C64_GROUP
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,SLOAD=06501,DAD=06775,DMP=07103,DDV=07577,DOT=07331,GSHIFT=07621,UNIT=03023,
 ABVAL=03176,ARCSIN=03610,SXA=02364,CALLITA=02507,BZEGOTO=02467,RTBBHIZ=02447,SWITCH=02516};
/* Erasable (unswitched). */
#define OPTION2 01145u
/* Fixed constants (fixed-fixed ONE and BIT2, 24,3665, 24,3664, the K.01
 * pair at 24,3666 and the 600SEC pair at 24,3662). */
#define K_ONE 01u
#define K_BIT2 02u
#define K_V6N34 01442u
#define K_V06N43 01453u
#define K_K01_HI 000243u
#define K_K01_LO 032703u
#define K_600SEC_HI 03u
#define K_600SEC_LO 025140u
/* Destinations outside the routines. */
#define BANKCALL 04616u
#define GOTOPOOH 06001u
#define INTPRET 06037u
#define FB24 050000u

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static void dload(apollo_overlay_state *s,agc_word word){opnd(s,word,DLOAD);lmi_dload_op(s,0);}
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
static void sload(apollo_overlay_state *s,agc_word word){lmi_direct(s,word,SLOAD);lmi_sload(s);}
static uint8_t bze(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);return lmi_bze(s);}
static const agc_word k01[2]={K_K01_HI,K_K01_LO},sec600[2]={K_600SEC_HI,K_600SEC_LO};

/* Canonical image at NEWOPS with LOC `loc` in FB 24, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 32D into it, MODE one of the three, IRQ
 * 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=FB24||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
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

/* DLOAD DSPTEM1; STCALL TDEC1 INTSTALL (after the INTPRET at 3523). */
INTERP(p21_tdec1,03524u)
 lmi_newops(s,077745);dload(s,001046);next(s);
 stcall(s,034041,027414);
 return EXEC_OK;
}
/* CLEAR CLEAR DIM0FLAG INTYPFLG; CALL INTEGRV. */
static uint8_t p21_integrv_body(apollo_overlay_state *s,p66_bias_io *io){
 lmi_newops(s,043014);if(sw(s,io,001676))return 0;right(s);if(sw(s,io,001673))return 0;next(s);
 lmi_newops(s,077624);call(s,027134);
 return 1;
}
/* BON CLEAR P21FLAG P21CONT VINTFLAG; SLOAD SR1 OPTION2; BHIZ SET +2
 * VINTFLAG; on into CLEAR CLEAR and CALL INTEGRV (where INTSTALL
 * returns). A set P21FLAG ends at P21CONT's NEWOPS, a zero OPTION2 at the
 * NEWOPS two words on. */
INTERP(p21_flags,03530u)
 lmi_newops(s,043014);
 if(sw(s,io,000304)){lmi_swbranch(s,051550);return EXEC_OK;}
 right(s);if(sw(s,io,001674))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,070535);sload(s,001146);right(s);shorts(s);next(s);
 lmi_newops(s,043030);lmi_misc(s,051541,RTBBHIZ);(void)ai_ccs(s,020);
 if(lmi_bhiz(s)){lmi_goto(s);return EXEC_OK;}
 right(s);if(sw(s,io,001474))return EXEC_INVALID_ACTION;next(s);
 if(!p21_integrv_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
INTERP(p21_integrv,03541u)
 if(!p21_integrv_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
/* GOTO P21VSAVE (where INTEGRV returns). */
INTERP(integrv_ret,03546u)
 lmi_newops(s,077650);lmi_misc(s,051571,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
/* P21CONT VLOAD P21BASER; STOVL RCV P21BASEV; STODL VCV P21TIME; STORE
 * TET; CLEAR CLEAR DIM0FLAG MOONFLAG; SLOAD BZE P21ORIG +3; SET MOONFLAG
 * unless P21ORIG is zero; CALL INTEGRVS. */
INTERP(p21cont,03550u)
 lmi_newops(s,077775);vload(s,003676);next(s);
 stovl(s,025535,003704);
 stodl(s,015543,003763);
 store(s,001517);
 lmi_newops(s,043014);if(sw(s,io,001676))return EXEC_INVALID_ACTION;right(s);if(sw(s,io,000263))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,053135);sload(s,001164);right(s);
 if(!bze(s,051567)){next(s);lmi_newops(s,077614);if(sw(s,io,000063))return EXEC_INVALID_ACTION;next(s);}
 lmi_newops(s,077624);call(s,027107);
 return EXEC_OK;
}
/* P21VSAVE DLOAD TAT; STOVL P21TIME RATT1; STOVL P21BASER VATT1; STORE
 * P21BASEV; ABVAL SL* 0,2; STOVL P21VEL RATT; UNIT DOT VATT; DDV ASIN
 * P21VEL, ASIN dispatched to ARCSIN (where INTEGRVS returns). */
INTERP(p21vsave,03571u)
 lmi_newops(s,077745);dload(s,000015);next(s);
 stovl(s,027763,000017);
 stovl(s,027676,000025);
 store(s,003704);
 lmi_newops(s,053646);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return EXEC_OK;right(s);gshift(s,057576);next(s);
 stovl(s,027712,000001);
 lmi_newops(s,050256);lmi_unary(s,UNIT);if(lmi_unit(s))return EXEC_OK;right(s);opnd(s,000007,DOT);lmi_dot(s,0);next(s);
 lmi_newops(s,067471);opnd(s,003712,DDV);lmi_ddv(s,0);right(s);lmi_unary(s,ARCSIN);
 return EXEC_OK;
}
/* P21DSP CLEAR SLOAD LUNAFLAG X2; BZE SET +2 LUNAFLAG, LUNAFLAG set unless
 * X2 is zero; VLOAD RATT; STODL ALPHAV TAT; CLEAR CALL ERADFLAG LAT-LONG. */
static uint8_t p21dsp_body(apollo_overlay_state *s,p66_bias_io *io){
 lmi_newops(s,067214);if(sw(s,io,001663))return 0;right(s);sload(s,000050);next(s);
 lmi_newops(s,043054);
 if(!bze(s,051633)){right(s);if(sw(s,io,001463))return 0;next(s);}
 lmi_newops(s,077775);vload(s,000001);next(s);
 stodl(s,016032,000015);
 lmi_newops(s,045014);if(sw(s,io,000662))return 0;right(s);call(s,026351);
 return 1;
}
/* STORE P21GAM; SXA,2 SLOAD P21ORIG OPTION2; SR1 BHIZ +3; GOTO +4; +3 BON
 * SURFFLAG P21DSP; +4 SET P21FLAG; on into P21DSP (where ARCSIN returns).
 * A set SURFFLAG ends at P21DSP's NEWOPS. */
INTERP(arcsin_ret,03610u)
 store(s,003714);
 lmi_newops(s,067334);lmi_misc(s,001163,SXA);lmi_sxa(s);right(s);sload(s,001146);next(s);
 lmi_newops(s,046142);shorts(s);right(s);lmi_misc(s,051620,RTBBHIZ);(void)ai_ccs(s,020);
 if(!lmi_bhiz(s)){next(s);lmi_newops(s,077650);lmi_misc(s,051623,BZEGOTO);lmi_goto_order(s);}
 else{lmi_goto(s);lmi_newops(s,077614);if(sw(s,io,004307)){lmi_swbranch(s,051625);return EXEC_OK;}next(s);}
 lmi_newops(s,077614);if(sw(s,io,000064))return EXEC_INVALID_ACTION;next(s);
 if(!p21dsp_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
INTERP(p21dsp,03625u)
 if(!p21dsp_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
/* DMP K.01; STORE P21ALT; EXIT (where LAT-LONG returns). */
INTERP(latlong_ret,03642u)
 lmi_newops(s,077605);lmi_direct(s,011667,DMP);lmi_dmp(s,k01);next(s);
 store(s,003716);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* DLOAD DAD P21TIME 600SEC; STORE DSPTEM1; RTB P21PROG1 (after the INTPRET
 * at 3653). */
INTERP(p21_next,03654u)
 lmi_newops(s,043345);dload(s,003763);right(s);lmi_direct(s,011663,DAD);lmi_dad(s,sec600);next(s);
 store(s,001046);
 lmi_newops(s,077634);lmi_misc(s,051515,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
#if HAS_G1
 {LM_P21_B24(03524),p21_tdec1},{LM_P21_B24(03530),p21_flags},{LM_P21_B24(03541),p21_integrv},{LM_P21_B24(03546),integrv_ret},
 {LM_P21_B24(03550),p21cont},{LM_P21_B24(03571),p21vsave},
#endif
#if HAS_G2
 {LM_P21_B24(03610),arcsin_ret},{LM_P21_B24(03625),p21dsp},
 {LM_P21_B24(03642),latlong_ret},{LM_P21_B24(03654),p21_next},
#endif
};
const agc_routine_table lm_p21_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 24 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=FB24||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* CAF BIT2; TC BANKCALL (CADR GOPERF4). */
static void perf4(apollo_overlay_state *s){ai_caf(s,K_BIT2);tc(s,03511u,BANKCALL);}
/* P21PROG1 CAF V6N34; TC BANKCALL (CADR GOFLASH). */
static void flash34(apollo_overlay_state *s){ai_caf(s,K_V6N34);tc(s,03517u,BANKCALL);}
/* PROG21 CAF ONE; TS OPTION2; on into the GOPERF4 request. */
ROUTINE(prog21){ENTRY(03505u);ai_caf(s,K_ONE);(void)ai_ts(s,OPTION2);perf4(s);return EXEC_OK;}
ROUTINE(r_perf4){ENTRY(03507u);perf4(s);return EXEC_OK;}
/* GOPERF4's returns: TC GOTOPOOH; TC +2 (P21PROG1); TC -5. */
ROUTINE(perf4_terminate){ENTRY(03512u);tc(s,03513u,GOTOPOOH);return EXEC_OK;}
ROUTINE(perf4_proceed){ENTRY(03513u);tc(s,03514u,03515u);flash34(s);return EXEC_OK;}
ROUTINE(perf4_enter){ENTRY(03514u);tc(s,03515u,03507u);perf4(s);return EXEC_OK;}
ROUTINE(p21prog1){ENTRY(03515u);flash34(s);return EXEC_OK;}
/* GOFLASH's returns: TC GOTOPOOH; TC +2 (TC INTPRET); TC -5. */
ROUTINE(f34_terminate){ENTRY(03520u);tc(s,03521u,GOTOPOOH);return EXEC_OK;}
ROUTINE(f34_proceed){ENTRY(03521u);tc(s,03522u,03523u);tc(s,03524u,INTPRET);return EXEC_OK;}
ROUTINE(f34_recycle){ENTRY(03522u);tc(s,03523u,03515u);flash34(s);return EXEC_OK;}
ROUTINE(r_intpret){ENTRY(03523u);tc(s,03524u,INTPRET);return EXEC_OK;}
/* After the EXIT at 3645: CAF V06N43; TC BANKCALL (CADR GOFLASH); its
 * returns TC GOTOPOOH, TC GOTOPOOH and TC INTPRET. */
ROUTINE(v06n43){ENTRY(03646u);ai_caf(s,K_V06N43);tc(s,03650u,BANKCALL);return EXEC_OK;}
ROUTINE(f43_terminate){ENTRY(03651u);tc(s,03652u,GOTOPOOH);return EXEC_OK;}
ROUTINE(f43_proceed){ENTRY(03652u);tc(s,03653u,GOTOPOOH);return EXEC_OK;}
ROUTINE(f43_recycle){ENTRY(03653u);tc(s,03654u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
#if HAS_G2
 {LM_P21_B24(03505),prog21},{LM_P21_B24(03507),r_perf4},{LM_P21_B24(03512),perf4_terminate},{LM_P21_B24(03513),perf4_proceed},
 {LM_P21_B24(03514),perf4_enter},{LM_P21_B24(03515),p21prog1},{LM_P21_B24(03520),f34_terminate},{LM_P21_B24(03521),f34_proceed},
 {LM_P21_B24(03522),f34_recycle},{LM_P21_B24(03523),r_intpret},{LM_P21_B24(03646),v06n43},{LM_P21_B24(03651),f43_terminate},
 {LM_P21_B24(03652),f43_proceed},{LM_P21_B24(03653),f43_recycle},
#else
 {0,0}
#endif
};
const agc_routine_table lm_p21_basic_routines={basic,(uint16_t)(HAS_G2?sizeof basic/sizeof *basic:0u)};
