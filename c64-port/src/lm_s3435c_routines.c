/* SPDX-License-Identifier: MIT */
/* Luminary099 S34/35.2's integration head (P34-35_P74-75.agc, bank 35
 * 03306-03326) and LOMAT tail (03367-03376), and S34/35.5 (03522-03605), as
 * routines of the control-transfer ABI; see include/lm_s3435c_routines.h
 * and docs/LM-S3435C.md. Every interpretive pair is compiled with the shared
 * bodies (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order
 * and operand words appear below only as the literal bookkeeping arguments of
 * those paths, and each runtime branch follows the outcome they return. Each
 * basic instruction is one ai_* register effect on the image, in source
 * order. MXV runs the compiled matrix phases on the work-area matrix LOMAT
 * leaves at 0D (src/lm_work_matrix.c); ZEROVECS is read from the certified
 * fixed data and TWOPI is a literal. */
#include "lm_s3435c_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "lm_work_matrix.h"
#include "fixed_data.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 S34/35.2, group 2
 * its LOMAT return, group 3 FLAGON, FLAGOFF's S34/35.4 return, VNPOOH's
 * INTPRET return and the basic routines. The runs from S34/35.2's INITVEL
 * return (on through LOMAT and MXV), the target check's INTPRET return (on
 * through S34/35.3, LOMAT, VXM and INTINT) and S34/35.5, NOCHG and FLAGOFF
 * (on through S34/35.4 and NOVRWRT's MXV) outgrow C64 RAM, so their cases
 * are host-only. Host builds register all. */
#ifndef LM_S3435C_C64_GROUP
#define HAS_S2 1
#define HAS_S2_INITVEL 1
#define HAS_S2_LOMAT 1
#define HAS_S5 1
#define HAS_FLAGON 1
#define HAS_NTARG 1
#define HAS_NOCHG 1
#define HAS_FLAGOFF_EXIT 1
#define HAS_S5_RETURN 1
#define HAS_BASIC 1
#else
#define HAS_S2 (LM_S3435C_C64_GROUP==1)
#define HAS_S2_INITVEL 0
#define HAS_S2_LOMAT (LM_S3435C_C64_GROUP==2)
#define HAS_S5 0
#define HAS_FLAGON (LM_S3435C_C64_GROUP==3)
#define HAS_NTARG 0
#define HAS_NOCHG 0
#define HAS_FLAGOFF_EXIT (LM_S3435C_C64_GROUP==3)
#define HAS_S5_RETURN (LM_S3435C_C64_GROUP==3)
#define HAS_BASIC (LM_S3435C_C64_GROUP==3)
#endif
#if defined __clang__ && defined LM_S3435C_C64_GROUP
/* A C64 group build compiles only its routines, so helpers that only the
 * other groups' routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,PDDL=06523,DLOAD=06052,PUSH=03247,CALLITA=02507,RTBBHIZ=02447,BZEGOTO=02467,
 SWITCH=02516};
/* Erasable (EB-window addresses as the source names them). */
#define Q_REG 2u
#define EBANK_REG 3u
#define GDT2 01236u
#define DELVLVC 01433u
/* Fixed constants (35,3727, 35,3726, 2,1016, 2,0756, 35,3731-3732). */
#define K_V06N81 01521u
#define K_V06N59 01473u
#define K_EBANK7 03400u
#define K_FIVE 05u
#define K_TWOPI_HI 014441u
#define K_TWOPI_LO 037325u
/* Destinations outside the file. */
#define BANKCALL 04616u
#define UPFLAG 05504u
#define GOTOPOOH 06001u
#define INTPRET 06037u
#define VNPOOH 03712u   /* 35,3712 (lm-p34) */

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void stq(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_ita(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
/* GOTO label (fixed CADR), and GOTO through an erasable CADR. */
static void go(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);lmi_goto_order(s);}
static void go_any(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);}
static void exit_order(apollo_overlay_state *s){lmi_exit(s);}
/* The certified fixed words that ADDRWD names after address formation (a
 * fixed-fixed address, or the FBANK the formation switched to). Banks 30-37
 * are the plain ones: the entry contract admits only SUPERBNK settings
 * below 4. */
static uint8_t fixed_operand(const apollo_overlay_state *s,agc_word *k,unsigned n){
 unsigned i,a=W[0116],bank;
 if(a>=04000u&&a+n<=010000u){bank=a>>10;a&=01777u;}
 else if(a>=02000u&&a+n<=04000u){bank=W[4]>>10;a-=02000u;}
 else return 0;
 if(bank>=040u)return 0;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(APOLLO_FIXED_LM,(uint16_t)(bank*1024u+a+i),&k[i]))return 0;
 return 1;
}

/* TWOPI 2DEC 6.283185307 B-4 (35,3731-3732, in P34-35_P74-75's own constants,
 * outside the certified fixed-data spans): the words, at the ADDRWD and
 * FBANK address formation leaves. */
static uint8_t twopi(const apollo_overlay_state *s,agc_word k[2]){
 if(W[0116]!=03731u||W[4]!=072000u)return 0;
 k[0]=K_TWOPI_HI;k[1]=K_TWOPI_LO;return 1;
}

