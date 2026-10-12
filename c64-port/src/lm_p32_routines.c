/* SPDX-License-Identifier: MIT */
/* Luminary099 P32-P35_P72-P75's P32/P72 with ALMXITA and ALMXIT (bank 35
 * 02027-02215) as routines of the control-transfer ABI; see
 * include/lm_p32_routines.h and docs/LM-P32.md. Every interpretive pair is
 * compiled with the shared bodies (src/lm_interp_steps.c,
 * src/lm_interp_orders.c): its original order and operand words appear below
 * only as the literal bookkeeping arguments of those paths, and each runtime
 * branch follows the outcome they return. P30ZERO, 60MIN, TWOPI, DPPOSMAX
 * and the ALARM/TB table are literals that the source map checks against the
 * fresh rope. Each basic instruction is one ai_* register effect on the
 * image, in source order. */
#include "lm_p32_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#define W s->words
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,PDDL=06523,DLOAD=06052,SLOAD=06501,DAD=06775,DSU=07005,GSHIFT=07621,SETPD=07613,
 PUSH=03247,COMP=07670,SXA=02364,LXC=02360,AXT=02344,CALLITA=02507,BZEGOTO=02467,BPLBMN=02474,SWITCH=02516};
/* Erasable (EB-window addresses as the source names them, EBANK=SUBEXIT). */
#define MPAC 0154u
#define CENTANG 01620u
#define NN1 01467u
#define TCSI 01633u
#define CSIALRM 03613u
/* Fixed constants (35,2417-2420, 35,2410, 2,1006, 35,3724, 35,3723,
 * 35,2412, 35,2421-2422, 35,3731-3732, 23,2527-2530 and ALARM/TB at
 * 35,2423-2431). */
#define K_P30ZERO_HI 0u
#define K_P30ZERO_LO 0u
#define K_V06N11 01413u
#define K_V05N09 01211u
#define K_V06N55 01467u
#define K_V06N37 01445u
#define K_V06N75 01513u
#define K_60MIN_HI 025u
#define K_60MIN_LO 037100u
#define K_TWOPI_HI 014441u
#define K_TWOPI_LO 037325u
#define K_DPPOSMAX_HI 037777u
#define K_DPPOSMAX_LO 037777u
#define K_ALARMTB1 0600u
#define K_ALARMTB2 0601u
#define K_ALARMTB3 0602u
#define K_ALARMTB4 0603u
#define K_ALARMTB5 0604u
#define K_ALARMTB6 0605u
#define K_ALARMTB7 0606u
/* Destinations outside the routines (AVFLAGA, AVFLAGP and P20FLGON are
 * lm-p33's, VNPOOH lm-p34's). */
#define AVFLAGA 02347u
#define AVFLAGP 02354u
#define P20FLGON 02361u
#define VNPOOH 03712u
#define VARALARM 05744u
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
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
/* STCALL code label (fixed CADR) to the callee's NEWOPS. */
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
/* BPL/BMN to a fixed label: 1 when taken (ending at the label's NEWOPS). */
static uint8_t bpl(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BPLBMN);if(!lmi_bplbmn(s))return 0;lmi_goto(s);return 1;}
static const agc_word p30zero[2]={K_P30ZERO_HI,K_P30ZERO_LO},sixtymin[2]={K_60MIN_HI,K_60MIN_LO},
 twopi[2]={K_TWOPI_HI,K_TWOPI_LO},dpposmax[2]={K_DPPOSMAX_HI,K_DPPOSMAX_LO},
 alarmtb[7]={K_ALARMTB1,K_ALARMTB2,K_ALARMTB3,K_ALARMTB4,K_ALARMTB5,K_ALARMTB6,K_ALARMTB7};
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

/* ALMXITA SXA,2 CSIALRM; on into ALMXIT's NEWOPS. */
INTERP(almxita,02036u)
 lmi_newops(s,077734);lmi_misc(s,003613,SXA);lmi_sxa(s);next(s);
 return EXEC_OK;
}
/* ALMXIT LXC,1 CSIALRM; SLOAD* EXIT ALARM/TB -1,1. The alarm table is
 * the seven words at 35,2423-2431, so CSIALRM must name one of them. */
