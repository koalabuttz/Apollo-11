/* SPDX-License-Identifier: MIT */
/* Luminary099 S34/35.1, S34/35.4, NOVRWRT and LOMAT (P34-35_P74-75.agc, bank
 * 35 03274-03305 and 03424-03461) as interpretive routines of the
 * control-transfer ABI; see include/lm_s3435_routines.h and
 * docs/LM-S3435.md. Every interpretive pair is compiled with the shared
 * bodies (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order
 * and operand words appear below only as the literal bookkeeping arguments of
 * those paths, and each runtime branch follows the outcome they return. MXV
 * runs the compiled matrix phases (src/matrix_native.c) on the work-area
 * matrix LOMAT leaves at 0D. */
#include "lm_s3435_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "matrix_native.h"
#define W s->words
#define LM APOLLO_FIXED_LM
/* The routines a C64 build registers (C64 RAM): lm-s3435's own groups 1
 * (S34/35.1, LOMAT) and 2 (NOVRWRT, S34/35.4), and group 3 (NOVRWRT) for
 * lm-s3435b's S34/35.3 INTINT return. Host builds register all four. */
#ifndef LM_S3435_C64_GROUP
#define HAS_S1 1
#define HAS_NOVRWRT 1
#define HAS_S4 1
#define HAS_LOMAT 1
#else
#define HAS_S1 (LM_S3435_C64_GROUP==1)
#define HAS_NOVRWRT (LM_S3435_C64_GROUP==2||LM_S3435_C64_GROUP==3)
#define HAS_S4 (LM_S3435_C64_GROUP==2)
#define HAS_LOMAT (LM_S3435_C64_GROUP==1)
#endif

#if defined __clang__ && defined LM_S3435_C64_GROUP
/* A C64 group build compiles only its routines, so order helpers that only
 * the other groups' routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,VSU=06747,VXV=07460,MXV=07334,SETPD=07613,COMP=07670,UNIT=03023,PUSH=03247,
 RVQ=03274,CALLITA=02507,BZEGOTO=02467};

static inline void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static inline void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static inline void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static inline void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static inline void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
#if HAS_S1||HAS_LOMAT
/* Orders only S34/35.1 and LOMAT use. */
static inline void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static inline void rvq(apollo_overlay_state *s){lmi_unary(s,RVQ);lmi_rvq(s);}
static inline void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
#endif
static inline uint8_t unit(apollo_overlay_state *s){lmi_unary(s,UNIT);return lmi_unit(s);}
static inline void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static inline void vcomp(apollo_overlay_state *s){lmi_unary(s,COMP);lmi_comp(s);}
static inline void vxv(apollo_overlay_state *s,agc_word word){opnd(s,word,VXV);lmi_vxv(s,0);}
/* MXV of the 18-word work-area matrix that the address formation names.
 * matrix_native_phase's domain starts at ADDRWD 01000, so the phases run on
 * ADDRWD+01000 and the bias comes back out after each (phases 1 and 2 leave
 * the sum in A), as lm-midgim's GET.LVC does. */
#if HAS_NOVRWRT
static uint8_t mxv_work(apollo_overlay_state *s,agc_word word){
 agc_word k[18];unsigned i;
 lmi_direct(s,word,MXV);
 if(W[0116]>=2048u-18u)return 0;
 for(i=0;i<18u;++i)k[i]=W[W[0116]+i];
 for(i=0;i<4u;++i){
  uint16_t bias=W[0116]<01000u?01000u:0u;W[0116]+=bias;
  if(!matrix_native_phase(s,LM,0,k,(uint8_t)i)){W[0116]-=bias;return 0;}
  W[0116]-=bias;if(i==1u||i==2u)W[0]-=bias;
 }
 return 1;
}
#endif

/* Canonical image at NEWOPS with LOC `loc` in FB 35, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC inside it, MODE one of the three, IRQ 0 or 1 and an
 * allowed SUPERBNK. */