/* Canonical image at NEWOPS with LOC `loc` in FB 35, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 32D into it, MODE one of the three, IRQ
 * 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=072000u||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS or, after EXIT, at the basic
 * word that follows: the trampoline continues there or stops outside the
 * registered routines. */

#if HAS_S2
/* S34/35.2 STQ VLOAD SUBEXIT VPASS3; PDVL PDDL RPASS3 INTIME; PDDL PDDL
 * TPASS4 TWOPI; PDDL BHIZ NN S3435.23; DLOAD (pushes up); DLOAD PUSH
 * ZEROVECS; S3435.23 CALL INTINT. */
INTERP(s3435_2,03306u)
 agc_word k[2];
 lmi_newops(s,077220);stq(s,003470);right(s);vload(s,003560);next(s);
 lmi_newops(s,065315);opnd(s,003552,PDVL);lmi_pdvl(s,0);right(s);opnd(s,003610,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,065325);opnd(s,003631,PDDL);lmi_pddl_op(s,0);right(s);
 lmi_direct(s,033732,PDDL);if(!twopi(s,k))return EXEC_INVALID_ACTION;lmi_pddl_op(s,k);next(s);
 lmi_newops(s,046125);opnd(s,003467,PDDL);lmi_pddl_op(s,0);right(s);lmi_misc(s,073325,RTBBHIZ);(void)ai_ccs(s,020);
 if(lmi_bhiz(s))lmi_goto(s);
 else{
  next(s);
  lmi_newops(s,077745);opnd(s,041545,DLOAD);lmi_dload_op(s,0);next(s);
  lmi_newops(s,041545);lmi_direct(s,006522,DLOAD);if(!fixed_operand(s,k,2))return EXEC_INVALID_ACTION;lmi_dload_op(s,k);
  right(s);push(s);next(s);
 }
 lmi_newops(s,077624);call(s,073466);
 return EXEC_OK;
}
#endif
#if HAS_S2_INITVEL
/* CALL LOMAT (where INITVEL returns). */
INTERP(s3435_2_initvel,03367u)
 lmi_newops(s,077624);call(s,073447);
 return EXEC_OK;
}
#endif
#if HAS_S2_LOMAT
/* VLOAD MXV DELVEET3 0D; VSL1; STCALL DELVLVC SUBEXIT (where LOMAT
 * returns). */
INTERP(s3435_2_lomat,03371u)
 lmi_newops(s,064375);vload(s,002366);right(s);if(!lm_work_matrix(s,000001,0))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,077772);shorts(s);next(s);
 lmi_store_code(s,037434);lmi_15bitadr(s,003470,CALLITA);lmi_call(s);lmi_goto_any(s);
 return EXEC_OK;
}
#endif
#if HAS_S5
/* S34/35.5 STQ BON SUBEXIT FINALFLG FLAGON; SET GOTO UPDATFLG FLAGOFF. */
INTERP(s3435_5,03522u)
 lmi_newops(s,043020);stq(s,003470);right(s);
 if(sw(s,io,001311)){lmi_swbranch(s,073531);return EXEC_OK;}
 next(s);
 lmi_newops(s,052014);if(sw(s,io,000470))return EXEC_INVALID_ACTION;right(s);go(s,073576);
 return EXEC_OK;
}
#endif
#if HAS_FLAGON
/* FLAGON CLEAR VLOAD NTARGFLG DELVLVC; STORE GDT/2; EXIT. */
INTERP(flagon,03531u)
 lmi_newops(s,077214);if(sw(s,io,003274))return EXEC_INVALID_ACTION;right(s);vload(s,003434);next(s);
 store(s,001237);
 lmi_newops(s,077776);exit_order(s);
 return EXEC_OK;
}
#endif
#if HAS_NTARG
/* BOFF CALL NTARGFLG NOCHG S34/35.3 (where the target check's INTPRET
 * returns). */
INTERP(ntarg_check,03566u)
 lmi_newops(s,045014);
 if(sw(s,io,003354)){lmi_swbranch(s,073572);return EXEC_OK;}
 right(s);call(s,073377);
 return EXEC_OK;
}
#endif
#if HAS_NOCHG
/* NOCHG CLEAR VLOAD XDELVFLG DELVEET3; STORE DELVSIN (on into FLAGOFF). */
INTERP(nochg,03572u)
 lmi_newops(s,077214);if(sw(s,io,001267))return EXEC_INVALID_ACTION;right(s);vload(s,002366);next(s);
 store(s,003656);
 return EXEC_OK;
}
#endif
#if HAS_NOCHG
/* FLAGOFF CALL S34/35.4. */
INTERP(flagoff,03576u)
 lmi_newops(s,077624);call(s,073442);
 return EXEC_OK;
}
#endif
#if HAS_FLAGOFF_EXIT
/* EXIT (where S34/35.4 returns). */
INTERP(flagoff_exit,03600u)
 lmi_newops(s,077776);exit_order(s);
 return EXEC_OK;
}
#endif
#if HAS_S5_RETURN
/* GOTO SUBEXIT (where VNPOOH's INTPRET returns). */
INTERP(s3435_5_return,03604u)
 lmi_newops(s,077650);go_any(s,003470);
 return EXEC_OK;
}
#endif