INTERP(almxit,02040u)
 if(W[CSIALRM]<1u||W[CSIALRM]>7u)return EXEC_OUT_OF_SCOPE;
 lmi_newops(s,077740);lmi_misc(s,003613,LXC);lmi_lxc(s);next(s);
 lmi_newops(s,077533);lmi_indexed(s,032423,SLOAD);
 if(W[4]!=FB35||W[0116]<02423u||W[0116]>02431u)return EXEC_INVALID_ACTION;
 /* SLOAD's effect on ADDRWD's word (the table literal): ZL; CA; SLOAD2. */
 W[1]=0;ai_caf(s,alarmtb[W[0116]-02423u]);ai_dxch(s,MPAC);ai_caf(s,0);(void)ai_ts(s,0156);(void)ai_ts(s,0163);W[5]=06061u;
 right(s);lmi_exit(s);
 return EXEC_OK;
}
/* DLOAD DCOMP TCSI; BMN DLOAD VN0655 TETLEM; STCALL TDEC1 PRECSET (after
 * the INTPRET at 2062). */
INTERP(tcsi_check,02063u)
 lmi_newops(s,057545);dload(s,003634);right(s);lmi_unary(s,COMP);lmi_comp(s);next(s);
 lmi_newops(s,071240);if(bpl(s,072113))return EXEC_OK;
 right(s);dload(s,001643);next(s);
 stcall(s,034041,046360);
 return EXEC_OK;
}
/* VLOAD VSR* RACT3 0,2; STOVL RVEC VACT3; VSR* SET 0,2 RVSW; STODL VVEC
 * DPPOSMAX; STCALL RDESIRED TIMERAD (where PRECSET returns). */
INTERP(precset_ret,02072u)
 lmi_newops(s,053775);vload(s,003536);right(s);gshift(s,057176);next(s);
 stovl(s,026655,003544);
 lmi_newops(s,043057);gshift(s,057176);right(s);if(sw(s,io,003466))return EXEC_INVALID_ACTION;next(s);
 lmi_store_code(s,016744);lmi_dodload(s);lmi_diradres(s,006530,DLOAD);lmi_dload_op(s,dpposmax);next(s);
 stcall(s,036756,025547);
 return EXEC_OK;
}
/* DAD TDEC2; STORE TCSI; EXIT (where TIMERAD returns). */
INTERP(timerad_ret,02106u)
 lmi_newops(s,077615);opnd(s,003574,DAD);lmi_dad(s,0);next(s);
 store(s,003634);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* VN0655 EXIT. */
INTERP(vn0655,02113u)
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* DLOAD TCSI; STCALL TIG SELECTMU (after the INTPRET at 2124). */
INTERP(tcsi_tig,02125u)
 lmi_newops(s,077745);dload(s,003634);next(s);
 stcall(s,037442,020000);
 return EXEC_OK;
}
/* P32/P72B CALL ADVANCE. */
INTERP(p32b,02131u)
 lmi_newops(s,077624);call(s,071016);
 return EXEC_OK;
}
/* SETPD VLOAD 0D VPASS1; PDVL PDDL RPASS1 TCSI; PDDL PDDL TTPI TWOPI; PUSH
 * CALL INTINT (where ADVANCE returns). */
INTERP(advance_ret,02133u)
 lmi_newops(s,077201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);vload(s,003506);next(s);
 lmi_newops(s,065315);pdvl(s,003500);right(s);opnd(s,003634,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,065325);opnd(s,003636,PDDL);lmi_pddl_op(s,0);right(s);lmi_direct(s,033732,PDDL);lmi_pddl_op(s,twopi);next(s);
 lmi_newops(s,045006);push(s);right(s);call(s,073466);
 return EXEC_OK;
}
/* CALL PASSIVE (where INTINT returns). */
INTERP(intint_ret,02146u)
 lmi_newops(s,077624);call(s,046412);
 return EXEC_OK;
}
/* CALL CSI/A (where PASSIVE returns). */
INTERP(passive_ret,02150u)
 lmi_newops(s,077624);call(s,070113);
 return EXEC_OK;
}
/* P32/P72E STORE T1TOT2; DSU BPL 60MIN P32/P72E, back to the STORE while
 * the difference is positive; DLOAD T2TOT3. P32/P72F STORE T2TOT3; DSU BPL
 * 60MIN P32/P72F likewise; EXIT. Each taken BPL ends at its label's
 * NEWOPS, where the loop goes on. */
