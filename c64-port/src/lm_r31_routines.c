/* SPDX-License-Identifier: MIT */
/* Luminary099 R31 (bank 40 03674-03712 and bank 37 03113-03407) as routines
 * of the control-transfer ABI; see include/lm_r31_routines.h and
 * docs/LM-R31.md. Every interpretive pair is compiled with the shared bodies
 * (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * those paths, and each runtime branch follows the outcome they return.
 * RTB NORMUNX1 runs the shared NORM front and UNIT phases (src/rtb_vector.c,
 * src/rtb_unit.c); ACOS is dispatched to ARCCOS (00,3612), outside these
 * routines. UNITZ, DPPOSMAX, V16N54 and the PRIO and BIT masks are literals
 * that the source map checks against the fresh rope. Each basic instruction
 * is one ai_* register effect on the image, in source order. */
#include "lm_r31_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "rtb_unit.h"
#include "agc_image_ops.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM), each group closed under the
 * routines its runs reach (runs stop at the split points 3240, 3246 and
 * 3355, tests/lm_r31_cases.c): group 1 COMPDISP (NORMUNX1), group 2 RANGE
 * and RRATE (UNIT), group 3 the return from CDU*NBSM (VXM, VPROJ, BVSU,
 * SIGN), group 4 GETRVN's first REFSMMAT rotation (VXM) and GETRVN2, group
 * 5 the second (VXM), group 6 the rest. Host builds
 * register all. Unregistered routines go unused there. */
#ifndef LM_R31_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#define HAS_G3 1
#define HAS_G4 1
#define HAS_G5 1
#define HAS_G6 1
#else
#define HAS_G1 (LM_R31_C64_GROUP==1)
#define HAS_G2 (LM_R31_C64_GROUP==2)
#define HAS_G3 (LM_R31_C64_GROUP==3)
#define HAS_G4 (LM_R31_C64_GROUP==4)
#define HAS_G5 (LM_R31_C64_GROUP==5)
#define HAS_G6 (LM_R31_C64_GROUP==6)
#endif
#if defined __clang__ && defined LM_R31_C64_GROUP
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,PDDL=06523,DLOAD=06052,DSU=07005,VSU=06747,BVSU=07036,DOT=07331,VXV=07460,VXM=07337,
 VPROJ=07425,SIGN=07655,GSHIFT=07621,SETPD=07613,PUSH=03247,UNIT=03023,ARCCOS=03612,CALLITA=02507,BZEGOTO=02467,
 BPLBMN=02474,RTBBHIZ=02447,SWITCH=02516};
/* Erasable (unswitched, and EBANK=TSTRT for EXTVBACT). */
#define FLAGWRD6 0102u
#define FLAGWRD7 0103u
#define FLAGWRD8 0104u
#define EXTVBACT 01044u
/* Fixed constants (fixed-fixed PRIO3, PRIO5, PRIO22, BIT5, BIT12,
 * AVEGFBIT, SURFFBIT and MUNFLBIT; V16N54 at 37,3407; UNITZ at 23,2513 and
 * DPPOSMAX at 23,2527). */
