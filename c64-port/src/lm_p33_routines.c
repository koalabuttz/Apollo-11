/* SPDX-License-Identifier: MIT */
/* Luminary099 P32-P35_P72-P75's P33/P73, AVFLAGA, AVFLAGP, P20FLGON and
 * DISDVLVC (bank 35 02216-02407) as routines of the control-transfer ABI;
 * see include/lm_p33_routines.h and docs/LM-P33.md. Every interpretive pair
 * is compiled with the shared bodies (src/lm_interp_steps.c,
 * src/lm_interp_orders.c): its original order and operand words appear below
 * only as the literal bookkeeping arguments of those paths, and each runtime
 * branch follows the outcome they return. MXV runs the compiled matrix
 * phases on the work-area matrix at 0D (src/lm_work_matrix.c). P30ZERO and
 * 60MIN are literal pairs that the source map checks against the fresh
 * rope. Each basic instruction is one ai_* register effect on the image, in
 * source order. */
#include "lm_p33_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "lm_work_matrix.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 DISDVLVC's
 * S32/33.X return (MXV) and the basic code its EXIT reaches, group 2 the
 * rest. Host builds register all. */
#ifndef LM_P33_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (LM_P33_C64_GROUP==1)
#define HAS_G2 (LM_P33_C64_GROUP==2)
#endif
#if defined __clang__ && defined LM_P33_C64_GROUP
/* A C64 group build compiles only its routines, so helpers and constants
 * that only the other group's routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-const-variable"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,DLOAD=06052,DAD=06775,DSU=07005,SIGN=07655,SETPD=07613,PUSH=03247,ABS=03176,
 STADR=06354,SXA=02364,CALLITA=02507,BZEGOTO=02467,BPLBMN=02474,SWITCH=02516};
/* Erasable (EB-window addresses as the source names them, EBANK=SUBEXIT). */
#define SUBEXIT 01470u
#define VERBNOUN 01615u
/* Fixed constants (35,2411, 2,1006, 35,2412, 35,2417-2420, 35,2421-2422). */
#define K_V06N13 01415u
#define K_V05N09 01211u
#define K_V06N75 01513u
#define K_P30ZERO_HI 0u
#define K_P30ZERO_LO 0u
#define K_60MIN_HI 025u
#define K_60MIN_LO 037100u
/* Destinations outside the routines (VNPOOH is lm-p34's). */
#define UPFLAG 05504u
#define DOWNFLAG 05516u
#define VNPOOH 03712u
#define ALARM 05567u
#define BANKCALL 04616u
#define GOTOPOOH 06001u
#define INTPRET 06037u
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
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);lmi_dload_op(s,0);next(s);
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
/* BPL/BMN to a fixed label: 1 when taken (ending at the label's NEWOPS). */
static uint8_t bpl(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BPLBMN);if(!lmi_bplbmn(s))return 0;lmi_goto(s);return 1;}
static const agc_word p30zero[2]={K_P30ZERO_HI,K_P30ZERO_LO},sixtymin[2]={K_60MIN_HI,K_60MIN_LO};
/* Canonical image at NEWOPS with LOC `loc` in FB 35, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 32D into it, MODE one of the three, IRQ
 * 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=FB35||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
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

/* P33/P73E DSU BPL 60MIN P33/P73E, until the difference is negative; DAD
 * 60MIN; STODL T1TOT2 TTPI; DSU PUSH TTPI0. P33/P73F ABS DSU 60MIN; BPL DAD
 * P33/P73F 60MIN, until negative; SIGN STADR (pushes up); STORE T2TOT3;
 * EXIT. Each taken BPL ends at its label's NEWOPS, where the loop goes
 * on. */
static void p33e_body(apollo_overlay_state *s){
 for(;;){
  lmi_newops(s,051025);lmi_direct(s,032422,DSU);lmi_dsu(s,sixtymin);right(s);
  if(!bpl(s,072314))break;
 }
 next(s);
 lmi_newops(s,077615);lmi_direct(s,032422,DAD);lmi_dad(s,sixtymin);next(s);
 stodl(s,016253,003636);
 lmi_newops(s,041425);opnd(s,003640,DSU);lmi_dsu(s,0);right(s);push(s);next(s);
}
static void p33f_body(apollo_overlay_state *s){
 for(;;){
  lmi_newops(s,045246);lmi_unary(s,ABS);(void)lmi_abvalabs(s);right(s);lmi_direct(s,032422,DSU);lmi_dsu(s,sixtymin);next(s);
  lmi_newops(s,043244);if(!bpl(s,072325))break;
 }
 right(s);lmi_direct(s,032422,DAD);lmi_dad(s,sixtymin);next(s);
 lmi_newops(s,045565);opnd(s,075522,SIGN);lmi_sign(s,0);right(s);lmi_unary(s,STADR);lmi_stadr(s,075522);next(s);
 lmi_newops(s,077776);lmi_exit(s);
}
/* P33/P73D DLOAD DAD NOMTPI TTPI; STORE TTPI; DSU TCDH; on into P33/P73E
 * and P33/P73F. */
