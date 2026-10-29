/* SPDX-License-Identifier: MIT */
/* Native Luminary099 VN1645 computation head
 * (Luminary099/P34-35_P74-75.agc, bank 35 03606-03634) and P3XORP7X
 * (35,3704-3711). Every interpretive pair is compiled: its original order
 * and operand words appear below only as the literal bookkeeping arguments
 * of the lm_interp_steps and lm_interp_orders paths, and each runtime branch
 * follows the outcome those paths return. P3XORP7X's basic instructions are
 * the agc_image_ops forms. DP-.01 (35,3741) and HIGH9 (07744) appear as
 * literals. GET+MGA is the
 * lm-midgim step. */
#include "lm_vn1645.h"
#include "lm_interp_steps.h"
#include "agc_image_ops.h"
#ifdef LM_VN1645_TRACE
void lm_vn1645_trace(const apollo_overlay_state *);
#define TRACE(s) lm_vn1645_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,DAD=06775,PUSH=03247,EXIT=06743,CALLITA=02507,SWITCH=02516};

static void next(apollo_overlay_state *s){lmi_danzig_next(s);TRACE(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
/* DP-.01 OCT 77777 OCT 61337 (35,3741), at the ADDRWD and FBANK address
 * formation leaves. */
static void dpm01_words(agc_word w[2]){w[0]=077777;w[1]=061337;}
static uint8_t dpm01(const apollo_overlay_state *s){return W[0116]==03741u&&W[4]==072000u;}
static uint8_t dload_dpm01(apollo_overlay_state *s){
 agc_word k[2];lmi_direct(s,033742,DLOAD);if(!dpm01(s))return 0;dpm01_words(k);lmi_dload_op(s,k);return 1;
}

uint8_t lvn_vn1645(apollo_overlay_state *s,p66_bias_io *io){
 agc_word ret,k[2];
 /* VN1645 STQ DLOAD SUBEXIT DP-.01; STORE +MGA. */
 lmi_newops(s,071220);lmi_misc(s,003470,CALLITA);lmi_ita(s);right(s);if(!dload_dpm01(s))return LMG_FAILED;next(s);
 store(s,002253);
 /* BOFF DLOAD FINALFLG GET45 DP-.01. */
 lmi_newops(s,071214);
 if(sw(s,io,001351)){lmi_swbranch(s,073634);TRACE(s);return LVN_GET45;}
 right(s);if(!dload_dpm01(s))return LMG_FAILED;next(s);
 /* DAD DP-.01; STORE +MGA. */
 lmi_newops(s,077615);lmi_direct(s,033742,DAD);if(!dpm01(s))return LMG_FAILED;dpm01_words(k);lmi_dad(s,k);next(s);
 store(s,002253);
 /* BOFF EXIT REFSMFLG GET45. */
 lmi_newops(s,077414);
 if(sw(s,io,001742)){lmi_swbranch(s,073634);TRACE(s);return LVN_GET45;}
 right(s);lmi_unary(s,EXIT);lmi_exit(s);
 /* TC P3XORP7X; P3XORP7X CAF HIGH9; MASK MODREG; EXTEND; BZF +2; INCR Q;
  * RETURN. */
 if(W[5]!=03624u)return LMG_FAILED;
 ai_tc(s,03625);ai_caf(s,077700); /* HIGH9 */
 ai_mask(s,01011);
 if(!ai_bzf(s))ai_incr(s,2);
 ret=W[2];ai_tc(s,03712);
 /* 35,3625 TC +2 (P3X), or 35,3626 TC GET45 +1 (P7X). */
 if(ret==03626u){ai_tc(s,03627);W[5]=03635;return LMG_RETURNED;}
 if(ret!=03625u)return LMG_FAILED;
 ai_tc(s,03626);
 /* 35,3627 TC INTPRET. */
 ai_tc(s,03630);W[5]=06037;lmi_intpret(s,io);TRACE(s);
 return LVN_MGA;
}
uint8_t lvn_mga(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* VLOAD PUSH DELVSIN; CALL GET+MGA. */
 lmi_newops(s,041575);lmi_direct(s,003656,VLOAD);lmi_vload(s,0);right(s);lmi_unary(s,PUSH);lmi_push(s);next(s);
 lmi_newops(s,077624);lmi_misc(s,015717,CALLITA);lmi_call(s);lmi_goto(s);TRACE(s);
 return LMG_GETMGA;
}
uint8_t lvn_get45(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* GET45 EXIT (to TC COMPTGO, not ported). */
 lmi_newops(s,077776);lmi_exit(s);
 return W[5]==03635u?LMG_RETURNED:LMG_FAILED;
}

uint8_t lvn_remap(const apollo_overlay_state *s,uint8_t next){
 if(next==LMG_RETURNED&&W[4]==072000u&&W[0164]==03634u)return LVN_GET45;
 return next;
}
uint8_t lm_vn1645_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LVN_VN1645:return lvn_vn1645(s,io);
 case LVN_MGA:return lvn_mga(s,io);
 case LMG_GETMGA:return lvn_remap(s,lmg_getmga(s,io));
 case LVN_GET45:return lvn_get45(s,io);
 default:return LMG_FAILED;
 }
}

/* Canonical image at NEWOPS of VN1645 with coherent banks (FBANK 35,
 * INTBIT15 set, superbank bits clear), VAC and PUSHLOC within it, MODE one
 * of the three. Any QPRET (STQ saves it in SUBEXIT). */
uint8_t lm_vn1645_entry(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(W[5]!=06070u||W[4]!=072000u||W[0164]!=03606u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6]||W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return LVN_VN1645;
}
uint8_t lm_vn1645(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;unsigned guard;
 if(!s||!io||!(phase=lm_vn1645_entry(s,io)))return 0;
 for(guard=0;guard<8u&&!LM_VN1645_DONE(phase);++guard)phase=lm_vn1645_step(s,io,phase);
 return phase==LMG_RETURNED?1u:phase==LMG_ABORTED?2u:phase==LMG_ARCSIN1301?3u:0u;
}