/* A C64 group registers only the routines its cases reach. */
static const agc_routine_entry interpretive[]={
#if HAS_S2
 {LM_S3435C_B35(03306),s3435_2},
#endif
#if HAS_S2_INITVEL
 {LM_S3435C_B35(03367),s3435_2_initvel},
#endif
#if HAS_S2_LOMAT
 {LM_S3435C_B35(03371),s3435_2_lomat},
#endif
#if HAS_S5
 {LM_S3435C_B35(03522),s3435_5},
#endif
#if HAS_FLAGON
 {LM_S3435C_B35(03531),flagon},
#endif
#if HAS_NTARG
 {LM_S3435C_B35(03566),ntarg_check},
#endif
#if HAS_NOCHG
 {LM_S3435C_B35(03572),nochg},
#endif
#if HAS_NOCHG
 {LM_S3435C_B35(03576),flagoff},
#endif
#if HAS_FLAGOFF_EXIT
 {LM_S3435C_B35(03600),flagoff_exit},
#endif
#if HAS_S5_RETURN
 {LM_S3435C_B35(03604),s3435_5_return},
#endif
};
const agc_routine_table lm_s3435c_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

#if HAS_BASIC
static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 35 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=072000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* FLAGON +5 3536: CAF V06N81; TC BANKCALL (CADR GOFLASH at Q). */
static void flagon_flash(apollo_overlay_state *s){ai_caf(s,K_V06N81);tc(s,03540u,BANKCALL);}
/* +2 3544: CA EBANK7; TS EBANK; ZL; CA FIVE; NTARGCHK TS Q; INDEX Q;
 * CS DELVLVC; INDEX Q; AD GDT/2; ADS L; CCS Q; TCF NTARGCHK; LXCH A;
 * EXTEND; BZF +3; TC UPFLAG (ADRES NTARGFLG follows); +3 TC INTPRET. Q
 * counts 5 down to +0, so CCS Q never takes its negative skips. */
static enum exec_status ntargchk(apollo_overlay_state *s){uint8_t c;
 ai_caf(s,K_EBANK7);(void)ai_ts(s,EBANK_REG);ai_zl(s);ai_caf(s,K_FIVE);
 do{(void)ai_ts(s,Q_REG);ai_cs(s,DELVLVC+W[Q_REG]);ai_ad(s,GDT2+W[Q_REG]);ai_ads(s,1u);c=ai_ccs(s,Q_REG);}while(c==0u);
 if(c!=1u)return EXEC_INVALID_ACTION;
 ai_lxch(s,0u);
 if(ai_bzf(s)){tc(s,03566u,INTPRET);return EXEC_OK;}
 tc(s,03564u,UPFLAG);return EXEC_OK;
}
ROUTINE(r_flagon_flash){ENTRY(03536u);flagon_flash(s);return EXEC_OK;}
/* GOFLASH's terminate, proceed and recycle returns: TC GOTOPOOH; TC +2;
 * TC FLAGON +5. */
ROUTINE(flash_terminate){ENTRY(03541u);tc(s,03542u,GOTOPOOH);return EXEC_OK;}
ROUTINE(flash_proceed){ENTRY(03542u);tc(s,03543u,03544u);return ntargchk(s);}
ROUTINE(flash_recycle){ENTRY(03543u);tc(s,03544u,03536u);flagon_flash(s);return EXEC_OK;}
ROUTINE(r_ntargchk){ENTRY(03544u);return ntargchk(s);}
/* Where UPFLAG returns: TC INTPRET. */
ROUTINE(upflag_intpret){ENTRY(03565u);tc(s,03566u,INTPRET);return EXEC_OK;}
/* After FLAGOFF's EXIT: CAF V06N59; TC VNPOOH. */
ROUTINE(flagoff_vnpooh){ENTRY(03601u);ai_caf(s,K_V06N59);tc(s,03603u,VNPOOH);return EXEC_OK;}
/* Where VNPOOH returns: TC INTPRET. */
ROUTINE(vnpooh_intpret){ENTRY(03603u);tc(s,03604u,INTPRET);return EXEC_OK;}
#endif
static const agc_routine_entry basic[]={
#if HAS_BASIC
 {LM_S3435C_B35(03536),r_flagon_flash},{LM_S3435C_B35(03541),flash_terminate},{LM_S3435C_B35(03542),flash_proceed},
 {LM_S3435C_B35(03543),flash_recycle},{LM_S3435C_B35(03544),r_ntargchk},{LM_S3435C_B35(03565),upflag_intpret},
 {LM_S3435C_B35(03601),flagoff_vnpooh},{LM_S3435C_B35(03603),vnpooh_intpret},
#else
 {0,0}
#endif
};
const agc_routine_table lm_s3435c_basic_routines={basic,(uint16_t)(HAS_BASIC?sizeof basic/sizeof *basic:0u)};