static void p33d_body(apollo_overlay_state *s){
 lmi_newops(s,043345);dload(s,002317);right(s);opnd(s,003636,DAD);lmi_dad(s,0);next(s);
 store(s,003636);
 lmi_newops(s,077625);opnd(s,001777,DSU);lmi_dsu(s,0);next(s);
 p33e_body(s);p33f_body(s);
}
/* P33/P73C BON SET FINALFLG P33/P73D UPDATFLG; on into P33/P73D. 0 when a
 * switch order fails. */
static uint8_t p33c_body(apollo_overlay_state *s,p66_bias_io *io){
 lmi_newops(s,043014);
 if(sw(s,io,001311))lmi_swbranch(s,072306);
 else{right(s);if(sw(s,io,000470))return 0;next(s);}
 p33d_body(s);
 return 1;
}
#if HAS_G2
/* DLOAD TTPI0; STODL TTPI TCDH; STCALL TIG SELECTMU (after P33/P73A's
 * INTPRET). */
INTERP(p33a_interp,02225u)
 lmi_newops(s,077745);dload(s,003640);next(s);
 stodl(s,017636,001777);
 stcall(s,037442,020000);
 return EXEC_OK;
}
/* P33/P73B CALL ADVANCE; CALL CDHMVR (where ADVANCE returns). */
INTERP(p33b,02233u)
 lmi_newops(s,077624);call(s,071016);
 return EXEC_OK;
}
INTERP(advance_ret,02235u)
 lmi_newops(s,077624);call(s,071133);
 return EXEC_OK;
}
/* SETPD VLOAD 0D VACT3; PDVL CALL RACT2 INTINT3P (where CDHMVR returns). */
INTERP(cdhmvr_ret,02237u)
 lmi_newops(s,077201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);vload(s,003544);next(s);
 lmi_newops(s,045115);pdvl(s,002311);right(s);call(s,071071);
 return EXEC_OK;
}
/* CALL ACTIVE (where INTINT3P returns). */
INTERP(intint_ret,02245u)
 lmi_newops(s,077624);call(s,046402);
 return EXEC_OK;
}
/* SETPD VLOAD 0D VPASS2; PDVL CALL RPASS2 INTINT3P (where ACTIVE
 * returns). */
INTERP(active_ret,02247u)
 lmi_newops(s,077201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);vload(s,003530);next(s);
 lmi_newops(s,045115);pdvl(s,003522);right(s);call(s,071071);
 return EXEC_OK;
}
/* CALL PASSIVE (where INTINT3P returns). */
INTERP(intint_ret2,02255u)
 lmi_newops(s,077624);call(s,046412);
 return EXEC_OK;
}
/* DLOAD SET P30ZERO ITSWICH; STCALL NOMTPI S33/34.1 (where PASSIVE
 * returns). */
INTERP(passive_ret,02257u)
 lmi_newops(s,043145);lmi_direct(s,032420,DLOAD);lmi_dload_op(s,p30zero);right(s);if(sw(s,io,003460))return EXEC_INVALID_ACTION;next(s);
 stcall(s,036317,072764);
 return EXEC_OK;
}
/* BZE EXIT P33/P73C (where S33/34.1 returns). */
INTERP(s331_ret,02264u)
 lmi_newops(s,077454);lmi_misc(s,072302,BZEGOTO);
 if(lmi_bze(s))return EXEC_OK;
 right(s);lmi_exit(s);
 return EXEC_OK;
}
/* DLOAD P30ZERO; STORE NOMTPI; on into P33/P73C (after the INTPRET at
 * 2276). */
