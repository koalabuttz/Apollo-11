/* SPDX-License-Identifier: MIT */
#include "t4rupt_core.h"
#include "agc_image_ops.h"
#define W s->words
#define BANKRUPT 016u
#define QRUPT 012u
#define DSPTAB 01023u
#define TIME4 027u
#define RUPTREG1 070u
#define DSRUPTEM 073u
#define FLAGWRD5 0101u
#define DSPCNT 0776u
#define NOUT 01016u
#define SEVEN 7u
#define ONE 1u
#define NEG0 077777u
#define LOW11 03777u
#define HI5 076000u
#define BIT9 0400u
#define BIT14 020000u
#define CHRPRIO 030000u
#define TABLNTH 012u
#define T20MS 037776u            /* 20MRUPT = OCT37776 */
#define T120MS 037764u           /* 120MRUPT, DEC 16372 B-14 */
#define K14_11_9 022400u
#define RELTAB11 060000u
static const agc_word reltab[12]={04025u,010003u,014031u,020033u,024017u,030036u,034034u,040023u,044035u,050037u,054000u,060000u};

/* Per mission: the erasable words that differ, T4JUMP's targets, and the
 * shift of everything from 06,2130 on (Comanche's OPTTEST is two words),
 * and the fixed-fixed routine Q+1 (INDEX Q; TC 1). */
typedef struct {agc_word dsruptsw,imodes33,novac,resume,jump[4],shift,qplus1;} t4_site;
/* LM: TC RCSMONIT, TCF RRAUTCHK, TCF IMUMON, TCF DAPT4S. */
static const t4_site lm_site={01313u,01303u,05072u,05270u,{03156u,03006u,02172u,03132u},0u,06737u};
/* CM: TCF OPTTEST (TC IBNKCALL; CADR OPTDRIVE), TCF OPTMON, IMUMON, RESUME. */
static const t4_site cm_site={01302u,01321u,05134u,05333u,{04740u,02765u,02174u,05333u},2u,06706u};
#define FB06 014000u
#define LATE(z) ((agc_word)((z)+k->shift))   /* an LM address from 06,2130 on */

static agc_word a15(const apollo_overlay_state *s){return agc_correct(W[0]);}
static void write_out0(apollo_overlay_state *s,agc_hardware *h){h->channel10=a15(s);}   /* EXTEND; WRITE OUT0 */
static void tc(apollo_overlay_state *s,agc_word at,agc_word target){ai_tc(s,(agc_word)(at+1u));W[5]=target;}
static void tc_q(apollo_overlay_state *s){W[5]=W[2];W[2]=3u;}
/* INDEX K adds C(K) in ones' complement: -0 indexes as +0. */
static unsigned ndx(agc_word v){return v==077777u?0u:v;}
/* T4JUMP: INDEX RUPTREG1; TCF +1; then the entry at 2120 + RUPTREG1. */
static void t4jump(apollo_overlay_state *s,const t4_site *k,int cm){unsigned r=W[RUPTREG1],i=r&3u;
 if(i!=0u)W[5]=k->jump[i];
 else if(cm)tc(s,02130u,k->jump[0]);                 /* TCF OPTTEST; TC IBNKCALL (CADR OPTDRIVE) */
 else tc(s,(agc_word)(02120u+r),k->jump[0]);         /* TC RCSMONIT */
}
/* SETTIME4 (A = the next T4 interval) and PROCEEDE: TS TIME4; CA IMODES33;
 * RXOR CHAN32; MASK BIT14; BZF T4JUMP; LXCH IMODES33; RXOR LCHAN; TS
 * IMODES33; MASK BIT14; CCS A; TCF T4JUMP; CAF CHRPRIO; TC NOVAC. */
static void settime4(apollo_overlay_state *s,const t4_site *k,const agc_hardware *h,int cm){unsigned c;
 (void)ai_ts(s,TIME4);
 ai_ca(s,k->imodes33);W[0]=agc_extend((agc_word)(a15(s)^h->channel32));ai_maskf(s,BIT14);
 if(ai_bzf(s)){t4jump(s,k,cm);return;}
 ai_lxch(s,k->imodes33);W[0]=(agc_word)(W[0]^W[1]);
 if(!ai_ts(s,k->imodes33))ai_maskf(s,BIT14);
 c=ai_ccs(s,0u);
 if(c==0u){t4jump(s,k,cm);return;}
 if(c==1u)ai_caf(s,CHRPRIO);
 tc(s,02113u,k->novac);                              /* 2CADR PROCKEY follows */
}
/* DSPOUTSB from TC DSPOUTSB at `at` (A = NOUT - 1). Returns 1 when it left
 * through TCF Q+1 (a relay word written to OUT0), 0 through TC Q. Q+1 is
 * the fixed-fixed routine INDEX Q; TC 1, which leaves Q after its TC. */