static void p32f_body(apollo_overlay_state *s){
 for(;;){
  store(s,002255);
  lmi_newops(s,051025);lmi_direct(s,032422,DSU);lmi_dsu(s,sixtymin);right(s);
  if(!bpl(s,072166))break;
 }
 next(s);
 lmi_newops(s,077776);lmi_exit(s);
}
static void p32e_body(apollo_overlay_state *s){
 for(;;){
  store(s,002253);
  lmi_newops(s,051025);lmi_direct(s,032422,DSU);lmi_dsu(s,sixtymin);right(s);
  if(!bpl(s,072160))break;
 }
 next(s);
 lmi_newops(s,077745);dload(s,002255);next(s);
}
/* P32/P72D DLOAD T1TOT2; on into P32/P72E and P32/P72F. */
static void p32d_body(apollo_overlay_state *s){
 lmi_newops(s,077745);dload(s,002253);next(s);
 p32e_body(s);p32f_body(s);
}
/* P32/P72C BON SET FINALFLG P32/P72D UPDATFLG; on into P32/P72D (where
 * CSI/A returns). */
INTERP(p32c,02152u)
 lmi_newops(s,043014);
 if(sw(s,io,001311))lmi_swbranch(s,072156);
 else{right(s);if(sw(s,io,000470))return EXEC_INVALID_ACTION;next(s);}
 p32d_body(s);
 return EXEC_OK;
}
INTERP(p32d,02156u)
 p32d_body(s);
 return EXEC_OK;
}
INTERP(p32e,02160u)
 p32e_body(s);p32f_body(s);
 return EXEC_OK;
}
INTERP(p32f,02166u)
 p32f_body(s);
 return EXEC_OK;
}
/* VLOAD CALL DELVEET1 S32/33.1 (after the INTPRET at 2175). */
INTERP(vnpooh_ret_i,02176u)
 lmi_newops(s,045175);vload(s,002267);right(s);call(s,071100);
 return EXEC_OK;
}
/* STOVL DELVEET1 RACT2; STOVL RACT1 DELVEET2; AXT,1 CALL VN0682 DISDVLVC
 * (where S32/33.1 returns). */
INTERP(s3233_ret,02201u)
 stovl(s,026267,002311);
 stovl(s,026303,002275);
 lmi_newops(s,045170);lmi_misc(s,001522,AXT);lmi_axt(s);right(s);call(s,072370);
 return EXEC_OK;
}
/* DLOAD TTPI; STCALL TTPI0 VN1645 (where DISDVLVC returns). */
INTERP(disdvlvc_ret,02210u)
 lmi_newops(s,077745);dload(s,003636);next(s);
 stcall(s,037640,073606);
 return EXEC_OK;
}
/* GOTO P32/P72B (where VN1645 returns). */
INTERP(loop,02214u)
 lmi_newops(s,077650);lmi_misc(s,072131,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
 {LM_P32_B35(02036),almxita},{LM_P32_B35(02040),almxit},{LM_P32_B35(02063),tcsi_check},{LM_P32_B35(02072),precset_ret},
 {LM_P32_B35(02106),timerad_ret},{LM_P32_B35(02113),vn0655},{LM_P32_B35(02125),tcsi_tig},{LM_P32_B35(02131),p32b},
 {LM_P32_B35(02133),advance_ret},{LM_P32_B35(02146),intint_ret},{LM_P32_B35(02150),passive_ret},{LM_P32_B35(02152),p32c},
 {LM_P32_B35(02156),p32d},{LM_P32_B35(02160),p32e},{LM_P32_B35(02166),p32f},{LM_P32_B35(02176),vnpooh_ret_i},
 {LM_P32_B35(02201),s3233_ret},{LM_P32_B35(02210),disdvlvc_ret},{LM_P32_B35(02214),loop},
};
const agc_routine_table lm_p32_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

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
/* P32 TC AVFLAGA; TC P32STRT (where AVFLAGA returns); P72 TC AVFLAGP. */
ROUTINE(p32){ENTRY(02027u);tc(s,02030u,AVFLAGA);return EXEC_OK;}
ROUTINE(avflaga_ret){ENTRY(02030u);tc(s,02031u,02032u);return EXEC_OK;}
ROUTINE(p72){ENTRY(02031u);tc(s,02032u,AVFLAGP);return EXEC_OK;}
/* P32STRT EXTEND; DCA P30ZERO; DXCH CENTANG; TC P32/P72A. */
ROUTINE(p32strt){ENTRY(02032u);ai_dcaf(s,p30zero);ai_dxch(s,CENTANG);tc(s,02036u,02053u);return EXEC_OK;}
/* After ALMXIT's EXIT: CA MPAC; TC VARALARM; CAF V05N09; TC BANKCALL
 * (CADR GOFLASH) (where VARALARM returns); GOFLASH's terminate and
 * recycle returns TC GOTOPOOH and TC -4. */