#define K_PRIO3 03000u
#define K_PRIO5 05000u
#define K_PRIO22 022000u
#define K_BIT5 020u
#define K_BIT12 04000u
#define K_AVEGFBIT 020u
#define K_SURFFBIT 0200u
#define K_MUNFLBIT 0200u
#define K_V16N54 04066u
#define K_UNITZ_0 0u
#define K_UNITZ_1 0u
#define K_UNITZ_2 0u
#define K_UNITZ_3 0u
#define K_UNITZ_4 020000u
#define K_UNITZ_5 0u
#define K_DPPOSMAX_HI 037777u
#define K_DPPOSMAX_LO 037777u
/* Destinations outside the routines. */
#define FINDVAC 05105u
#define FIXDELAY 05221u
#define NOVAC 05072u
#define TASKOVER 05261u
#define BANKCALL 04616u
#define B5OFF 05563u
#define PRIOCHNG 05146u
#define ENDEXT 05472u
#define INTPRET 06037u
#define FB37 076000u
#define FB30 060000u

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void vload(apollo_overlay_state *s,agc_word word){opnd(s,word,VLOAD);lmi_vload(s,0);}
static void pdvl(apollo_overlay_state *s,agc_word word,const agc_word *k){opnd(s,word,PDVL);lmi_pdvl(s,k);}
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);lmi_dload_op(s,0);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);lmi_vload(s,0);next(s);
}
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void rtb(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);}
static void goto_(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BZEGOTO);lmi_goto_order(s);}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static void setpd(apollo_overlay_state *s,agc_word word){lmi_direct(s,word,SETPD);lmi_setpd(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void vxm(apollo_overlay_state *s,agc_word word){opnd(s,word,VXM);lmi_vxm(s,0);}
static const agc_word unitz[6]={K_UNITZ_0,K_UNITZ_1,K_UNITZ_2,K_UNITZ_3,K_UNITZ_4,K_UNITZ_5},
 dpposmax[2]={K_DPPOSMAX_HI,K_DPPOSMAX_LO};

/* Canonical image at NEWOPS with LOC `loc` in FB 37, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 32D into it, MODE one of the three, IRQ
 * 0 or 1 and an allowed SUPERBNK below bank 40's. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=FB37||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS, at ARCCOS, at POODOO1 or
 * UNIT's 1302 exit, after RTB at the routine it names, or after EXIT at the
 * basic word that follows: the trampoline continues there or stops outside
 * the registered routines. */

/* V83CALL's RTB LOADTIME (after its TC INTPRET). */
INTERP(v83_rtb,03132u)
 lmi_newops(s,077634);rtb(s,021573);
 return EXEC_OK;
}
/* STCALL TDEC1 LEMPREC (where LOADTIME returns). */
INTERP(v83_lemprec,03134u)
 stcall(s,034041,027057);
 return EXEC_OK;
}
/* VLOAD RATT1; STOVL BASETHP VATT1; STODL BASETHV TAT (where LEMPREC
 * returns); on into DOCMBASE. */
INTERP(lemprec_ret,03136u)
 lmi_newops(s,077775);vload(s,000017);next(s);
 stovl(s,026170,000025);
 stodl(s,016105,000015);
 return EXEC_OK;
}
/* DOCMBASE STORE BASETIME; STCALL TDEC1 CSMPREC. */
INTERP(docmbase,03144u)
 store(s,002114);
 stcall(s,034041,027043);
 return EXEC_OK;
}
/* VLOAD RATT1; STOVL BASEOTP VATT1; STORE BASEOTV; EXIT (where CSMPREC
 * returns) into REV83. */
INTERP(csmprec_ret,03147u)
 lmi_newops(s,077775);vload(s,000017);next(s);
 stovl(s,026140,000025);
 store(s,002120);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* REV83's RTB LOADTIME (after its TC INTPRET). */
INTERP(rev83_rtb,03166u)
 lmi_newops(s,077634);rtb(s,021573);
 return EXEC_OK;
}
/* STCALL TDEC1 INTSTALL (where LOADTIME returns). */
INTERP(rev83_intstall,03170u)
 stcall(s,034041,027414);
 return EXEC_OK;
}
/* VLOAD CLEAR BASETHP MOONFLAG; STOVL RCV BASETHV; STODL VCV BASETIME; BOF
 * SET MOONTHIS +2 MOONFLAG; SET INTYPFLG; STCALL TET INTEGRVS (where
 * INTSTALL returns). */
INTERP(intstall_ret,03172u)
 lmi_newops(s,043175);vload(s,002170);right(s);if(sw(s,io,000263))return EXEC_INVALID_ACTION;next(s);
 stovl(s,025535,002105);
 stodl(s,015543,002114);
 lmi_newops(s,043014);
 if(sw(s,io,004344))lmi_swbranch(s,077205);
 else{right(s);if(sw(s,io,000063))return EXEC_INVALID_ACTION;next(s);}
 lmi_newops(s,077614);if(sw(s,io,001473))return EXEC_INVALID_ACTION;next(s);
 stcall(s,035517,027107);
 return EXEC_OK;
}
/* OTHCONIC VLOAD RATT; STOVL RONE VATT; STCALL VONE INTSTALL (where
 * INTEGRVS returns, and R31SURF's GOTO). */
