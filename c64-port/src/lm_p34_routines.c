/* SPDX-License-Identifier: MIT */
/* Luminary099 P34-35_P74-75 as routines of the control-transfer ABI; see
 * include/lm_p34_routines.h and docs/LM-P34-TRANSFER.md. Each basic
 * instruction of the source is one ai_* register effect on the image, in
 * source order; the interpretive routines run the lm-vn1645 and lm-midgim
 * steps and CLUPDATE's CLEAR GOTO with the shared interpretive bodies. */
#include "lm_p34_routines.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_vn1645.h"
#define W s->words
/* Erasable (EB-window addresses are the source's; EB selects the bank). */
#define FLAGWRD2 076u
#define MODREG 01011u
#define DISPDEX 01163u
#define NORMEX 01463u
#define QSAVED 01464u
#define RTRN 01465u
#define SUBEXIT 01470u
#define VERBNOUN 01615u
/* Fixed constants. */
#define K_BIT6 040u
#define K_1SEC 0144u
#define K_HIGH9 077700u
#define K_V06N55 01467u
#define K_V16N45 04055u
/* Destinations outside the file. */
#define COMPTGO 02432u  /* 35,2432 (P32-P35_P72-P75) */
#define BANKCALL 04616u
#define PHASCHNG 05353u
#define UPFLAG 05504u
#define GOTOPOOH 06001u
#define INTPRET 06037u
enum {SWITCH=02516,BZEGOTO=02467};

static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
static void tcf(apollo_overlay_state *s,agc_word target){W[5]=target;}

/* Basic entry: canonical image at Z in bank 35 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=072000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)

/* KILCLOCK 3651: CA Z; TS DISPDEX; TC GOTOPOOH. */
static void kilclock(apollo_overlay_state *s){ai_caf(s,03652u);(void)ai_ts(s,DISPDEX);tc(s,03654u,GOTOPOOH);}
/* N45PROC 3654: CS FLAGWRD2; MASK BIT6; EXTEND; BZF KILCLOCK;
 * TC PHASCHNG (OCT 04024 follows). */
static void n45proc(apollo_overlay_state *s){
 ai_cs(s,FLAGWRD2);ai_maskf(s,K_BIT6);
 if(ai_bzf(s)){kilclock(s);return;}
 tc(s,03661u,PHASCHNG);
}
/* CLUPDATE 3664: CA Z; TS DISPDEX; TC PHASCHNG (OCT 04024 follows). */
static void clupdate(apollo_overlay_state *s){ai_caf(s,03665u);(void)ai_ts(s,DISPDEX);tc(s,03667u,PHASCHNG);}
/* DISPLAYE 3676 and VNPOOH 3715: the display request, TC(R) BANKCALL with
 * CADR GOFLASH at Q. */
static void displaye_flash(apollo_overlay_state *s){ai_caf(s,K_V06N55);tc(s,03700u,BANKCALL);}
static void vnpooh_flash(apollo_overlay_state *s){ai_ca(s,VERBNOUN);tc(s,03717u,BANKCALL);}
/* TC NORMEX / TC RTRN at `at`: the word DISPLAYE or VNPOOH stored there by
 * QXCH is a TC to its caller's return, which runs from erasable and leaves Q
 * one past it. Any other word is outside this file's contract. */
static enum exec_status stored_return(apollo_overlay_state *s,agc_word at,unsigned cell){agc_word w=ai_word(s,cell);
 if(w<02000u||w>07777u)return EXEC_OUT_OF_SCOPE;
 tc(s,(agc_word)(at+1u),(agc_word)cell);tc(s,(agc_word)(cell+1u),w);return EXEC_OK;
}

/* GET45 +1: TC COMPTGO. */
enum exec_status lm_p34_get45_comptgo(const agc_native_program *p,apollo_overlay_state *s,void *io){ENTRY(03635u);tc(s,03636u,COMPTGO);return EXEC_OK;}
/* COMPTGO's return: CA SUBEXIT; TS QSAVED; CAF 1SEC; TC BANKCALL with
 * CADR DELAYJOB. */
