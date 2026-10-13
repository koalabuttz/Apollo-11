/* SPDX-License-Identifier: MIT */
/* Luminary099 LAMBERT_AIMPOINT_GUIDANCE (P31 and REVN1645, bank 35
 * 02450-02544) as routines of the control-transfer ABI; see
 * include/lm_p31_routines.h and docs/LM-P31.md. Every interpretive pair is
 * compiled with the shared bodies (src/lm_interp_steps.c,
 * src/lm_interp_orders.c): its original order and operand words appear below
 * only as the literal bookkeeping arguments of those paths, and each runtime
 * branch follows the outcome they return. P30ZERO is a literal pair that
 * the source map checks against the fresh rope. Each basic instruction is one ai_* register effect on the image, in
 * source order. */
#include "lm_p31_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,PDDL=06523,DLOAD=06052,DAD=06775,SETPD=07613,PUSH=03247,ABVAL=03176,SXA=02364,
 CALLITA=02507,BZEGOTO=02467,SWITCH=02516};
/* Fixed constants (35,2025, 35,3727, 35,2026 and the P30ZERO pair at
 * 35,2417-2420). */
#define K_V06N33 01441u
#define K_V06N81 01521u
#define K_V06N42 01452u
#define K_P30ZERO_HI 0u
#define K_P30ZERO_LO 0u
/* Destinations outside the routines (P20FLGON is P32-P35_P72-P75's, VNPOOH
 * lm-p34's). */
#define P20FLGON 02361u
#define VNPOOH 03712u
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
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
/* STCALL code label (fixed CADR) to the callee's NEWOPS. */
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void sxa(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,SXA);lmi_sxa(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
/* ABVAL: 1 when it ends at POODOO1 (alarm 1302). */
static uint8_t abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);return lmi_abvalabs(s);}
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

/* CLEAR DLOAD UPDATFLG TIG; STCALL TDEC1 LEMPREC (after P31's INTPRET). */
INTERP(p31_start,02454u)
 lmi_newops(s,071214);if(sw(s,io,000670))return EXEC_INVALID_ACTION;right(s);dload(s,003442);next(s);
 stcall(s,034041,027057);
 return EXEC_OK;
}
/* VLOAD SETPD RATT 0D; STORE RTIG; STOVL RINIT VATT; STORE VTIG; STODL
 * VINIT P30ZERO; PUSH PDDL DELLT4; DAD SXA,1 TIG RTX1; STORE TPASS4; SXA,2
 * CALL RTX2 INITVEL (where LEMPREC returns). */
INTERP(lemprec_ret,02461u)
 static const agc_word k[2]={K_P30ZERO_HI,K_P30ZERO_LO};
 lmi_newops(s,040375);vload(s,000001);right(s);lmi_direct(s,000001,SETPD);lmi_setpd(s);next(s);
 store(s,003642);
 stovl(s,026323,000007);
 store(s,003650);
 lmi_store_code(s,016331);lmi_dodload(s);lmi_diradres(s,032420,DLOAD);lmi_dload_op(s,k);next(s);
 lmi_newops(s,065206);push(s);right(s);opnd(s,003452,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,066015);opnd(s,003442,DAD);lmi_dad(s,0);right(s);sxa(s,002776);next(s);
 store(s,003631);
 lmi_newops(s,045134);sxa(s,002777);right(s);call(s,022000);
 return EXEC_OK;
}
/* VLOAD PUSH DELVEET3; STORE DELVSIN; ABVAL CLEAR XDELVFLG; STCALL VGDISP
 * GET.LVC (where INITVEL returns). */