INTERP(proceed_ret,02277u)
 lmi_newops(s,077745);lmi_direct(s,032420,DLOAD);lmi_dload_op(s,p30zero);next(s);
 store(s,002317);
 if(!p33c_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
INTERP(p33c,02302u)
 if(!p33c_body(s,io))return EXEC_INVALID_ACTION;
 return EXEC_OK;
}
INTERP(p33d,02306u)
 p33d_body(s);
 return EXEC_OK;
}
INTERP(p33e,02314u)
 p33e_body(s);p33f_body(s);
 return EXEC_OK;
}
INTERP(p33f,02325u)
 p33f_body(s);
 return EXEC_OK;
}
/* VLOAD CALL DELVEET2 S32/33.1 (after the INTPRET at 2337). */
INTERP(vnpooh_ret_i,02340u)
 lmi_newops(s,045175);vload(s,002275);right(s);call(s,071100);
 return EXEC_OK;
}
/* STCALL DELVEET2 VN1645 (where S32/33.1 returns). */
INTERP(s3233_ret,02343u)
 stcall(s,036275,073606);
 return EXEC_OK;
}
/* GOTO P33/P73B (where VN1645 returns). */
INTERP(loop,02345u)
 lmi_newops(s,077650);lmi_misc(s,072233,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
/* DISDVLVC STORE DELVLVC; STQ CALL NORMEX S32/33.X. */
INTERP(disdvlvc,02370u)
 store(s,003434);
 lmi_newops(s,045020);stq(s,003463);right(s);call(s,071120);
 return EXEC_OK;
}
#endif
#if HAS_G1
/* VLOAD MXV DELVLVC 0D; VSL1 SXA,1 VERBNOUN; STORE DELVLVC; EXIT (where
 * S32/33.X returns). */
INTERP(s3233x_ret,02374u)
 lmi_newops(s,064375);vload(s,003434);right(s);if(!lm_work_matrix(s,000001,0))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,066172);shorts(s);right(s);sxa(s,003615);next(s);
 store(s,003434);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
#endif
#if HAS_G2
/* GOTO NORMEX: to the CADR in NORMEX (after DISDVLVC's INTPRET). */
INTERP(normex_goto,02406u)
 lmi_newops(s,077650);lmi_misc(s,003463,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);
 return EXEC_OK;
}
#endif
static const agc_routine_entry interpretive[]={
#if HAS_G2
 {LM_P33_B35(02225),p33a_interp},{LM_P33_B35(02233),p33b},{LM_P33_B35(02235),advance_ret},{LM_P33_B35(02237),cdhmvr_ret},
 {LM_P33_B35(02245),intint_ret},{LM_P33_B35(02247),active_ret},{LM_P33_B35(02255),intint_ret2},{LM_P33_B35(02257),passive_ret},
 {LM_P33_B35(02264),s331_ret},{LM_P33_B35(02277),proceed_ret},{LM_P33_B35(02302),p33c},{LM_P33_B35(02306),p33d},
 {LM_P33_B35(02314),p33e},{LM_P33_B35(02325),p33f},{LM_P33_B35(02340),vnpooh_ret_i},{LM_P33_B35(02343),s3233_ret},
 {LM_P33_B35(02345),loop},{LM_P33_B35(02370),disdvlvc},
#endif
#if HAS_G1
 {LM_P33_B35(02374),s3233x_ret},
#endif
#if HAS_G2
 {LM_P33_B35(02406),normex_goto},
#endif
};
const agc_routine_table lm_p33_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

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
#if HAS_G2
/* P33 TC AVFLAGA; TC P33/P73A (where AVFLAGA returns); P73 TC AVFLAGP. */
ROUTINE(p33){ENTRY(02216u);tc(s,02217u,02347u);return EXEC_OK;}
ROUTINE(avflaga_ret){ENTRY(02217u);tc(s,02220u,02221u);return EXEC_OK;}
ROUTINE(p73){ENTRY(02220u);tc(s,02221u,02354u);return EXEC_OK;}
/* P33/P73A TC P20FLGON; CAF V06N13; TC VNPOOH (where P20FLGON returns);
 * TC INTPRET. */
