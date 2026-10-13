/* SPDX-License-Identifier: MIT */
/* Luminary099 FRESH_START_AND_RESTART interpretive code: bank 13
 * (02026-02067) STATEUP, DOINT, NO-INT, DOINT2 and THISVINT, and R00's
 * CALL INTSTALL; EXIT in bank 04 (02144-02146). One case per order-pair word, each
 * the compiled interpreter path for its words in source order (NEWOPS,
 * the left order, DANZIG's right-hand dispatch, the right order, DANZIG),
 * with the pair's words as literal constants; nothing is read from code
 * memory. */
#include "lm_stateup.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"

#define W s->words
#define NEWJOB 067u
#define EDOP 023u
#define FIXLOC 0120u
#define PUSHLOC 0166u
#define LOC LM_STATEUP_LOC
#define BANKSET LM_STATEUP_BANKSET
#define B13 (013u<<10)
#define B04 (04u<<10)
#define X_CHANG2 05126u
/* MISCJUMP/INDJUMP/UNAJUMP body entries. */
enum {DLOAD=06052,RVQ=03274,CALLITA=02507,SWITCH=02516};

/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS (the next
 * pair); positive goes to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS
 * NEWJOB; TCF CHANG2), where the run leaves. */
static enum exec_status next(apollo_overlay_state *s){
 agc_word j=W[NEWJOB];
 if(j==0u){lmi_danzig_next(s);return EXEC_OK;}
 /* A job switch (positive: the new job's core-set relative address) is
  * taken below. Negative is refused: EXECUTIVE stores only +0, positive
  * core-set addresses and DUMMYJOB's idle -0 there, and DUMMYJOB leaves -0
  * only while no job runs (ADVAN starts the next job through CHANJOB, which
  * sets +0), so a running interpretive job never finds -0 or negative. */
 if(j>037777u)return EXEC_OUT_OF_SCOPE;
 ai_ca(s,BANKSET);(void)ai_ts(s,6);(void)ai_ccs(s,EDOP);(void)ai_ccs(s,NEWJOB);W[5]=X_CHANG2;
 return EXEC_OK;
}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* A switch order (word = its switch/operation word): 1 when it branches. */
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
/* The branch word of a switch order that branched: GOTO to its NEWOPS. */
static enum exec_status branch(apollo_overlay_state *s,agc_word word){lmi_swbranch(s,word);return EXEC_OK;}
/* CALL label: QPRET, then GOTO to the callee's NEWOPS. */
static enum exec_status call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);return EXEC_OK;}

/* The interpreter's own state at NEWOPS: EDOP +0, a work area (one of the
 * five VAC areas, or FAKEPRET 0110 for a NOVAC job such as the V37 job that
 * runs R00, whose QPRET is MPAC +6; PUSHLOC inside it) and BBANK under
 * BANKSET in the pair's bank (13, or 04 for R00). */
static p66_bias_io *ctx(const apollo_overlay_state *s,void *io){
 p66_bias_io *x=io;agc_word f;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 if(W[5]!=LM_STATEUP_NEWOPS||(W[4]&076000u)!=(W[LOC]>=02144u?B04:B13)||((W[BANKSET]^W[4])&076000u)||W[EDOP])return 0;
 f=W[FIXLOC];
 if(f!=0110u&&f!=0401u&&f!=0455u&&f!=0531u&&f!=0605u&&f!=0661u)return 0;
 if(W[PUSHLOC]<f||W[PUSHLOC]>f+042u)return 0;
 return x;
}

static enum exec_status pair(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);
 (void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 switch(W[LOC]){
 /* STATEUP SET BOF VINTFLAG SURFFLAG DOINT. */
 case 02026u:lmi_newops(s,043014);(void)sw(s,x,001474);right(s);if(sw(s,x,004347))return branch(s,026036);return next(s);
 /* BOF SET RENDWFLG DOINT DIM0FLAG. */
 case 02032u:lmi_newops(s,043014);if(sw(s,x,002756))return branch(s,026036);right(s);(void)sw(s,x,001476);return next(s);
 /* DOINT CLEAR CALL PRECIFLG INTEGRV. */
 case 02036u:lmi_newops(s,045014);(void)sw(s,x,001667);right(s);return call(s,027134);
 /* BON DLOAD SURFFLAG NO-INT TETCSM. */
 case 02041u:lmi_newops(s,071214);if(sw(s,x,004307))return branch(s,026063);right(s);lmi_direct(s,001571,DLOAD);lmi_dload_op(s,0);return next(s);
 /* STCALL TDEC1 INTSTALL. */
 case 02045u:lmi_store_code(s,034041);lmi_15bitadr(s,027414,CALLITA);lmi_call(s);lmi_goto(s);return EXEC_OK;
 /* CLEAR CALL VINTFLAG SETIFLGS. */
 case 02047u:lmi_newops(s,045014);(void)sw(s,x,001674);right(s);return call(s,026644);
 /* BOF RENDWFLG DOINT2. */
 case 02052u:lmi_newops(s,077614);if(sw(s,x,002756))return branch(s,026060);return next(s);
 /* SET SET DIM0FLAG D6OR9FLG. */
 case 02055u:lmi_newops(s,043014);(void)sw(s,x,001476);right(s);(void)sw(s,x,001475);return next(s);
 /* DOINT2 SET CALL PRECIFLG INTEGRV. */
 case 02060u:lmi_newops(s,045014);(void)sw(s,x,001467);right(s);return call(s,027134);
 /* NO-INT CLRGO NODOFLAG ENDINT. */
 case 02063u:lmi_newops(s,077614);if(!sw(s,x,001236))return EXEC_OUT_OF_SCOPE;return branch(s,026632);
 /* THISVINT CLEAR RVQ VINTFLAG: RVQ returns to the CADR in QPRET, which a
  * CALL leaves as a fixed address. */
 case 02066u:lmi_newops(s,043414);(void)sw(s,x,001674);right(s);
  if(!(W[(W[FIXLOC]+052u)&03777u]&074000u))return EXEC_OUT_OF_SCOPE;
  lmi_unary(s,RVQ);lmi_rvq(s);return EXEC_OK;
 /* R00 (bank 04, after its TC INTPRET) CALL INTSTALL; DUMMYAD EXIT. */
 case 02144u:lmi_newops(s,077624);return call(s,027414);
 case 02146u:lmi_newops(s,077776);lmi_exit(s);return EXEC_OK;
 default:return EXEC_OUT_OF_SCOPE;}
}

const uint16_t lm_stateup_entries[LM_STATEUP_ROUTINES]={02026u,02032u,02036u,02041u,02045u,02047u,02052u,02055u,02060u,02063u,02066u,02144u,02146u};
#define K13(z) (uint16_t)(013u*1024u+((z)-02000u))
#define K04(z) (uint16_t)(04u*1024u+((z)-02000u))
static const agc_routine_entry entries[LM_STATEUP_ROUTINES]={
 {K04(02144),pair},{K04(02146),pair},
 {K13(02026),pair},{K13(02032),pair},{K13(02036),pair},{K13(02041),pair},{K13(02045),pair},{K13(02047),pair},
 {K13(02052),pair},{K13(02055),pair},{K13(02060),pair},{K13(02063),pair},{K13(02066),pair}};
const agc_routine_table lm_stateup_interpretive={entries,LM_STATEUP_ROUTINES};
