/* SPDX-License-Identifier: MIT */
/* Comanche055 FRESH_START_AND_RESTART interpretive code: bank 13
 * (02000-02035) STATEUP, STATEND and THISVINT, and R00's CALL INTSTALL;
 * DUMMYAD EXIT in bank 04 (02131-02133). One case per order-pair word, each
 * the compiled Comanche055 interpreter path for its words in source order
 * (NEWOPS, the left order, DANZIG's right-hand dispatch, the right order,
 * DANZIG), with the pair's words as literal constants; nothing is read from
 * code memory. */
#include "cm_stateup.h"
#include "agc_image_ops.h"
#include "cm_interp.h"

#define W s->words
#define NEWJOB 067u
#define EDOP 023u
#define FIXLOC 0120u
#define PUSHLOC 0166u
#define LOC CM_STATEUP_LOC
#define BANKSET CM_STATEUP_BANKSET
#define B13 (013u<<10)
#define B04 (04u<<10)
#define X_CHANG2 05170u
/* MISCJUMP/INDJUMP/UNAJUMP body entries (CALL/ITA, SWITCHES, DLOAD, RVQ). */
enum {DLOAD=06021,RVQ=03274,CALLITA=02534,SWITCH=02543};

/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS (the next
 * pair); positive goes to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS
 * NEWJOB; TCF CHANG2), where the run leaves. */
static enum exec_status next(apollo_overlay_state *s){
 agc_word j=W[NEWJOB];
 if(j==0u){cmi_danzig_next(s);return EXEC_OK;}
 /* A job switch (positive: the new job's core-set relative address) is
  * taken below. Negative is refused: EXECUTIVE stores only +0, positive
  * core-set addresses and DUMMYJOB's idle -0 there, and DUMMYJOB leaves -0
  * only while no job runs (ADVAN starts the next job through CHANJOB, which
  * sets +0), so a running interpretive job never finds -0 or negative. */
 if(j>037777u)return EXEC_OUT_OF_SCOPE;
 ai_ca(s,BANKSET);(void)ai_ts(s,6);(void)ai_ccs(s,EDOP);(void)ai_ccs(s,NEWJOB);W[5]=X_CHANG2;
 return EXEC_OK;
}
static void right(apollo_overlay_state *s){cmi_danzig_right(s);}
/* A switch order (word = its switch/operation word): 1 when it branches. */
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){cmi_misc(s,word,SWITCH);return cmi_switch(s,io);}
/* The branch word of a switch order that branched: GOTO to its NEWOPS. */
static enum exec_status branch(apollo_overlay_state *s,agc_word word){cmi_swbranch(s,word);return EXEC_OK;}
/* CALL label: QPRET, then GOTO to the callee's NEWOPS. */
static enum exec_status call(apollo_overlay_state *s,agc_word word){cmi_misc(s,word,CALLITA);cmi_call(s);cmi_goto(s);return EXEC_OK;}

/* The interpreter's own state at NEWOPS: EDOP +0, a work area (one of the
 * five VAC areas, PUSHLOC inside it) and BBANK under BANKSET in the pair's
 * bank (13, or 04 for R00). */
static p66_bias_io *ctx(const apollo_overlay_state *s,void *io){
 p66_bias_io *x=io;agc_word f;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 if(W[5]!=CM_STATEUP_NEWOPS||(W[4]&076000u)!=(W[LOC]>=02131u?B04:B13)||((W[BANKSET]^W[4])&076000u)||W[EDOP])return 0;
 f=W[FIXLOC];
 if(f!=0401u&&f!=0455u&&f!=0531u&&f!=0605u&&f!=0661u)return 0;
 if(W[PUSHLOC]<f||W[PUSHLOC]>f+042u)return 0;
 return x;
}

static enum exec_status pair(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);
 (void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 switch(W[LOC]){
 /* STATEUP SET BOF VINTFLAG ORBWFLAG +3. */
 case 02000u:cmi_newops(s,043014);(void)sw(s,x,001474);right(s);if(sw(s,x,001751))return branch(s,026006);return next(s);
 /* SET DIM0FLAG. */
 case 02004u:cmi_newops(s,077614);(void)sw(s,x,001476);return next(s);
 /* CLEAR CALL PRECIFLG INTEGRV. */
 case 02006u:cmi_newops(s,045014);(void)sw(s,x,001667);right(s);return call(s,027162);
 /* BON DLOAD SURFFLAG STATEND TETCSM. */
 case 02011u:cmi_newops(s,071214);if(sw(s,x,004307))return branch(s,026031);right(s);cmi_direct(s,001571,DLOAD);cmi_dload_op(s,0);return next(s);
 /* STCALL TDEC1 INTSTALL. */
 case 02015u:cmi_store_code(s,034041);cmi_15bitadr(s,027442,CALLITA);cmi_call(s);cmi_goto(s);return EXEC_OK;
 /* CLEAR CALL VINTFLAG SETIFLGS. */
 case 02017u:cmi_newops(s,045014);(void)sw(s,x,001674);right(s);return call(s,026670);
 /* BOF SET RENDWFLG +2 DIM0FLAG. */
 case 02022u:cmi_newops(s,043014);if(sw(s,x,002756))return branch(s,026026);right(s);(void)sw(s,x,001476);return next(s);
 /* SET CALL PRECIFLG INTEGRV. */
 case 02026u:cmi_newops(s,045014);(void)sw(s,x,001467);right(s);return call(s,027162);
 /* STATEND CLRGO NODOFLAG ENDINT. */
 case 02031u:cmi_newops(s,077614);if(!sw(s,x,001236))return EXEC_OUT_OF_SCOPE;return branch(s,026656);
 /* THISVINT SET RVQ VINTFLAG: RVQ returns to the CADR in QPRET, which a
  * CALL leaves as a fixed address. */
 case 02034u:cmi_newops(s,043414);(void)sw(s,x,001474);right(s);
  if(!(W[(W[FIXLOC]+052u)&03777u]&074000u))return EXEC_OUT_OF_SCOPE;
  cmi_unary(s,RVQ);cmi_rvq(s);return EXEC_OK;
 /* R00 (bank 04, after ROC's TC INTPRET) CALL INTSTALL; DUMMYAD EXIT. */
 case 02131u:cmi_newops(s,077624);return call(s,027442);
 case 02133u:cmi_newops(s,077776);cmi_exit(s);return EXEC_OK;
 default:return EXEC_OUT_OF_SCOPE;}
}

const uint16_t cm_stateup_entries[CM_STATEUP_ROUTINES]={02000u,02004u,02006u,02011u,02015u,02017u,02022u,02026u,02031u,02034u,02131u,02133u};
#define K13(z) (uint16_t)(013u*1024u+((z)-02000u))
#define K04(z) (uint16_t)(04u*1024u+((z)-02000u))
static const agc_routine_entry entries[CM_STATEUP_ROUTINES]={
 {K04(02131),pair},{K04(02133),pair},
 {K13(02000),pair},{K13(02004),pair},{K13(02006),pair},{K13(02011),pair},{K13(02015),pair},{K13(02017),pair},
 {K13(02022),pair},{K13(02026),pair},{K13(02031),pair},{K13(02034),pair}};
const agc_routine_table cm_stateup_interpretive={entries,CM_STATEUP_ROUTINES};