INTERP(othconic,03211u)
 lmi_newops(s,077775);vload(s,000001);next(s);
 stovl(s,026207,000007);
 stcall(s,036215,027414);
 return EXEC_OK;
}
/* SET DLOAD INTYPFLG TAT (where INTSTALL returns); on into OTHINT. */
INTERP(intstall_ret2,03217u)
 lmi_newops(s,071214);if(sw(s,io,001473))return EXEC_INVALID_ACTION;right(s);lmi_direct(s,000015,DLOAD);lmi_dload_op(s,0);next(s);
 return EXEC_OK;
}
/* OTHINT STORE TDEC1; VLOAD CLEAR BASEOTP MOONFLAG; STOVL RCV BASEOTV;
 * STODL VCV BASETIME; BOF SET MOONTHIS +2 MOONFLAG; STCALL TET INTEGRVS. */
INTERP(othint,03222u)
 store(s,000041);
 lmi_newops(s,043175);vload(s,002140);right(s);if(sw(s,io,000263))return EXEC_INVALID_ACTION;next(s);
 stovl(s,025535,002120);
 stodl(s,015543,002114);
 lmi_newops(s,043014);
 if(sw(s,io,004344))lmi_swbranch(s,077236);
 else{right(s);if(sw(s,io,000063))return EXEC_INVALID_ACTION;next(s);}
 stcall(s,035517,027107);
 return EXEC_OK;
}
/* COMPDISP VLOAD VSU RATT RONE; RTB PDDL NORMUNX1 36D (the NORM front and
 * UNIT's phases) (where INTEGRVS returns, and GETRVN's GOTO). */
INTERP(compdisp,03240u)
 enum rtb_unit_status st;uint8_t phase;
 lmi_newops(s,052375);vload(s,000001);right(s);opnd(s,002207,VSU);lmi_vsu(s,0);next(s);
 lmi_newops(s,065234);rtb(s,021724);
 if(rtb_vector(s,io,RTB_LM,RTB_NORMUNX1_FRONT)!=RTB_OK)return EXEC_INVALID_ACTION;
 for(phase=0;phase<3u;++phase){
  st=rtb_unit_phase(s,io,RTB_LM,phase);
  if(st==RTB_UNIT_ABORT_1302)return EXEC_OK;
  if(st==RTB_UNIT_OUT_OF_SCOPE)return EXEC_INVALID_ACTION;
  if(st==RTB_UNIT_OVERFLOW)break;
 }
 right(s);opnd(s,000045,PDDL);lmi_pddl_op(s,0);next(s);
 return EXEC_OK;
}
/* SL* 0,1; STOVL RANGE VATT; VSU DOT VONE; SL1; STOVL RRATE RONE; UNIT PDVL
 * UNITZ; CALL CDU*NBSM. */
INTERP(range_rate,03246u)
 lmi_newops(s,077657);lmi_indexed(s,020201,GSHIFT);lmi_gshift(s);next(s);
 stovl(s,026201,000007);
 lmi_newops(s,050251);opnd(s,002215,VSU);lmi_vsu(s,0);right(s);opnd(s,077752,DOT);lmi_dot(s,0);next(s);
 lmi_newops(s,077752);shorts(s);next(s);
 stovl(s,026203,002207);
 lmi_newops(s,063256);lmi_unary(s,UNIT);if(lmi_unit(s))return EXEC_OK;right(s);pdvl(s,006514,unitz);next(s);
 lmi_newops(s,077624);call(s,047661);
 return EXEC_OK;
}
/* VXM PUSH REFSMMAT; VPROJ VSL2 0D; BVSU UNIT 6D; PDVL VXV 0D VONE; VXV DOT
 * 0D 12D; PDVL; DOT SIGN 6D 12D; SL2 ACOS, ACOS dispatched to ARCCOS (where
 * CDU*NBSM returns). */
INTERP(cdunbsm_ret,03263u)
 lmi_newops(s,041505);vxm(s,001734);right(s);push(s);next(s);
 lmi_newops(s,072431);opnd(s,000001,VPROJ);lmi_vproj(s,0);right(s);shorts(s);next(s);
 lmi_newops(s,053445);opnd(s,000007,BVSU);lmi_bvsu(s,0);right(s);lmi_unary(s,UNIT);if(lmi_unit(s))return EXEC_OK;next(s);
 lmi_newops(s,047315);pdvl(s,000001,0);right(s);opnd(s,002215,VXV);lmi_vxv(s,0);next(s);
 lmi_newops(s,050235);opnd(s,000001,VXV);lmi_vxv(s,0);right(s);opnd(s,000015,DOT);lmi_dot(s,0);next(s);
 lmi_newops(s,077715);pdvl(s,075241,0);next(s);
 lmi_newops(s,075241);opnd(s,000007,DOT);lmi_dot(s,0);right(s);opnd(s,000015,SIGN);lmi_sign(s,0);next(s);
 lmi_newops(s,065512);shorts(s);right(s);lmi_unary(s,ARCCOS);
 return EXEC_OK;
}
/* STOVL RTHETA 0D; DOT BPL 6D +5; DLOAD DSU DPPOSMAX RTHETA; STORE RTHETA,
 * unless the BPL is taken; EXIT (where ARCCOS returns). */
