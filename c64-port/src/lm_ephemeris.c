/* SPDX-License-Identifier: MIT */
/* Native Luminary099 LSPOS/LUNPOS and LUNVEL
 * (Luminary099/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc, bank 15 words
 * 03664-03775). Every interpretive pair is compiled: its original order and
 * operand words appear below only as the literal bookkeeping arguments of
 * the lm_interp_steps paths. Arithmetic runs in the existing frozen native
 * bodies (time scale, TAD, DDV, scalar shift, DMP/DAD/DSU, SINE/COSINE,
 * MXV and UNIT). Constants come from the certified fixed-data tables. */
#include "lm_ephemeris.h"
#include "lm_interp_steps.h"
#include "ephemeris_time_scale.h"
#include "ephemeris_add.h"
#include "ephemeris_day.h"
#include "ephemeris_shift.h"
#include "sband_scalar.h"
#include "sband_trig.h"
#include "matrix_native.h"
#include "rtb_unit.h"
#ifdef LM_EPHEMERIS_TRACE
void lm_ephemeris_trace(const apollo_overlay_state *);
#define TRACE(s) lm_ephemeris_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#define W s->words
#define LM APOLLO_FIXED_LM

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {DLOAD=06052,SLOAD=06501,PDDL=06523,TAD=07071,DDV=07577,GSHIFT=07621,SETPD=07613,
 DSU=07005,DAD=06775,DMP=07572,MXV=07334,AXT=02344,INCR=02406,BZEGOTO=02467,BPLBMN=02474,
 SWITCH=02516,SINE=03530,COSINE=03517,VDEF=03232,UNIT=03023,RVQ=03274};

/* Original DP fixed operand selected by ADDRWD in the current FBANK. */
static uint8_t fixed_pair(const apollo_overlay_state *s,agc_word out[2]){
 uint16_t at=(uint16_t)((W[4]>>10)*1024u+(W[0116]-02000u));
 return W[0116]>=02000u&&W[0116]<03777u&&apollo_fixed_data_read(LM,at,&out[0])&&apollo_fixed_data_read(LM,(uint16_t)(at+1u),&out[1]);
}
static uint8_t scalar(apollo_overlay_state *s,enum sband_scalar_operation op,uint8_t fixed){
 agc_word k[2];
 if(!fixed)return sband_scalar(s,LM,op,0);
 return fixed_pair(s,k)&&sband_scalar(s,LM,op,k);
}
static uint8_t dload_fixed(apollo_overlay_state *s){agc_word k[2];if(!fixed_pair(s,k))return 0;lmi_dload_fixed(s,k);return 1;}
static uint8_t matrix(apollo_overlay_state *s){
 agc_word k[18];unsigned i;uint16_t at=(uint16_t)((W[4]>>10)*1024u+(W[0116]-02000u));
 for(i=0;i<18u;++i)if(!apollo_fixed_data_read(LM,(uint16_t)(at+i),&k[i]))return 0;
 for(i=0;i<4u;++i)if(!matrix_native_phase(s,LM,0,k,(uint8_t)i))return 0;
 return 1;
}
static uint8_t unit(apollo_overlay_state *s,p66_bias_io *io){
 enum rtb_unit_status st;unsigned i;
 if(rtb_unit_begin(s,io,RTB_LM)!=RTB_UNIT_OK)return 0;
 for(i=0;i<3u;++i){st=rtb_unit_phase(s,io,RTB_LM,(uint8_t)i);if(st==RTB_UNIT_ABORT_1302)return 2;if(st==RTB_UNIT_OUT_OF_SCOPE)return 0;if(st==RTB_UNIT_OVERFLOW)break;}
 return 1;
}
/* SIN or COS (left) with a PDDL of a work-area pair (right). */
static uint8_t trig_pddl(apollo_overlay_state *s,agc_word pair,uint8_t cosine,agc_word operand){
 lmi_newops(s,pair);lmi_unary(s,cosine?COSINE:SINE);if(!sband_trig(s,LM,cosine))return 0;
 lmi_danzig_right(s);lmi_direct(s,operand,PDDL);lmi_pddl(s);lmi_danzig_next(s);TRACE(s);return 1;
}
/* COS VDEF, MXV KONMAT UNIT and STORE: one unit vector (VMOON or VSUN). */
static uint8_t vector(apollo_overlay_state *s,p66_bias_io *io,agc_word store){
 uint8_t st;
 lmi_newops(s,055546);lmi_unary(s,COSINE);if(!sband_trig(s,LM,1))return 0;
 lmi_danzig_right(s);lmi_unary(s,VDEF);lmi_vdef(s);lmi_danzig_next(s);TRACE(s);
 lmi_newops(s,053521);lmi_direct(s,012002,MXV);if(!matrix(s))return 0;
 lmi_danzig_right(s);lmi_unary(s,UNIT);st=unit(s,io);if(st!=1u)return st;lmi_danzig_next(s);TRACE(s);
 lmi_store(s,store);lmi_danzig_next(s);TRACE(s);return 1;
}

