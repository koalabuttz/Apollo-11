/* SPDX-License-Identifier: MIT */
/* Luminary099 S34/35.3 (P34-35_P74-75.agc, bank 35 03377-03423) and
 * GOINT/INTINT (03462-03521) as interpretive routines of the
 * control-transfer ABI; see include/lm_s3435b_routines.h and
 * docs/LM-S3435B.md. Every interpretive pair is compiled with the shared
 * bodies (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order
 * and operand words appear below only as the literal bookkeeping arguments of
 * those paths, and each runtime branch follows the outcome they return. VXM
 * runs the compiled matrix phases (src/matrix_native.c) on the work-area
 * matrix LOMAT leaves at 0D; DPPOSMAX and ZEROVECS are read from the
 * certified fixed data. */
#include "lm_s3435b_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "matrix_native.h"
#include "agc_image_w.h"
#define LM APOLLO_FIXED_LM
/* The routines a C64 build registers (C64 RAM): group 1 S34/35.3's LOMAT
 * return and INTINT, group 2 GOINT and INTINT with INTINT's INTSTALL return,
 * group 3 INTINT's INTEGRVS return and S34/35.3's INTINT return (where
 * RTRN can lead, on into NOVRWRT). S34/35.3 itself runs
 * on through lm-s3435's LOMAT, the LOMAT return's VXM and INTINT, which
 * together outgrow C64 RAM, so its cases are host-only. Host builds
 * register all seven. */
#ifndef LM_S3435B_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#define HAS_G3 1
#define HAS_HOST 1
#else
#define HAS_G1 (LM_S3435B_C64_GROUP==1)
#define HAS_G2 (LM_S3435B_C64_GROUP==2)
#define HAS_G3 (LM_S3435B_C64_GROUP==3)
#define HAS_HOST 0
#endif

#if defined __clang__ && defined LM_S3435B_C64_GROUP
/* A C64 group build compiles only its routines, so order helpers that only
 * the other groups' routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,PDDL=06523,DLOAD=06052,VAD=06751,MXV=07334,GSHIFT=07621,PUSH=03247,STADR=06354,
 LXA=02354,CALLITA=02507,BZEGOTO=02467,SWITCH=02516};

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,VLOAD);else lmi_diradres(s,word,VLOAD);
 lmi_vload(s,0);next(s);
}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void stq(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_ita(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
/* The certified fixed words that ADDRWD names after address formation (a
 * fixed-fixed address, or the FBANK the formation switched to). */
static uint8_t fixed_operand(const apollo_overlay_state *s,agc_word *k,unsigned n){
 unsigned i,a=W[0116],bank;
 if(a>=04000u&&a+n<=010000u){bank=a>>10;a&=01777u;}
 else if(a>=02000u&&a+n<=04000u){bank=W[4]>>10;a-=02000u;}
 else return 0;
 if(bank>=030u)return 0;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(LM,(uint16_t)(bank*1024u+a+i),&k[i]))return 0;
 return 1;
}
/* PDDL of a fixed-fixed constant (DPPOSMAX, ZEROVECS). */
static uint8_t pddl_fixed(apollo_overlay_state *s,agc_word word){
 agc_word k[2];lmi_direct(s,word,PDDL);if(!fixed_operand(s,k,2))return 0;lmi_pddl_op(s,k);return 1;
}
/* VXM of the 18-word work-area matrix that the address formation names
 * (matrix_native_phase column 1). Its domain starts at ADDRWD 01000, so the
 * phases run on ADDRWD+01000 and the bias comes back out after each (phases 1
 * and 2 leave the sum in A), as lm-midgim's GET.LVC does for MXV. */
static uint8_t vxm_work(apollo_overlay_state *s,agc_word word){
 agc_word k[18];unsigned i;
 lmi_direct(s,word,MXV+3u);
 if(W[0116]>=2048u-18u)return 0;
 for(i=0;i<18u;++i)k[i]=W[W[0116]+i];
 for(i=0;i<4u;++i){
  uint16_t bias=W[0116]<01000u?01000u:0u;W[0116]+=bias;
  if(!matrix_native_phase(s,LM,1,k,(uint8_t)i)){W[0116]-=bias;return 0;}
  W[0116]-=bias;if(i==1u||i==2u)W[0]-=bias;
 }
 return 1;
}

/* Canonical image at NEWOPS with LOC `loc` in FB 35, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 40D into it (the integration arguments
 * sit above the work the callers pushed), MODE one of the three, IRQ 0 or 1
 * and an allowed SUPERBNK. */
static uint8_t entry_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=072000u||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+050u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ROUTINE(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!entry_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every routine ends at a NEWOPS: the trampoline continues there or stops
 * outside the registered routines. */

#if HAS_HOST
/* S34/35.3 STQ CALL NORMEX LOMAT. */
ROUTINE(s3435_3,03377u)
 lmi_newops(s,045020);stq(s,003463);right(s);call(s,073447);
 return EXEC_OK;
}
#endif
#if HAS_G1
/* VLOAD VXM DELVLVC 0D; VSL1; STORE DELVEET3; VAD PDVL VACT3 RACT3;
 * PDDL PDDL TIG TPASS4; PDDL PUSH DPPOSMAX; CALL INTINT. */