INTERP(initvel_ret,02503u)
 lmi_newops(s,041575);vload(s,002366);right(s);push(s);next(s);
 store(s,003656);
 lmi_newops(s,043046);if(abval(s))return EXEC_OK;right(s);if(sw(s,io,001267))return EXEC_INVALID_ACTION;next(s);
 stcall(s,037664,015733);
 return EXEC_OK;
}
/* VLOAD PDVL RTIG VIPRIME; CALL PERIAPO1 (where GET.LVC returns). */
INTERP(getlvc_ret,02512u)
 lmi_newops(s,063375);vload(s,003642);right(s);pdvl(s,002337);next(s);
 lmi_newops(s,077624);call(s,046316);
 return EXEC_OK;
}
/* CALL SHIFTR1 (where PERIAPO1 returns); CALL MAXCHK (where SHIFTR1
 * returns). */
INTERP(periapo_ret,02517u)
 lmi_newops(s,077624);call(s,046426);
 return EXEC_OK;
}
INTERP(shiftr1_ret,02521u)
 lmi_newops(s,077624);call(s,045636);
 return EXEC_OK;
}
/* STODL HPER 4D; CALL SHIFTR1 (where MAXCHK returns). */
INTERP(maxchk_ret,02523u)
 stodl(s,016321,000005);
 lmi_newops(s,077624);call(s,046426);
 return EXEC_OK;
}
/* CALL MAXCHK (where SHIFTR1 returns). */
INTERP(shiftr1_ret2,02527u)
 lmi_newops(s,077624);call(s,045636);
 return EXEC_OK;
}
/* STORE HAPO; EXIT (where MAXCHK returns). */
INTERP(maxchk_ret2,02531u)
 store(s,002317);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* REVN1645 SET CALL FINALFLG VN1645. */
INTERP(revn1645,02540u)
 lmi_newops(s,045014);if(sw(s,io,001071))return EXEC_INVALID_ACTION;right(s);call(s,073606);
 return EXEC_OK;
}
/* GOTO REVN1645 (where VN1645 returns). */
INTERP(loop,02543u)
 lmi_newops(s,077650);lmi_misc(s,072540,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
 {LM_P31_B35(02454),p31_start},{LM_P31_B35(02461),lemprec_ret},{LM_P31_B35(02503),initvel_ret},{LM_P31_B35(02512),getlvc_ret},
 {LM_P31_B35(02517),periapo_ret},{LM_P31_B35(02521),shiftr1_ret},{LM_P31_B35(02523),maxchk_ret},{LM_P31_B35(02527),shiftr1_ret2},
 {LM_P31_B35(02531),maxchk_ret2},{LM_P31_B35(02540),revn1645},{LM_P31_B35(02543),loop},
};
const agc_routine_table lm_p31_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

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
/* P31 TC P20FLGON. */
ROUTINE(p31){ENTRY(02450u);tc(s,02451u,P20FLGON);return EXEC_OK;}
/* CAF V06N33; TC VNPOOH (where P20FLGON returns); TC INTPRET. */
ROUTINE(p20_ret){ENTRY(02451u);ai_caf(s,K_V06N33);tc(s,02453u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret){ENTRY(02453u);tc(s,02454u,INTPRET);return EXEC_OK;}
/* After the EXIT at 2532: CAF V06N81; TC VNPOOH; CAF V06N42; TC VNPOOH;
 * TC INTPRET. */
ROUTINE(v06n81){ENTRY(02533u);ai_caf(s,K_V06N81);tc(s,02535u,VNPOOH);return EXEC_OK;}
ROUTINE(v06n42){ENTRY(02535u);ai_caf(s,K_V06N42);tc(s,02537u,VNPOOH);return EXEC_OK;}
ROUTINE(v06n42_ret){ENTRY(02537u);tc(s,02540u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
 {LM_P31_B35(02450),p31},{LM_P31_B35(02451),p20_ret},{LM_P31_B35(02453),vnpooh_ret},
 {LM_P31_B35(02533),v06n81},{LM_P31_B35(02535),v06n42},{LM_P31_B35(02537),v06n42_ret},
};
const agc_routine_table lm_p31_basic_routines={basic,(uint16_t)(sizeof basic/sizeof *basic)};