static uint8_t entry_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=072000u||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ROUTINE(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;\
 if(!entry_ok(s,(const p66_bias_io *)io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every routine ends at a NEWOPS (the trampoline continues there) or at
 * POODOO1 with UNIT's alarm 1302 (an unmapped point). */

#if HAS_S1
/* S34/35.1 VLOAD VSU RPASS3 RACT3; UNIT PUSH; STOVL ULOS RACT3;
 * VXV UNIT VACT3; STORE UNRM; RVQ. */
ROUTINE(s3435_1,03274u)
 lmi_newops(s,052375);vload(s,003552);right(s);opnd(s,003536,VSU);lmi_vsu(s,0);next(s);
 lmi_newops(s,041456);if(unit(s))return EXEC_OK;right(s);push(s);next(s);
 stovl(s,026311,003536);
 lmi_newops(s,053435);vxv(s,003544);right(s);if(unit(s))return EXEC_OK;next(s);
 store(s,002261);
 lmi_newops(s,077616);rvq(s);
 return EXEC_OK;
}
#endif
#if HAS_NOVRWRT
/* NOVRWRT VLOAD PUSH ULOS; VXV VCOMP UNRM; UNIT PUSH; VXV VSL1 ULOS; PDVL
 * (pushes up); PDVL MXV DELVEET3 0D; VSL1; STCALL DVLOS NORMEX (to the CADR
 * in NORMEX). */
ROUTINE(novrwrt,03424u)
 lmi_newops(s,041575);vload(s,002311);right(s);push(s);next(s);
 lmi_newops(s,057435);vxv(s,002261);right(s);vcomp(s);next(s);
 lmi_newops(s,041456);if(unit(s))return EXEC_OK;right(s);push(s);next(s);
 lmi_newops(s,076435);vxv(s,002311);right(s);shorts(s);next(s);
 lmi_newops(s,077715);opnd(s,064315,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,064315);opnd(s,002366,PDVL);lmi_pdvl(s,0);right(s);if(!mxv_work(s,000001))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,077772);shorts(s);next(s);
 lmi_store_code(s,036303);lmi_15bitadr(s,003463,CALLITA);lmi_call(s);lmi_goto_any(s);
 return EXEC_OK;
}
#endif
#if HAS_S4
/* S34/35.4 STQ SETPD NORMEX 0D; GOTO NOVRWRT. */
ROUTINE(s3435_4,03442u)
 lmi_newops(s,040220);lmi_misc(s,003463,CALLITA);lmi_ita(s);right(s);lmi_direct(s,000001,SETPD);lmi_setpd(s);
 /* SETPD's NOIBNKSW (EDOP +0): CCS NEWJOB; INCR LOC; NEWOPS. BB already
  * equals BANKSET, so DANZIG's head changes nothing CCS EDOP keeps. */
 next(s);
 lmi_newops(s,077650);lmi_misc(s,073424,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
#endif
#if HAS_LOMAT
/* LOMAT VLOAD VCOMP UNRM; STOVL 6D RACT3; UNIT VCOMP; STORE 12D; VXV VSL1
 * UNRM; STORE 0D; SETPD RVQ 18D. */
ROUTINE(lomat,03447u)
 lmi_newops(s,057575);vload(s,002261);right(s);vcomp(s);next(s);
 stovl(s,024007,003536);
 lmi_newops(s,057456);if(unit(s))return EXEC_OK;right(s);vcomp(s);next(s);
 store(s,000015);
 lmi_newops(s,076435);vxv(s,002261);right(s);shorts(s);next(s);
 store(s,000001);
 lmi_newops(s,043401);lmi_direct(s,000023,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);rvq(s);
 return EXEC_OK;
}
#endif
/* A C64 group registers only the routines its cases reach. */
static const agc_routine_entry entries[]={
#if HAS_S1
 {LM_S3435_B35(03274),s3435_1},
#endif
#if HAS_NOVRWRT
 {LM_S3435_B35(03424),novrwrt},
#endif
#if HAS_S4
 {LM_S3435_B35(03442),s3435_4},
#endif
#if HAS_LOMAT
 {LM_S3435_B35(03447),lomat},
#endif
};
const agc_routine_table lm_s3435_routines={entries,(uint16_t)(sizeof entries/sizeof *entries)};