ROUTINE(get45_delay){ENTRY(03636u);ai_ca(s,SUBEXIT);(void)ai_ts(s,QSAVED);ai_caf(s,K_1SEC);tc(s,03642u,BANKCALL);return EXEC_OK;}
/* DELAYJOB's return: CAF V16N45; TC BANKCALL with CADR GOFLASH. */
ROUTINE(get45_flash){ENTRY(03643u);ai_caf(s,K_V16N45);tc(s,03645u,BANKCALL);return EXEC_OK;}
/* GOFLASH's terminate, proceed and recycle returns. */
ROUTINE(get45_terminate){ENTRY(03646u);tc(s,03647u,03651u);kilclock(s);return EXEC_OK;}
ROUTINE(get45_proceed){ENTRY(03647u);tc(s,03650u,03654u);n45proc(s);return EXEC_OK;}
ROUTINE(get45_recycle){ENTRY(03650u);tc(s,03651u,03664u);clupdate(s);return EXEC_OK;}
ROUTINE(r_kilclock){ENTRY(03651u);kilclock(s);return EXEC_OK;}
ROUTINE(r_n45proc){ENTRY(03654u);n45proc(s);return EXEC_OK;}
/* PHASCHNG's return in N45PROC: TC UPFLAG with ADRES FINALFLG; UPFLAG
 * returns to CLUPDATE. */
ROUTINE(n45proc_upflag){ENTRY(03662u);tc(s,03663u,UPFLAG);return EXEC_OK;}
ROUTINE(r_clupdate){ENTRY(03664u);clupdate(s);return EXEC_OK;}
/* PHASCHNG's return in CLUPDATE: TC INTPRET. */
ROUTINE(clupdate_intpret){ENTRY(03670u);tc(s,03671u,INTPRET);return EXEC_OK;}
/* DISPLAYE 3674: EXTEND; QXCH NORMEX; CAF V06N55; TCR BANKCALL. */
ROUTINE(r_displaye){ENTRY(03674u);ai_qxch(s,NORMEX);displaye_flash(s);return EXEC_OK;}
/* GOFLASH's returns: TCF GOTOPOOH; TC NORMEX; TCF -5. */
ROUTINE(displaye_terminate){ENTRY(03701u);tcf(s,GOTOPOOH);return EXEC_OK;}
ROUTINE(displaye_proceed){ENTRY(03702u);return stored_return(s,03702u,NORMEX);}
ROUTINE(displaye_recycle){ENTRY(03703u);tcf(s,03676u);displaye_flash(s);return EXEC_OK;}
/* P3XORP7X 3704: CAF HIGH9; MASK MODREG; EXTEND; BZF +2; INCR Q; RETURN.
 * RETURN (TC Q) runs the TC that Q holds and leaves Q one past Q itself. */
ROUTINE(r_p3xorp7x){agc_word ret;ENTRY(03704u);
 ai_caf(s,K_HIGH9);ai_mask(s,MODREG);if(!ai_bzf(s))ai_incr(s,2u);
 ret=W[2];tc(s,3u,ret);return EXEC_OK;
}
/* VNPOOH 3712: EXTEND; QXCH RTRN; TS VERBNOUN; CA VERBNOUN (skipped when
 * TS overflows); TCR BANKCALL. */
ROUTINE(r_vnpooh){ENTRY(03712u);ai_qxch(s,RTRN);
 if(ai_ts(s,VERBNOUN)){tc(s,03717u,BANKCALL);return EXEC_OK;}
 vnpooh_flash(s);return EXEC_OK;
}
ROUTINE(vnpooh_terminate){ENTRY(03720u);tcf(s,GOTOPOOH);return EXEC_OK;}
ROUTINE(vnpooh_proceed){ENTRY(03721u);return stored_return(s,03721u,RTRN);}
ROUTINE(vnpooh_recycle){ENTRY(03722u);tcf(s,03715u);vnpooh_flash(s);return EXEC_OK;}

static const agc_routine_entry basic[LM_P34_BASIC_ROUTINES]={
 {LM_P34_B35(03635),lm_p34_get45_comptgo},{LM_P34_B35(03636),get45_delay},{LM_P34_B35(03643),get45_flash},
 {LM_P34_B35(03646),get45_terminate},{LM_P34_B35(03647),get45_proceed},{LM_P34_B35(03650),get45_recycle},
 {LM_P34_B35(03651),r_kilclock},{LM_P34_B35(03654),r_n45proc},{LM_P34_B35(03662),n45proc_upflag},
 {LM_P34_B35(03664),r_clupdate},{LM_P34_B35(03670),clupdate_intpret},{LM_P34_B35(03674),r_displaye},
 {LM_P34_B35(03701),displaye_terminate},{LM_P34_B35(03702),displaye_proceed},{LM_P34_B35(03703),displaye_recycle},
 {LM_P34_B35(03704),r_p3xorp7x},{LM_P34_B35(03712),r_vnpooh},{LM_P34_B35(03720),vnpooh_terminate},
 {LM_P34_B35(03721),vnpooh_proceed},{LM_P34_B35(03722),vnpooh_recycle}};