ROUTINE(almxit_exit){ENTRY(02044u);ai_ca(s,MPAC);tc(s,02046u,VARALARM);return EXEC_OK;}
ROUTINE(varalarm_ret){ENTRY(02046u);ai_caf(s,K_V05N09);tc(s,02050u,BANKCALL);return EXEC_OK;}
ROUTINE(flash_terminate){ENTRY(02051u);tc(s,02052u,GOTOPOOH);return EXEC_OK;}
ROUTINE(flash_recycle){ENTRY(02052u);tc(s,02053u,02046u);return EXEC_OK;}
/* P32/P72A TC P20FLGON; CAF P30ZERO; TS NN +1; TS TCSI; TS TCSI +1, on
 * into VN0611 (where P20FLGON returns). */
ROUTINE(p32a){ENTRY(02053u);tc(s,02054u,P20FLGON);return EXEC_OK;}
ROUTINE(p20_ret){ENTRY(02054u);ai_caf(s,K_P30ZERO_HI);(void)ai_ts(s,NN1);(void)ai_ts(s,TCSI);(void)ai_ts(s,TCSI+1u);W[5]=02060u;return EXEC_OK;}
/* VN0611 CAF V06N11; TC VNPOOH; TC INTPRET (where VNPOOH returns). */
ROUTINE(vn0611){ENTRY(02060u);ai_caf(s,K_V06N11);tc(s,02062u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret){ENTRY(02062u);tc(s,02063u,INTPRET);return EXEC_OK;}
/* After the EXIT at 2111: TC VN0611. */
ROUTINE(tcsi_exit){ENTRY(02112u);tc(s,02113u,02060u);return EXEC_OK;}
/* After VN0655's EXIT: CAF V06N55; TC BANKCALL (CADR GOFLASH); its
 * returns TC GOTOPOOH, TC +2 and TC -5; CAF V06N37; TC VNPOOH; TC
 * INTPRET. */
ROUTINE(vn0655_exit){ENTRY(02114u);ai_caf(s,K_V06N55);tc(s,02116u,BANKCALL);return EXEC_OK;}
ROUTINE(flash_terminate2){ENTRY(02117u);tc(s,02120u,GOTOPOOH);return EXEC_OK;}
ROUTINE(flash_proceed2){ENTRY(02120u);tc(s,02121u,02122u);return EXEC_OK;}
ROUTINE(flash_recycle2){ENTRY(02121u);tc(s,02122u,02114u);return EXEC_OK;}
ROUTINE(v06n37){ENTRY(02122u);ai_caf(s,K_V06N37);tc(s,02124u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret2){ENTRY(02124u);tc(s,02125u,INTPRET);return EXEC_OK;}
/* After the EXIT at 2172: CAF V06N75; TC VNPOOH; TC INTPRET. */
ROUTINE(v06n75){ENTRY(02173u);ai_caf(s,K_V06N75);tc(s,02175u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret3){ENTRY(02175u);tc(s,02176u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
 {LM_P32_B35(02027),p32},{LM_P32_B35(02030),avflaga_ret},{LM_P32_B35(02031),p72},{LM_P32_B35(02032),p32strt},
 {LM_P32_B35(02044),almxit_exit},{LM_P32_B35(02046),varalarm_ret},{LM_P32_B35(02051),flash_terminate},
 {LM_P32_B35(02052),flash_recycle},{LM_P32_B35(02053),p32a},{LM_P32_B35(02054),p20_ret},{LM_P32_B35(02060),vn0611},
 {LM_P32_B35(02062),vnpooh_ret},{LM_P32_B35(02112),tcsi_exit},{LM_P32_B35(02114),vn0655_exit},
 {LM_P32_B35(02117),flash_terminate2},{LM_P32_B35(02120),flash_proceed2},{LM_P32_B35(02121),flash_recycle2},
 {LM_P32_B35(02122),v06n37},{LM_P32_B35(02124),vnpooh_ret2},{LM_P32_B35(02173),v06n75},{LM_P32_B35(02175),vnpooh_ret3},
};
const agc_routine_table lm_p32_basic_routines={basic,(uint16_t)(sizeof basic/sizeof *basic)};