INTERP(acos_ret,03304u)
 stovl(s,026205,000001);
 lmi_newops(s,051041);opnd(s,000007,DOT);lmi_dot(s,0);right(s);lmi_misc(s,077315,BPLBMN);
 if(lmi_bplbmn(s))lmi_goto(s);
 else{
  next(s);
  lmi_newops(s,045345);lmi_direct(s,006530,DLOAD);lmi_dload_op(s,dpposmax);right(s);opnd(s,002205,DSU);lmi_dsu(s,0);next(s);
  store(s,002205);
 }
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* GETRVN VLOAD SETPD RN 0; STOVL RONE VN; STOVL VONE V(CSM); PDVL PDDL
 * R(CSM) PIPTIME; EXIT (after GETRVN's TC INTPRET). */
INTERP(getrvn_i,03331u)
 lmi_newops(s,040375);vload(s,001221);right(s);setpd(s,000001);next(s);
 stovl(s,026207,001227);
 stovl(s,026215,001726);
 lmi_newops(s,065315);pdvl(s,001720,0);right(s);opnd(s,001235,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* BOFF VLOAD MUNFLAG GETRVN2; VXM VSR4 REFSMMAT; PDVL (after the second TC
 * INTPRET). A clear MUNFLAG ends at GETRVN2's NEWOPS. */
INTERP(getrvn_i2,03347u)
 lmi_newops(s,077214);
 if(sw(s,io,003347)){lmi_swbranch(s,077363);return EXEC_OK;}
 right(s);vload(s,060505);next(s);
 lmi_newops(s,060505);vxm(s,001734);right(s);shorts(s);next(s);
 lmi_newops(s,077715);pdvl(s,076505,0);next(s);
 return EXEC_OK;
}
/* VXM VSL1 REFSMMAT; PUSH SETPD 0; GOTO COMPDISP. */
INTERP(vxm_vsl1,03355u)
 lmi_newops(s,076505);vxm(s,001734);right(s);shorts(s);next(s);
 lmi_newops(s,040206);push(s);right(s);setpd(s,000001);next(s);
 lmi_newops(s,077650);goto_(s,077240);
 return EXEC_OK;
}
/* GETRVN2 CALL INTSTALL. */
INTERP(getrvn2,03363u)
 lmi_newops(s,077624);call(s,027414);
 return EXEC_OK;
}
/* CLEAR GOTO INTYPFLG OTHINT (where INTSTALL returns). */
INTERP(intstall_ret3,03365u)
 lmi_newops(s,052014);if(sw(s,io,001673))return EXEC_INVALID_ACTION;right(s);goto_(s,077222);
 return EXEC_OK;
}
/* R31SURF's RTB LOADTIME (after its TC INTPRET). */
INTERP(r31surf_rtb,03371u)
 lmi_newops(s,077634);rtb(s,021573);
 return EXEC_OK;
}
/* STCALL TDEC1 LEMPREC (where LOADTIME returns). */
INTERP(r31surf_lemprec,03373u)
 stcall(s,034041,027057);
 return EXEC_OK;
}
/* GOTO OTHCONIC (where LEMPREC returns). */
INTERP(lemprec_ret2,03375u)
 lmi_newops(s,077650);goto_(s,077211);
 return EXEC_OK;
}
/* ONEBASE RTB GOTO LOADTIME DOCMBASE (after its TC INTPRET): the run stops
 * at LOADTIME, whose DANZIG return takes the GOTO (077144) outside these
 * routines. */