ROUTINE(s3435_3_lomat,03402u)
 lmi_newops(s,061375);vload(s,003434);right(s);if(!vxm_work(s,000001))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,077772);shorts(s);next(s);
 store(s,002366);
 lmi_newops(s,063255);opnd(s,003544,VAD);lmi_vad(s,0);right(s);opnd(s,003536,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,065325);opnd(s,003442,PDDL);lmi_pddl_op(s,0);right(s);opnd(s,003631,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,041525);if(!pddl_fixed(s,006530))return EXEC_INVALID_ACTION;right(s);push(s);next(s);
 lmi_newops(s,077624);call(s,073466);
 return EXEC_OK;
}
#endif
#if HAS_G3
/* VLOAD RATT; STORE RTARG (on into NOVRWRT). */
ROUTINE(s3435_3_intint,03421u)
 lmi_newops(s,077775);vload(s,000001);next(s);
 store(s,003444);
 return EXEC_OK;
}
#endif
#if HAS_G2
/* GOINT PDDL PDDL ZEROVECS NOMTPI; PUSH PUSH (on into INTINT). */
ROUTINE(goint,03462u)
 lmi_newops(s,065325);if(!pddl_fixed(s,006522))return EXEC_INVALID_ACTION;right(s);opnd(s,002317,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,041406);push(s);right(s);push(s);next(s);
 return EXEC_OK;
}
#endif
#if HAS_G1||HAS_G2
/* INTINT STQ CALL RTRN INTSTALL. */
ROUTINE(intint,03466u)
 lmi_newops(s,045020);stq(s,003465);right(s);call(s,027414);
 return EXEC_OK;
}
#endif
#if HAS_G2
/* CLEAR DLOAD INTYPFLG (the load pushes up); BZE SET +2 INTYPFLG; DLOAD
 * STADR (the load pushes up); STODL TDEC1 (pushes up); SET LXA,2 MOONFLAG
 * RTX2; BON CLEAR CMOONFLG ALLSET MOONFLAG; ALLSET STOVL TET (pushes up);
 * VSR* 0,2; STOVL RCV (pushes up); VSR* 0,2; STCALL VCV INTEGRVS. */
ROUTINE(intint_installed,03471u)
 lmi_newops(s,071214);if(sw(s,io,001673))return EXEC_INVALID_ACTION;right(s);opnd(s,043054,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,043054);lmi_misc(s,073476,BZEGOTO);
 if(!lmi_bze(s)){right(s);if(sw(s,io,001473))return EXEC_INVALID_ACTION;next(s);}
 lmi_newops(s,045545);opnd(s,063736,DLOAD);lmi_dload_op(s,0);right(s);lmi_unary(s,STADR);lmi_stadr(s,063736);
 lmi_dodload(s);lmi_diradres_pushup(s,073014,DLOAD);lmi_dload_op(s,0);next(s);
 lmi_newops(s,073014);if(sw(s,io,000063))return EXEC_INVALID_ACTION;right(s);lmi_misc(s,002777,LXA);lmi_lxa(s);next(s);
 lmi_newops(s,043014);
 if(sw(s,io,004303))lmi_swbranch(s,073507);
 else{right(s);if(sw(s,io,000263))return EXEC_INVALID_ACTION;next(s);}
 stovl(s,025517,077657);
 lmi_newops(s,077657);gshift(s,057176);next(s);
 stovl(s,025535,077657);
 lmi_newops(s,077657);gshift(s,057176);next(s);
 lmi_store_code(s,035543);lmi_15bitadr(s,027107,CALLITA);lmi_call(s);lmi_goto(s);
 return EXEC_OK;
}
#endif
#if HAS_G3
/* VLOAD GOTO RATT RTRN (to the CADR in RTRN). */
ROUTINE(intint_integrated,03517u)
 lmi_newops(s,052175);vload(s,000001);right(s);lmi_misc(s,003465,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);
 return EXEC_OK;
}
#endif
/* A C64 group registers only the routines its cases reach. */
static const agc_routine_entry entries[]={
#if HAS_HOST
 {LM_S3435B_B35(03377),s3435_3},
#endif
#if HAS_G1
 {LM_S3435B_B35(03402),s3435_3_lomat},
#endif
#if HAS_G3
 {LM_S3435B_B35(03421),s3435_3_intint},
#endif
#if HAS_G2
 {LM_S3435B_B35(03462),goint},
#endif
#if HAS_G1||HAS_G2
 {LM_S3435B_B35(03466),intint},
#endif
#if HAS_G2
 {LM_S3435B_B35(03471),intint_installed},
#endif
#if HAS_G3
 {LM_S3435B_B35(03517),intint_integrated},
#endif
};
const agc_routine_table lm_s3435b_routines={entries,(uint16_t)(sizeof entries/sizeof *entries)};