static int dspoutsb(apollo_overlay_state *s,const t4_site *k,agc_hardware *h,agc_word at){unsigned c;
 ai_tc(s,(agc_word)(at+1u));
 (void)ai_ts(s,NOUT);ai_csf(s,0u);(void)ai_ts(s,DSRUPTEM);ai_xch(s,DSPCNT);
scan2:                                               /* DSPSCAN -2 */
 ai_adf(s,NEG0);
scan1:                                               /* DSPSCAN -1 */
 (void)ai_ts(s,DSPCNT);
 c=ai_ccs(s,DSPTAB+ndx(W[DSPCNT]));                  /* DSPSCAN: INDEX DSPCNT; CCS DSPTAB */
 if(c==1u)goto scan2;
 if(c==2u)goto dsplay;
 c=ai_ccs(s,DSPCNT);
 if(c==0u)goto scan2;
 if(c==1u)goto dsplay;
 c=ai_ccs(s,DSRUPTEM);                               /* DSPCNT -0 (negative has no producer) */
 if(c==1u){(void)ai_ts(s,NOUT);tc_q(s);return 0;}    /* TS NOUT; TC Q */
 if(c==2u){tc_q(s);return 0;}
 (void)ai_ts(s,DSRUPTEM);ai_caf(s,TABLNTH);goto scan1;   /* -0: TS DSRUPTEM; CAF TABLNTH; TCF DSPSCAN -1 */
dsplay:
 ai_adf(s,ONE);(void)ai_ts(s,DSPTAB+ndx(W[DSPCNT]));ai_maskf(s,LOW11);(void)ai_ts(s,DSRUPTEM);
 ai_caf(s,HI5);ai_maskf(s,reltab[ndx(W[DSPCNT])]);ai_ad(s,DSRUPTEM);write_out0(s,h);
 W[5]=(agc_word)(W[2]+1u);W[2]=(agc_word)(k->qplus1+2u);return 1;   /* TCF Q+1: INDEX Q; TC 1 */
}
static void nodspout(apollo_overlay_state *s,const t4_site *k,agc_hardware *h,int cm){write_out0(s,h);ai_caf(s,T120MS);settime4(s,k,h,cm);}
/* HANG20: CS 14,11,9; ADS DSRUPTSW; CAF 20MRUPT; SETTIME4. */
static void hang20(apollo_overlay_state *s,const t4_site *k,agc_hardware *h,int cm){ai_csf(s,K14_11_9);ai_ads(s,k->dsruptsw);ai_caf(s,T20MS);settime4(s,k,h,cm);}
/* DSPOUT from TC DSPOUT at `at`: CCS FLAGWRD5; CAF ZERO; TCF NODSPOUT; CCS
 * NOUT; TC DSPOUTSB; TCF NODSPOUT; then HANG20. */
static void dspout(apollo_overlay_state *s,const t4_site *k,agc_hardware *h,int cm,agc_word at){unsigned c;
 ai_tc(s,(agc_word)(at+1u));
 c=ai_ccs(s,FLAGWRD5);
 if(c==0u)ai_caf(s,0u);
 if(c<2u){nodspout(s,k,h,cm);return;}
 if(c==2u){c=ai_ccs(s,NOUT);
  if(c==1u){nodspout(s,k,h,cm);return;}
  if(c==2u){hang20(s,k,h,cm);return;}
  if(c==3u){ai_ads(s,k->dsruptsw);ai_caf(s,T20MS);settime4(s,k,h,cm);return;}}
 if(dspoutsb(s,k,h,02067u))hang20(s,k,h,cm);else nodspout(s,k,h,cm);
}
/* QUIKDSP, QUIKRUPT, NODSPY, SYNCT4, QUIKOFF: the 20 ms phase, to RESUME. */
static void quikdsp(apollo_overlay_state *s,const t4_site *k,agc_hardware *h){unsigned c;
 ai_caf(s,BIT14);ai_mask(s,k->dsruptsw);
 if(ai_bzf(s)){write_out0(s,h);ai_caf(s,BIT14);goto quikrupt;}     /* QUIKOFF */
 c=ai_ccs(s,NOUT);
 if(c==0u&&dspoutsb(s,k,h,LATE(02141u)))c=2u;
 if(c==2u){ai_csf(s,BIT14);goto quikrupt;}
 if(c==3u)goto quikrupt;
 write_out0(s,h);                                    /* NODSPY */
 for(;;){                                            /* SYNCT4 */
  ai_caf(s,T20MS);ai_ads(s,TIME4);ai_caf(s,BIT9);ai_ads(s,k->dsruptsw);
  c=ai_ccs(s,k->dsruptsw);
  if(c==0u){tc(s,LATE(02161u),k->resume);return;}
  if(c==3u){tc(s,LATE(02164u),k->resume);return;}
  /* +0 first runs OCT37737 as CA 7737, which the CAF overwrites. */
  ai_tc(s,LATE(02164u));                             /* TC SYNCT4 */
 }
quikrupt:
 ai_ads(s,k->dsruptsw);ai_caf(s,T20MS);(void)ai_ts(s,TIME4);ai_caf(s,BIT9);ai_ads(s,k->dsruptsw);
 tc(s,LATE(02151u),k->resume);
}
/* DSPCNT 0-012 or -0, and no display word (DSPTAB 0-10) +0 or -0: fresh
 * start fills them with -BIT12 and DSPIN stores complemented codes (it
 * alarms on +0). A zero word would send DSPSCAN round forever (+0) or into
 * TABLNTH executed as TC 012 (-0). */
