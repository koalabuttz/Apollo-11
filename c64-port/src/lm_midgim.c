/* SPDX-License-Identifier: MIT */
/* Native Luminary099 midcourse display subroutines
 * (Luminary099/P34-35_P74-75.agc): HALFREV, GET+MGA and GET.LVC (bank 6
 * 03715-03752) and SELECTMU (bank 10 02000-02027). Every interpretive pair
 * is compiled: its original order and operand words appear below only as
 * the literal bookkeeping arguments of the lm_interp_steps and
 * lm_interp_orders paths, and each runtime branch follows the outcome those
 * paths return. ARCSIN is the original bank-0 body (src/inverse_trig.c), MXV
 * the compiled matrix phases (src/matrix_native.c) on the work-area matrix,
 * MUTABLE (bank 4) is read from the certified fixed-data table and HALFREV
 * appears as a literal. */
#include "lm_midgim.h"
#include "lm_interp_steps.h"
#include "agc_image_ops.h"
#include "inverse_trig.h"
#include "matrix_native.h"
#ifdef LM_MIDGIM_TRACE
void lm_midgim_trace(const apollo_overlay_state *);
#define TRACE(s) lm_midgim_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,DAD=06775,DOT=07331,VXV=07460,MXV=07334,GSHIFT=07621,COMP=07670,UNIT=03023,
 ARCSIN=03610,RVQ=03274,AXT=02344,AXC=02351,SXA=02364,BPLBMN=02474,BZEGOTO=02467,SWITCH=02516};

/* The certified fixed words at the FBANK and ADDRWD address formation left. */
static uint8_t fixed_operand(const apollo_overlay_state *s,agc_word *k,unsigned n){
 unsigned i,a=W[0116],bank=W[4]>>10;
 if(a<02000u||a+n>04000u||bank>=030u)return 0;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(LM,(uint16_t)(bank*1024u+a-02000u+i),&k[i]))return 0;
 return 1;
}
static void next(apollo_overlay_state *s){lmi_danzig_next(s);TRACE(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
static uint8_t unit(apollo_overlay_state *s){lmi_unary(s,UNIT);return lmi_unit(s);}
static void right_rvq(apollo_overlay_state *s){right(s);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void axt(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,AXT);lmi_axt(s);}
static void axc(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,AXC);lmi_axc(s);}
static void sxa(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,SXA);lmi_sxa(s);}