/* The frozen GENSHIFT kernels admit only their qualified descriptor form
 * 020200+count (020616 for SR14). GENSHIFT reads ADDRWD through MASK LOW7
 * and MP BIT8 (and the left path's rotation); the bank-15 caller's actual
 * word, 02000+(operand-1)&01777, yields the same count, direction and
 * register debris. The actual source word is restored afterwards. */
static uint8_t shift(apollo_overlay_state *s,uint8_t right){
 agc_word actual=W[0116];uint8_t ok;
 W[0116]=(agc_word)(right?020616u:(020200u|(actual&0177u)));
 ok=right?ephemeris_time_downscale(s,LM):ephemeris_scalar_left(s,LM);
 W[0116]=actual;return ok;
}
static uint8_t admitted(const apollo_overlay_state *s,const p66_bias_io *io){
 static const agc_word returns[5]={022563,023273,023373,030512,065627};
 unsigned i,vac=W[0120];agc_word q;uint8_t known=0;
 if(W[5]!=06070u||W[4]!=032000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(032000u|(W[3]>>8))||W[0165]!=W[6]||W[7]||W[023]||W[067]||W[0115]||W[0163])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 q=W[vac+052u];
 if(W[0164]==03775u)known=q==023417u;
 else if(W[0164]==03664u)for(i=0;i<5u;++i)known|=(uint8_t)(q==returns[i]);
 return known;
}