static int display_ok(const apollo_overlay_state *s){unsigned i;
 if(W[DSPCNT]>TABLNTH&&W[DSPCNT]!=077777u)return 0;
 for(i=0;i<=10u;++i)if(W[DSPTAB+i]==0u||W[DSPTAB+i]==077777u)return 0;
 return 1;
}
static enum exec_status t4rupt(const agc_native_program *p,apollo_overlay_state *s,const t4_site *k,int cm){agc_word z,d;unsigned c;agc_hardware *h;
 if(!s||!p||!(h=p->hardware)||h->in_interrupt!=1u||W[4]!=FB06)return EXEC_OUT_OF_SCOPE;
 z=W[5];
 if(z==02116u){if(W[RUPTREG1]>7u)return EXEC_OUT_OF_SCOPE;t4jump(s,k,cm);return EXEC_OK;}   /* NOVAC's return */
 if(!cm&&(z==02121u||z==02125u)){W[5]=k->jump[1];return EXEC_OK;}  /* RCSMONIT's return: TCF RRAUTCHK */
 if(cm&&z==02130u){tc(s,02130u,k->jump[0]);return EXEC_OK;}        /* OPTTEST */
 if(z!=02000u)return EXEC_OUT_OF_SCOPE;
 d=W[k->dsruptsw];
 if((!(d&040000u)&&d>8u)||!display_ok(s)||(((W[0]>>15)^(W[0]>>14))&1u))return EXEC_OUT_OF_SCOPE;
 /* T4RUPT: TS BANKRUPT; EXTEND; QXCH QRUPT; CCS DSRUPTSW. */
 (void)ai_ts(s,BANKRUPT);ai_qxch(s,QRUPT);
 c=ai_ccs(s,k->dsruptsw);
 if(c==2u){quikdsp(s,k,h);return EXEC_OK;}
 if(c!=0u)ai_caf(s,SEVEN);                           /* NORMT4 */
 (void)ai_ts(s,RUPTREG1);(void)ai_ts(s,k->dsruptsw);
 /* CDRVE: CCS DSPTAB +11D; TC DSPOUT; TC DSPOUT; XCH DSPTAB +11D; MASK
  * LOW11; TS DSPTAB +11D; AD RELTAB11; EXTEND; WRITE OUT0; TC HANG20. */
 c=ai_ccs(s,DSPTAB+11u);
 if(c<2u){dspout(s,k,h,cm,(agc_word)(02013u+c));return EXEC_OK;}
 if(c==2u)ai_xch(s,DSPTAB+11u);
 ai_maskf(s,LOW11);(void)ai_ts(s,DSPTAB+11u);ai_adf(s,RELTAB11);write_out0(s,h);
 ai_tc(s,02024u);hang20(s,k,h,cm);
 return EXEC_OK;
}
static enum exec_status lm_t4(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)io;return t4rupt(p,s,&lm_site,0);}
static enum exec_status cm_t4(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)io;return t4rupt(p,s,&cm_site,1);}
#define B6(z) (uint16_t)(06u*1024u+((z)-02000u))
static const agc_routine_entry lm_entries[4]={{B6(02000u),lm_t4},{B6(02116u),lm_t4},{B6(02121u),lm_t4},{B6(02125u),lm_t4}};
const agc_routine_table lm_t4rupt_core_routines={lm_entries,4};
/* The same entries for the flight store's T4RUPT module (tools/flight_store_build.py
 * T4PACK), which packs them beside the radar step and RCSMON. */
const agc_routine_table lm_t4rupt_core_hot_routines={lm_entries,4};
static const agc_routine_entry cm_entries[3]={{B6(02000u),cm_t4},{B6(02116u),cm_t4},{B6(02130u),cm_t4}};
const agc_routine_table cm_t4rupt_core_routines={cm_entries,3};