uint8_t lmg_getmga(apollo_overlay_state *s,p66_bias_io *io){
 /* HALFREV 2DEC 1 B-1 (6,3715). */
 static const agc_word HALFREV[2]={020000,0};
 enum inverse_trig_status t;
 (void)io;
 /* GET+MGA VLOAD UNIT (the load pushes up); UNIT; DOT SL1 REFSMMAT +6. */
 lmi_newops(s,053575);opnd(s,077656,VLOAD);lmi_vload(s,0);right(s);if(unit(s))return LMG_ABORTED;next(s);
 lmi_newops(s,077656);if(unit(s))return LMG_ABORTED;next(s);
 lmi_newops(s,072441);opnd(s,001742,DOT);lmi_dot(s,0);right(s);shorts(s);next(s);
 /* ARCSIN BPL SETMGA. */
 lmi_newops(s,051136);lmi_unary(s,ARCSIN);
 t=inverse_trig_prepare(s,INVERSE_TRIG_LM,1);
 if(t==INVERSE_TRIG_OK)t=inverse_trig_root(s,INVERSE_TRIG_LM);
 if(t==INVERSE_TRIG_OK)t=inverse_trig_polynomial(s,INVERSE_TRIG_LM);
 if(t==INVERSE_TRIG_OK)t=inverse_trig_finish(s,INVERSE_TRIG_LM);
 if(t==INVERSE_TRIG_ALARM_1301)return LMG_ARCSIN1301;
 if(t!=INVERSE_TRIG_DONE)return LMG_FAILED;
 right(s);lmi_misc(s,015730,BPLBMN);
 if(lmi_bplbmn(s)){lmi_goto(s);TRACE(s);}
 else{
  next(s);
  /* DAD DAD HALFREV HALFREV. */
  lmi_newops(s,043215);opnd(s,015716,DAD);lmi_dad(s,HALFREV);right(s);opnd(s,015716,DAD);lmi_dad(s,HALFREV);next(s);
 }
 /* SETMGA STORE +MGA; CLR RVQ MGLVFLAG. */
 store(s,002253);
 lmi_newops(s,043414);if(sw(s,io,002675))return LMG_FAILED;right_rvq(s);
 return LMG_RETURNED;
}
uint8_t lmg_getlvc(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[18];unsigned i;
 /* GET.LVC VLOAD UNIT RINIT; VCOMP; STORE 18D; VXV UNIT VINIT; STORE 12D;
  * VXV UNIT 18D; STOVL 6D 0D. */
 lmi_newops(s,053575);opnd(s,002323,VLOAD);lmi_vload(s,0);right(s);if(unit(s))return LMG_ABORTED;next(s);
 lmi_newops(s,077676);lmi_unary(s,COMP);lmi_comp(s);next(s);
 store(s,000023);
 lmi_newops(s,053435);opnd(s,002331,VXV);lmi_vxv(s,0);right(s);if(unit(s))return LMG_ABORTED;next(s);
 store(s,000015);
 lmi_newops(s,053435);opnd(s,000023,VXV);lmi_vxv(s,0);right(s);if(unit(s))return LMG_ABORTED;next(s);
 stovl(s,024007,000001);
 /* MXV VSL1 6D (the work-area matrix); STORE DELVLVC; SET RVQ MGLVFLAG. */
 lmi_newops(s,076521);lmi_direct(s,000007,MXV);
 if(W[0116]>=2048u-18u)return LMG_FAILED;
 for(i=0;i<18u;++i)k[i]=W[W[0116]+i];
 /* matrix_native_phase's domain starts at ADDRWD 01000; the work-area
  * operand is below it. ADDRWD only moves by small additions in the phases
  * (and phases 1 and 2 leave the sum in A), so run them on ADDRWD+01000 and
  * take the bias back out after each one. */
 for(i=0;i<4u;++i){
  uint16_t bias=W[0116]<01000u?01000u:0u;W[0116]+=bias;
  if(!matrix_native_phase(s,LM,0,k,(uint8_t)i)){W[0116]-=bias;return LMG_FAILED;}
  W[0116]-=bias;if(i==1u||i==2u)W[0]-=bias;
 }
 right(s);shorts(s);next(s);
 store(s,003434);
 lmi_newops(s,043414);if(sw(s,io,002475))return LMG_FAILED;right_rvq(s);
 return LMG_RETURNED;
}
uint8_t lmg_selectmu(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[2];
 /* SELECTMU AXC,1 AXT,2 2D 0D; BOFF CMOONFLG SETMUER; AXC,1 AXT,2 10D 2D. */
 lmi_newops(s,077160);axc(s,000002);right(s);axt(s,000000);next(s);
 lmi_newops(s,077614);
 if(sw(s,io,004343)){lmi_swbranch(s,020011);TRACE(s);}
 else{
  next(s);
  lmi_newops(s,077160);axc(s,000012);right(s);axt(s,000002);next(s);
 }
 /* SETMUER DLOAD* SXA,1 MUTABLE +4,1 RTX1; STODL* RTSR1/MU MUTABLE -2,1
  * (DODLOAD*: CAF DLOAD*, TS CYR, INDEX). */
 lmi_newops(s,066143);lmi_indexed(s,010011,DLOAD);if(!fixed_operand(s,k,2))return LMG_FAILED;lmi_dload_op(s,k);right(s);sxa(s,002776);next(s);
 lmi_store_code(s,022317);
 if(W[5]!=06144u)return LMG_FAILED;
 ai_caf(s,040015);(void)ai_ts(s,020);lmi_indexed(s,010003,DLOAD);if(!fixed_operand(s,k,2))return LMG_FAILED;lmi_dload_op(s,k);next(s);
 /* BOFF SR CMOONFLG RTRNMU 6D; RTRNMU STORE RTMU. */
 lmi_newops(s,054214);
 if(sw(s,io,004343)){lmi_swbranch(s,020022);TRACE(s);}
 else{right(s);lmi_direct(s,020607,GSHIFT);lmi_gshift(s);next(s);}
 store(s,002321);
 /* SXA,2 CLEAR RTX2 FINALFLG; GOTO VN1645. */
 lmi_newops(s,043134);sxa(s,002777);right(s);if(sw(s,io,001271))return LMG_FAILED;next(s);
 lmi_newops(s,077650);lmi_misc(s,073606,BZEGOTO);lmi_goto_order(s);TRACE(s);
 return LMG_RETURNED;
}

uint8_t lm_midgim_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LMG_GETMGA:return lmg_getmga(s,io);
 case LMG_GETLVC:return lmg_getlvc(s,io);
 case LMG_SELECTMU:return lmg_selectmu(s,io);
 default:return LMG_FAILED;
 }
}

/* The Luminary callers' returns: CALL GET+MGA (P35 35,3632), CALL or
 * STCALL GET.LVC (P34 35,2510, P35 27,2332 and 27,2401). SELECTMU ends at
 * VN1645 whatever QPRET holds. */
static const agc_word q_getmga[1]={073634},q_getlvc[3]={072512,056334,056403};
static uint8_t member(agc_word q,const agc_word *list,unsigned n){unsigned i;for(i=0;i<n;++i)if(list[i]==q)return 1;return 0;}
/* Canonical image with coherent banks (BIT15 clear for banks 6 and 10),
 * VAC and PUSHLOC within it (at least 6D for GET+MGA's pushed vector), MODE
 * one of the three. */
uint8_t lm_midgim_entry(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,vac=W[0120],pd=W[0166];agc_word q,mode=W[0163];
 if(W[5]!=06070u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6]||W[0115]||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 q=W[vac+052u];
 if(W[4]==014000u&&W[0164]==03717u)return member(q,q_getmga,1)&&pd>=vac+6u?LMG_GETMGA:0u;
 if(W[4]==014000u&&W[0164]==03733u)return member(q,q_getlvc,3)?LMG_GETLVC:0u;
 if(W[4]==020000u&&W[0164]==02000u)return LMG_SELECTMU;
 return 0;
}
uint8_t lm_midgim(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;
 if(!s||!io||!(phase=lm_midgim_entry(s,io)))return 0;
 phase=lm_midgim_step(s,io,phase);
 return phase==LMG_RETURNED?1u:phase==LMG_ABORTED?2u:phase==LMG_ARCSIN1301?3u:0u;
}