INTERP(onebase_rtb,03404u)
 lmi_newops(s,052034);rtb(s,021573);
 /* The right-hand GOTO DOCMBASE (077144) runs only after LOADTIME. */
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
#if HAS_G6
 {LM_R31_B37(03132),v83_rtb},{LM_R31_B37(03134),v83_lemprec},{LM_R31_B37(03136),lemprec_ret},{LM_R31_B37(03144),docmbase},
 {LM_R31_B37(03147),csmprec_ret},{LM_R31_B37(03166),rev83_rtb},{LM_R31_B37(03170),rev83_intstall},{LM_R31_B37(03172),intstall_ret},
 {LM_R31_B37(03211),othconic},{LM_R31_B37(03217),intstall_ret2},{LM_R31_B37(03222),othint},
#endif
#if HAS_G1
 {LM_R31_B37(03240),compdisp},
#endif
#if HAS_G2
 {LM_R31_B37(03246),range_rate},
#endif
#if HAS_G3
 {LM_R31_B37(03263),cdunbsm_ret},
#endif
#if HAS_G6
 {LM_R31_B37(03304),acos_ret},{LM_R31_B37(03331),getrvn_i},
#endif
#if HAS_G4
 {LM_R31_B37(03347),getrvn_i2},
#endif
#if HAS_G5
 {LM_R31_B37(03355),vxm_vsl1},
#endif
#if HAS_G4||HAS_G6
 {LM_R31_B37(03363),getrvn2},
#endif
#if HAS_G6
 {LM_R31_B37(03365),intstall_ret3},{LM_R31_B37(03371),r31surf_rtb},{LM_R31_B37(03373),r31surf_lemprec},
 {LM_R31_B37(03375),lemprec_ret2},{LM_R31_B37(03404),onebase_rtb},
#endif
};
const agc_routine_table lm_r31_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
static void tcf(apollo_overlay_state *s,agc_word target){W[5]=target;}
/* Basic entry: canonical image at Z in bank 37 (SUPERBNK below bank 40's)
 * or in bank 40 (FB 30 with SUPERBNK 4), with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z,agc_word fb){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=fb||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(fb==FB30?io->superbank!=0100u:io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z),FB37))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ENTRY40(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z),FB30))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* R31CALL CAF PRIO3; TC FINDVAC (2CADR V83CALL follows). */
ROUTINE(r31call){ENTRY40(03674u);ai_caf(s,K_PRIO3);tc(s,03676u,FINDVAC);return EXEC_OK;}
/* DSPDELAY TC FIXDELAY (DEC 100 follows). */
ROUTINE(dspdelay){ENTRY40(03700u);tc(s,03701u,FIXDELAY);return EXEC_OK;}
/* CA EXTVBACT; MASK BIT12; EXTEND; BZF DSPDELAY; CAF PRIO5; TC NOVAC
 * (2CADR DISPN5X follows). */
ROUTINE(fixdelay_ret){ENTRY40(03702u);
 ai_ca(s,EXTVBACT);ai_maskf(s,K_BIT12);
 if(ai_bzf(s)){tcf(s,03700u);return EXEC_OK;}
 ai_caf(s,K_PRIO5);tc(s,03710u,NOVAC);return EXEC_OK;}
/* TCF TASKOVER (where NOVAC returns). */
ROUTINE(r_taskover){ENTRY40(03712u);tcf(s,TASKOVER);return EXEC_OK;}
/* DISPN5X CAF V16N54; TC BANKCALL (CADR GOMARKF). */
ROUTINE(dispn5x){ENTRY(03113u);ai_caf(s,K_V16N54);tc(s,03115u,BANKCALL);return EXEC_OK;}
/* GOMARKF's returns: TC B5OFF; TC B5OFF; TCF DISPN5X. */
ROUTINE(b5off_1){ENTRY(03116u);tc(s,03117u,B5OFF);return EXEC_OK;}
ROUTINE(b5off_2){ENTRY(03117u);tc(s,03120u,B5OFF);return EXEC_OK;}
ROUTINE(redisplay){ENTRY(03120u);tcf(s,03113u);return EXEC_OK;}
/* V83CALL CS FLAGWRD7; MASK AVEGFBIT; EXTEND; BZF MUNG?; CS FLAGWRD8; MASK
 * SURFFBIT; EXTEND; BZF ONEBASE; TC INTPRET. */
