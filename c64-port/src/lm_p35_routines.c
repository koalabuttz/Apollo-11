/* SPDX-License-Identifier: MIT */
/* Luminary099 P35/P75 (P34-35_P74-75.agc, bank 35 02725-02763) as routines
 * of the control-transfer ABI; see include/lm_p35_routines.h and
 * docs/LM-P35.md. Every interpretive pair is compiled with the shared bodies
 * (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * those paths. Each basic instruction is one ai_* register effect on the
 * image, in source order. */
#include "lm_p35_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#define W s->words
/* INDJUMP/MISCJUMP body entries. */
enum {DAD=06775,CALLITA=02507,RTBBHIZ=02447,BZEGOTO=02467};
/* Erasable (EB-window addresses as the source names them, EBANK=KT; the
 * DCA and DXCH words carry K+1). */
#define ATIGINC 01400u
#define PTIGINC 01402u
#define KT 01575u
/* Destinations outside the routines: AVFLAGA, AVFLAGP and P20FLGON
 * (P32-P35_P72-P75, bank 35) and INTPRET. */
#define AVFLAGA 02347u
#define AVFLAGP 02354u
#define P20FLGON 02361u
#define INTPRET 06037u

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}

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
 p66_bias_io *io=v;(void)p;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS or at RTB's basic routine: the
 * trampoline continues there or stops outside the registered routines. */

/* CALL SELECTMU (after TC INTPRET). */
INTERP(sel_mu,02737u)
 lmi_newops(s,077624);call(s,020000);
 return EXEC_OK;
}
/* P35/P75B RTB LOADTIME. */
static void p35b(apollo_overlay_state *s){
 lmi_newops(s,077634);lmi_misc(s,021573,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);
}
INTERP(r_p35b,02741u)
 p35b(s);
 return EXEC_OK;
}
/* STORE TSTRT; DAD KT; STORE TIG; STORE INTIME; STCALL TDEC1 PRECSET (where
 * LOADTIME returns). */
INTERP(loadtime_ret,02743u)
 store(s,003612);
 lmi_newops(s,077615);lmi_direct(s,003576,DAD);lmi_dad(s,0);next(s);
 store(s,003442);
 store(s,003610);
 lmi_store_code(s,034041);lmi_15bitadr(s,046360,CALLITA);lmi_call(s);lmi_goto(s);
 return EXEC_OK;
}
/* The CALLs of S34/35.1, S34/35.2, S34/35.5 and VN1645, each where the one
 * before returns. */
INTERP(call_s1,02752u)
 lmi_newops(s,077624);call(s,073274);
 return EXEC_OK;
}
INTERP(call_s2,02754u)
 lmi_newops(s,077624);call(s,073306);
 return EXEC_OK;
}
INTERP(call_s5,02756u)
 lmi_newops(s,077624);call(s,073522);
 return EXEC_OK;
}
INTERP(call_vn1645,02760u)
 lmi_newops(s,077624);call(s,073606);
 return EXEC_OK;
}
/* GOTO P35/P75B (where VN1645 returns). */
INTERP(loop,02762u)
 lmi_newops(s,077650);lmi_misc(s,072741,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
 {LM_P35_B35(02737),sel_mu},{LM_P35_B35(02741),r_p35b},{LM_P35_B35(02743),loadtime_ret},
 {LM_P35_B35(02752),call_s1},{LM_P35_B35(02754),call_s2},{LM_P35_B35(02756),call_s5},
 {LM_P35_B35(02760),call_vn1645},{LM_P35_B35(02762),loop},
};
const agc_routine_table lm_p35_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

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
/* P35/P75A DXCH KT; TC P20FLGON. */
static void p35a(apollo_overlay_state *s){ai_dxch(s,KT);tc(s,02736u,P20FLGON);}
ROUTINE(p35){ENTRY(02725u);tc(s,02726u,AVFLAGA);return EXEC_OK;}
/* EXTEND; DCA ATIGINC; TC P35/P75A. */
ROUTINE(p35_avflaga){ENTRY(02726u);ai_dca(s,ATIGINC);tc(s,02731u,02734u);p35a(s);return EXEC_OK;}
ROUTINE(p75){ENTRY(02731u);tc(s,02732u,AVFLAGP);return EXEC_OK;}
/* EXTEND; DCA PTIGINC; on into P35/P75A. */
ROUTINE(p75_avflagp){ENTRY(02732u);ai_dca(s,PTIGINC);p35a(s);return EXEC_OK;}
/* TC INTPRET (where P20FLGON returns). */
ROUTINE(p20_intpret){ENTRY(02736u);tc(s,02737u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
 {LM_P35_B35(02725),p35},{LM_P35_B35(02726),p35_avflaga},{LM_P35_B35(02731),p75},
 {LM_P35_B35(02732),p75_avflagp},{LM_P35_B35(02736),p20_intpret},
};
const agc_routine_table lm_p35_basic_routines={basic,(uint16_t)(sizeof basic/sizeof *basic)};