ROUTINE(p33a){ENTRY(02221u);tc(s,02222u,02361u);return EXEC_OK;}
ROUTINE(p20_ret){ENTRY(02222u);ai_caf(s,K_V06N13);tc(s,02224u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret){ENTRY(02224u);tc(s,02225u,INTPRET);return EXEC_OK;}
/* After S33/34.1's EXIT: TC ALARM (OCT 611 follows); CAF V05N09; TC
 * BANKCALL (CADR GOFLASH at Q) (where ALARM returns). */
ROUTINE(r_alarm){ENTRY(02266u);tc(s,02267u,ALARM);return EXEC_OK;}
ROUTINE(alarm_ret){ENTRY(02270u);ai_caf(s,K_V05N09);tc(s,02272u,BANKCALL);return EXEC_OK;}
/* GOFLASH's terminate, proceed and recycle returns: TC GOTOPOOH; TC +2;
 * TC P33/P73A; then TC INTPRET. */
ROUTINE(flash_terminate){ENTRY(02273u);tc(s,02274u,GOTOPOOH);return EXEC_OK;}
ROUTINE(flash_proceed){ENTRY(02274u);tc(s,02275u,02276u);return EXEC_OK;}
ROUTINE(flash_recycle){ENTRY(02275u);tc(s,02276u,02221u);return EXEC_OK;}
ROUTINE(r_intpret){ENTRY(02276u);tc(s,02277u,INTPRET);return EXEC_OK;}
/* After the EXIT at 2334: CAF V06N75; TC VNPOOH; TC INTPRET. */
ROUTINE(v06n75){ENTRY(02335u);ai_caf(s,K_V06N75);tc(s,02337u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret2){ENTRY(02337u);tc(s,02340u,INTPRET);return EXEC_OK;}
/* AVFLAGA EXTEND; QXCH SUBEXIT; TC UPFLAG (ADRES AVFLAG); TC SUBEXIT. */
ROUTINE(avflaga){ENTRY(02347u);ai_qxch(s,SUBEXIT);tc(s,02352u,UPFLAG);return EXEC_OK;}
ROUTINE(avflaga_up){ENTRY(02353u);tc(s,02354u,SUBEXIT);return EXEC_OK;}
/* AVFLAGP EXTEND; QXCH SUBEXIT; TC DOWNFLAG (ADRES AVFLAG); TC SUBEXIT. */
ROUTINE(avflagp){ENTRY(02354u);ai_qxch(s,SUBEXIT);tc(s,02357u,DOWNFLAG);return EXEC_OK;}
ROUTINE(avflagp_down){ENTRY(02360u);tc(s,02361u,SUBEXIT);return EXEC_OK;}
/* P20FLGON EXTEND; QXCH SUBEXIT; TC UPFLAG (ADRES UPDATFLG); TC UPFLAG
 * (ADRES TRACKFLG); TC SUBEXIT. */
ROUTINE(p20flgon){ENTRY(02361u);ai_qxch(s,SUBEXIT);tc(s,02364u,UPFLAG);return EXEC_OK;}
ROUTINE(p20flgon_up){ENTRY(02365u);tc(s,02366u,UPFLAG);return EXEC_OK;}
ROUTINE(p20flgon_up2){ENTRY(02367u);tc(s,02370u,SUBEXIT);return EXEC_OK;}
ROUTINE(vnpooh_ret3){ENTRY(02405u);tc(s,02406u,INTPRET);return EXEC_OK;}
#endif
/* After DISDVLVC's EXIT: CA VERBNOUN; TC VNPOOH (and TC INTPRET above). */
ROUTINE(disdvlvc_exit){ENTRY(02403u);ai_ca(s,VERBNOUN);tc(s,02405u,VNPOOH);return EXEC_OK;}
static const agc_routine_entry basic[]={
#if HAS_G2
 {LM_P33_B35(02216),p33},{LM_P33_B35(02217),avflaga_ret},{LM_P33_B35(02220),p73},{LM_P33_B35(02221),p33a},
 {LM_P33_B35(02222),p20_ret},{LM_P33_B35(02224),vnpooh_ret},{LM_P33_B35(02266),r_alarm},{LM_P33_B35(02270),alarm_ret},
 {LM_P33_B35(02273),flash_terminate},{LM_P33_B35(02274),flash_proceed},{LM_P33_B35(02275),flash_recycle},
 {LM_P33_B35(02276),r_intpret},{LM_P33_B35(02335),v06n75},{LM_P33_B35(02337),vnpooh_ret2},
 {LM_P33_B35(02347),avflaga},{LM_P33_B35(02353),avflaga_up},{LM_P33_B35(02354),avflagp},{LM_P33_B35(02360),avflagp_down},
 {LM_P33_B35(02361),p20flgon},{LM_P33_B35(02365),p20flgon_up},{LM_P33_B35(02367),p20flgon_up2},
 {LM_P33_B35(02403),disdvlvc_exit},{LM_P33_B35(02405),vnpooh_ret3},
#else
 {LM_P33_B35(02403),disdvlvc_exit},
#endif
};
const agc_routine_table lm_p33_basic_routines={basic,(uint16_t)(sizeof basic/sizeof *basic)};