const agc_routine_table lm_p34_basic_routines={basic,LM_P34_BASIC_ROUTINES};
/* DISPLAYE's and VNPOOH's CADR GOFLASH, after TCR BANKCALL (the generated
 * INTER-BANK site table lists TC sites only). */
static const agc_site_operand sites[LM_P34_SITES]={{LM_P34_B35(03700),020476u},{LM_P34_B35(03717),020476u}};
const agc_site_operand *lm_p34_site_table(uint16_t *count){if(count)*count=LM_P34_SITES;return sites;}

/* Interpretive entry: canonical image at NEWOPS with LOC `loc` in FB `fb`,
 * coherent banks with BANKSET = BB, INTBIT15 as the interpreter sets it for
 * that bank, the VAC one of the five with PUSHLOC inside it, MODE one of the
 * three. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc,agc_word fb){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=fb||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=(fb>=040000u?040000u:0u)||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
static enum exec_status stepped(uint8_t next){return next==LMG_FAILED?EXEC_INVALID_ACTION:EXEC_OK;}
#define INTERP(name) enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
INTERP(lm_p34_vn1645){(void)p;if(!s||!io||lm_vn1645_entry(s,io)!=LVN_VN1645)return EXEC_OUT_OF_SCOPE;return stepped(lvn_vn1645(s,io));}
INTERP(lm_p34_vn1645_mga){(void)p;if(!newops_ok(s,io,03630u,072000u))return EXEC_OUT_OF_SCOPE;return stepped(lvn_mga(s,io));}
INTERP(lm_p34_get45){(void)p;if(!newops_ok(s,io,03634u,072000u))return EXEC_OUT_OF_SCOPE;return stepped(lvn_get45(s,io));}
static uint8_t midgim(const apollo_overlay_state *s,const p66_bias_io *io,uint8_t phase){return (uint8_t)(s&&io&&lm_midgim_entry(s,io)==phase);}
INTERP(lm_p34_getmga){(void)p;if(!midgim(s,io,LMG_GETMGA))return EXEC_OUT_OF_SCOPE;return stepped(lmg_getmga(s,io));}
INTERP(lm_p34_getlvc){(void)p;if(!midgim(s,io,LMG_GETLVC))return EXEC_OUT_OF_SCOPE;return stepped(lmg_getlvc(s,io));}
INTERP(lm_p34_selectmu){(void)p;if(!midgim(s,io,LMG_SELECTMU))return EXEC_OUT_OF_SCOPE;return stepped(lmg_selectmu(s,io));}
/* CLUPDATE 35,3671: CLEAR GOTO UPDATFLG QSAVED, to the NEWOPS of the
 * interpretive return VN1645 saved in QSAVED. */
INTERP(lm_p34_clupdate){(void)p;if(!newops_ok(s,io,03671u,072000u))return EXEC_OUT_OF_SCOPE;
 lmi_newops(s,052014);lmi_misc(s,000670,SWITCH);if(lmi_switch(s,io))return EXEC_INVALID_ACTION;
 lmi_danzig_right(s);lmi_misc(s,003464,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);return EXEC_OK;
}
static const agc_routine_entry interpretive[LM_P34_INTERPRETIVE_ROUTINES]={
 {LM_P34_B06(03717),lm_p34_getmga},{LM_P34_B06(03733),lm_p34_getlvc},{LM_P34_B10(02000),lm_p34_selectmu},
 {LM_P34_B35(03606),lm_p34_vn1645},{LM_P34_B35(03630),lm_p34_vn1645_mga},{LM_P34_B35(03634),lm_p34_get45},{LM_P34_B35(03671),lm_p34_clupdate}};
const agc_routine_table lm_p34_interpretive_routines={interpretive,LM_P34_INTERPRETIVE_ROUTINES};