ROUTINE(v83call){ENTRY(03121u);
 ai_cs(s,FLAGWRD7);ai_maskf(s,K_AVEGFBIT);if(ai_bzf(s)){tcf(s,03377u);return EXEC_OK;}
 ai_cs(s,FLAGWRD8);ai_maskf(s,K_SURFFBIT);if(ai_bzf(s)){tcf(s,03403u);return EXEC_OK;}
 tc(s,03132u,INTPRET);return EXEC_OK;}
/* REV83 CS FLAGWRD7; MASK AVEGFBIT; EXTEND; BZF GETRVN; CS FLAGWRD8; MASK
 * SURFFBIT; EXTEND; BZF R31SURF; TC INTPRET. */
ROUTINE(rev83){ENTRY(03155u);
 ai_cs(s,FLAGWRD7);ai_maskf(s,K_AVEGFBIT);if(ai_bzf(s)){tcf(s,03326u);return EXEC_OK;}
 ai_cs(s,FLAGWRD8);ai_maskf(s,K_SURFFBIT);if(ai_bzf(s)){tcf(s,03370u);return EXEC_OK;}
 tc(s,03166u,INTPRET);return EXEC_OK;}
/* After the EXIT at 3315: CA BIT5; MASK EXTVBACT; EXTEND; BZF ENDEXT; CS
 * EXTVBACT; MASK BIT12; ADS EXTVBACT; TCF REV83. */
ROUTINE(exit_ret){ENTRY(03316u);
 ai_caf(s,K_BIT5);ai_mask(s,EXTVBACT);if(ai_bzf(s)){tcf(s,ENDEXT);return EXEC_OK;}
 ai_cs(s,EXTVBACT);ai_maskf(s,K_BIT12);ai_ads(s,EXTVBACT);tcf(s,03155u);return EXEC_OK;}
/* GETRVN CA PRIO22; TC PRIOCHNG; TC INTPRET. */
ROUTINE(getrvn){ENTRY(03326u);ai_caf(s,K_PRIO22);tc(s,03330u,PRIOCHNG);return EXEC_OK;}
ROUTINE(prio_ret){ENTRY(03330u);tc(s,03331u,INTPRET);return EXEC_OK;}
/* After the EXIT at 3343: CA PRIO3; TC PRIOCHNG; TC INTPRET. */
ROUTINE(getrvn_exit){ENTRY(03344u);ai_caf(s,K_PRIO3);tc(s,03346u,PRIOCHNG);return EXEC_OK;}
ROUTINE(prio_ret2){ENTRY(03346u);tc(s,03347u,INTPRET);return EXEC_OK;}
/* R31SURF TC INTPRET. */
ROUTINE(r31surf){ENTRY(03370u);tc(s,03371u,INTPRET);return EXEC_OK;}
/* MUNG? CS FLAGWRD6; MASK MUNFLBIT; EXTEND; BZF GETRVN; on into ONEBASE
 * TC INTPRET. */
ROUTINE(mung){ENTRY(03377u);
 ai_cs(s,FLAGWRD6);ai_maskf(s,K_MUNFLBIT);if(ai_bzf(s)){tcf(s,03326u);return EXEC_OK;}
 tc(s,03404u,INTPRET);return EXEC_OK;}
ROUTINE(onebase){ENTRY(03403u);tc(s,03404u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
#if HAS_G6
 {LM_R31_B37(03113),dispn5x},{LM_R31_B37(03116),b5off_1},{LM_R31_B37(03117),b5off_2},{LM_R31_B37(03120),redisplay},
 {LM_R31_B37(03121),v83call},{LM_R31_B37(03155),rev83},{LM_R31_B37(03316),exit_ret},{LM_R31_B37(03326),getrvn},
 {LM_R31_B37(03330),prio_ret},{LM_R31_B37(03344),getrvn_exit},{LM_R31_B37(03346),prio_ret2},{LM_R31_B37(03370),r31surf},
 {LM_R31_B37(03377),mung},{LM_R31_B37(03403),onebase},
 {LM_R31_B40(03674),r31call},{LM_R31_B40(03700),dspdelay},{LM_R31_B40(03702),fixdelay_ret},{LM_R31_B40(03712),r_taskover},
#else
 {0,0}
#endif
};
const agc_routine_table lm_r31_basic_routines={basic,(uint16_t)(HAS_G6?sizeof basic/sizeof *basic:0u)};