uint8_t lm_ephemeris(apollo_overlay_state *s,p66_bias_io *io){
 agc_word k[2];uint8_t st;
 if(!s||!io||!admitted(s,io))return 0;
 if(W[0164]==03775u){
  /* LUNVEL RVQ (to fool integration). */
  lmi_newops(s,077616);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);return 1;
 }
 /* LSPOS SETPD 0; SR 14D. */
 lmi_newops(s,054201);lmi_direct(s,000001,SETPD);lmi_setpd(s);
 lmi_noibnksw_right(s);lmi_direct(s,020617,GSHIFT);lmi_gshift_prologue(s);
 if(!shift(s,1))return 0;
 lmi_danzig_next(s);TRACE(s);
 /* TAD TEPHEM; DDV CSTODAY. */
 lmi_newops(s,056371);lmi_direct(s,001707,TAD);if(!ephemeris_triple_add(s,LM))return 0;
 lmi_danzig_right(s);lmi_direct(s,012024,DDV);if(!fixed_pair(s,k))return 0;lmi_ddv_prologue(s,k);
 if(!ephemeris_day_divide(s,LM))return 0;
 lmi_danzig_next(s);TRACE(s);
 /* STORE TIMEP. */
 lmi_store(s,000031);lmi_danzig_next(s);TRACE(s);
 /* AXT,1 0; AXT,2 0. */
 lmi_newops(s,077170);lmi_misc(s,0,AXT);lmi_axt(s);lmi_danzig_right(s);lmi_misc(s,0,AXT);lmi_axt(s);lmi_danzig_next(s);TRACE(s);
 /* CLEAR FREEFLAG. */
 lmi_newops(s,077614);lmi_misc(s,000274,SWITCH);(void)lmi_switch(s,io);lmi_danzig_next(s);TRACE(s);
 for(;;){
  /* POSITA DLOAD KONMAT +2 (zero); STORE GTMP. */
  lmi_newops(s,077745);lmi_direct(s,012004,DLOAD);if(!dload_fixed(s))return 0;lmi_danzig_next(s);TRACE(s);
  lmi_store(s,000027);lmi_danzig_next(s);TRACE(s);
  do{
   /* POSITB DLOAD TIMEP; DMP* VAL67 +4,1. */
   lmi_newops(s,040745);lmi_direct(s,000031,DLOAD);lmi_dload(s);
   lmi_danzig_right(s);lmi_indexed(s,012050,DMP);if(!scalar(s,SBAND_DMP,1))return 0;lmi_danzig_next(s);TRACE(s);
   /* SL 8D; DAD* VAL67 +2,1. */
   lmi_newops(s,042661);lmi_direct(s,020211,GSHIFT);lmi_gshift_prologue(s);if(!shift(s,0))return 0;
   lmi_danzig_right(s);lmi_indexed(s,012046,DAD);if(!scalar(s,SBAND_DAD,1))return 0;lmi_danzig_next(s);TRACE(s);
   /* SIN; DMP* VAL67,1. */
   lmi_newops(s,040756);lmi_unary(s,SINE);if(!sband_trig(s,LM,0))return 0;
   lmi_danzig_right(s);lmi_indexed(s,012044,DMP);if(!scalar(s,SBAND_DMP,1))return 0;lmi_danzig_next(s);TRACE(s);
   /* DAD GTMP; INCR,1 -6. */
   lmi_newops(s,062015);lmi_direct(s,000027,DAD);if(!scalar(s,SBAND_DAD,0))return 0;
   lmi_danzig_right(s);lmi_misc(s,077771,INCR);lmi_incr(s);lmi_danzig_next(s);TRACE(s);
   /* STORE GTMP. */
   lmi_store(s,000027);lmi_danzig_next(s);TRACE(s);
   /* BOFSET FREEFLAG POSITB. */
   lmi_newops(s,077614);lmi_misc(s,000054,SWITCH);
   if(lmi_switch(s,io)){lmi_swbranch(s,033703);TRACE(s);continue;}
   lmi_danzig_next(s);TRACE(s);break;
  }while(1);
  for(;;){
   /* POSITD DLOAD TIMEP; DMP* RATESP,2. */
   lmi_newops(s,040745);lmi_direct(s,000031,DLOAD);lmi_dload(s);
   lmi_danzig_right(s);lmi_indexed(s,065747,DMP);if(!scalar(s,SBAND_DMP,1))return 0;lmi_danzig_next(s);TRACE(s);
   /* SL 5D; DAD* RATESP +6,2. */
   lmi_newops(s,042661);lmi_direct(s,020206,GSHIFT);lmi_gshift_prologue(s);if(!shift(s,0))return 0;
   lmi_danzig_right(s);lmi_indexed(s,065741,DAD);if(!scalar(s,SBAND_DAD,1))return 0;lmi_danzig_next(s);TRACE(s);
   /* DSU GTMP. */
   lmi_newops(s,077625);lmi_direct(s,000027,DSU);if(!scalar(s,SBAND_DSU,0))return 0;lmi_danzig_next(s);TRACE(s);
   /* STORE STMP,2. */
   lmi_store(s,010021);lmi_danzig_next(s);TRACE(s);
   /* SLOAD X2; INCR,2 -2. */
   lmi_newops(s,063135);lmi_direct(s,000050,SLOAD);lmi_sload(s);
   lmi_danzig_right(s);lmi_misc(s,077775,INCR);lmi_incr(s);lmi_danzig_next(s);TRACE(s);
   /* DAD RCB-13; BZE POSITE. */
   lmi_newops(s,053015);lmi_direct(s,012026,DAD);if(!scalar(s,SBAND_DAD,1))return 0;
   lmi_danzig_right(s);lmi_misc(s,033770,BZEGOTO);
   if(lmi_bze(s)){
    TRACE(s);
    /* POSITE DLOAD KONMAT +2; STORE GTMP; GOTO POSITD. */
    lmi_newops(s,077745);lmi_direct(s,012004,DLOAD);if(!dload_fixed(s))return 0;lmi_danzig_next(s);TRACE(s);
    lmi_store(s,000027);lmi_danzig_next(s);TRACE(s);
    lmi_newops(s,077650);lmi_misc(s,033722,BZEGOTO);lmi_goto_order(s);TRACE(s);continue;
   }
   lmi_danzig_next(s);TRACE(s);
   /* BPL POSITA. */
   lmi_newops(s,077644);lmi_misc(s,033700,BPLBMN);
   if(lmi_bpl(s)){TRACE(s);break;}
   lmi_danzig_next(s);TRACE(s);
   /* POSITF DLOAD STMP; DSU STMP +4; SIN PDDL STMP; SIN PDDL STMP. */
   lmi_newops(s,045345);lmi_direct(s,000021,DLOAD);lmi_dload(s);
   lmi_danzig_right(s);lmi_direct(s,000025,DSU);if(!scalar(s,SBAND_DSU,0))return 0;lmi_danzig_next(s);TRACE(s);
   if(!trig_pddl(s,065356,0,000021)||!trig_pddl(s,065356,0,000021))return 0;
   /* COS VDEF; MXV KONMAT UNIT; STORE VMOON. */
   if((st=vector(s,io,002723))!=1u)return st;
   /* DLOAD KONMAT +2; PDDL STMP +2; SIN PDDL STMP +2. */
   lmi_newops(s,065345);lmi_direct(s,012004,DLOAD);if(!dload_fixed(s))return 0;
   lmi_danzig_right(s);lmi_direct(s,000023,PDDL);lmi_pddl(s);lmi_danzig_next(s);TRACE(s);
   if(!trig_pddl(s,065356,0,000023))return 0;
   /* COS VDEF; MXV KONMAT UNIT; STORE VSUN. */
   if((st=vector(s,io,002715))!=1u)return st;
   /* RVQ. */
   lmi_newops(s,077616);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);return 1;
  }
 }
}
