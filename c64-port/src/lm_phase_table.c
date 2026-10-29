/* SPDX-License-Identifier: MIT */
/* Luminary099 PHASE_TABLE_MAINTENANCE: NEWMODEX/NEWMODEA/MMDSPLAY/PREBJUMP,
 * CHECKMM, SETUPDSP, 2PHSCHNG, PHASCHNG, PHSCHNGA, PHASJUMP, PHSCHNG2/BELOW1-4, ONEORTWO,
 * OLDPRIO/CON1/TOCON2/GETPRIO/GETNEWNM, CON2 and CHECKB. One agc_image_ops
 * call per original instruction, under labels L<address>; INDEX applied at
 * compile time from the image; inline operands from lm_phase_sites. The instruction sequence is
 * Comanche055's (src/cm_phase_table.c) apart from PHSCHNGA. */
#include "lm_phase_table.h"
#include "agc_image_ops.h"
#include "p66_bias.h"
#include "lm_phase_sites.inc"

#include "agc_image_w.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B10(z) (uint16_t)(010u*1024u+((z)-02000u))
#define FB10 020000u
#define Q 2u
#define BB 6u
#define TIME2 024u
#define TIME1 025u
#define TEMPG 061u
#define TEMPP 062u
#define TEMPNM 063u
#define TEMPBB 064u
#define TEMPSW 065u
#define TEMPSW2 066u
#define TEMPPR 070u
#define TEMPG2 071u
#define TEMPP2 072u
#define TEMPBBCN 073u
#define PHASE1M2 0751u   /* PHASE1 -2 */
#define MPHASE1M2 0750u  /* -PHASE1 -2 */
#define TBASE1M2 01051u
#define PHSPRDT1M2 01052u
#define PHSNAME1M2 01434u
#define MODREG 01011u
#define LONGBASE 01151u
/* Fixed constants (FIXED-FIXED_CONSTANT_POOL and this file). */
#define ONE 1u
#define TWO 2u
#define OCT7 7u
#define OCT17770 017770u
#define BIT7 0100u
#define BIT8 0200u
#define BIT12 04000u
#define BIT15 040000u
#define OCT14000 014000u
#define OCT60000 060000u
#define PRIO30 030000u
#define BIT14COM 017777u
static const agc_word adrpchn2[2]={LM_PHASE_ADRPCHN2};

static p66_bias_io *ctx(const apollo_overlay_state *s,void *io){
 p66_bias_io *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
/* NDX Q; CA 0 (or CS 0, DCA 0): the caller's inline operand. */
static int operand(const agc_native_program *p,const apollo_overlay_state *s,p66_bias_io *x,unsigned k,agc_word *w){
 return agc_transfer_operand(p,s,x->superbank,(agc_word)(W[Q]+k),w);
}
/* DTCB (DXCH Z) at address `at`: Z <- A, BB <- L; A, L <- next Z, old BB. */
static void dtcb(apollo_overlay_state *s,agc_word at){W[5]=(agc_word)(at+1u);ai_dxch(s,5u);}

/* NEWMODEX 5311 / NEWMODEA 5314 / MMDSPLAY 5315 through PREBJUMP's TCF BANKJUMP. */
static enum exec_status mode(const agc_native_program *p,apollo_overlay_state *s,void *io,unsigned entry){
 p66_bias_io *x=ctx(s,io);agc_word w;
 if(!x)return EXEC_OUT_OF_SCOPE;
 if(entry==0u){
  if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
  ai_caf(s,w);                     /* 5311 INDEX Q; 5312 CAF 0 */
  ai_incr(s,Q);                    /* 5313 INCR Q */
 }
 if(entry<=1u&&ai_ts(s,MODREG))return EXEC_OUT_OF_SCOPE; /* 5314 TS MODREG: no caller overflows */
 ai_caf(s,LM_PHASE_SETUPDSP_CADR); /* 5315 CAF +3 */
 ai_lxch(s,BB);                    /* 5316 LXCH BBANK */
 W[5]=LM_PT_BANKJUMP;              /* 5317 TCF BANKJUMP */
 return EXEC_OK;
}
static enum exec_status newmodex(const agc_native_program *p,apollo_overlay_state *s,void *io){return mode(p,s,io,0u);}
static enum exec_status newmodea(const agc_native_program *p,apollo_overlay_state *s,void *io){return mode(p,s,io,1u);}
static enum exec_status mmdsplay(const agc_native_program *p,apollo_overlay_state *s,void *io){return mode(p,s,io,2u);}

/* CHECKMM 5321: INDEX Q; CS 0; AD MODREG; EXTEND; BZF Q+2; TCF Q+1. */
static enum exec_status checkmm(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);agc_word w;
 if(!x||!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
 ai_csf(s,w);ai_ad(s,MODREG);
 W[5]=ai_bzf(s)?LM_PT_Q_PLUS_2:LM_PT_Q_PLUS_1;
 return EXEC_OK;
}

/* SETUPDSP 10,2213: INHINT; DXCH RUPTREG1; CAF PRIO30; TC NOVAC (2CADR DSPMMJOB). */
static enum exec_status setupdsp(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);(void)p;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 x->interrupt_enabled=0;ai_dxch(s,070u);ai_caf(s,PRIO30);ai_tc(s,02217u);W[5]=LM_PT_NOVAC;
 return EXEC_OK;
}
/* 10,2221, NOVAC's return: DXCH RUPTREG1; RELINT; DXCH Z. */
static enum exec_status setupdsp_return(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);(void)p;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,070u);x->interrupt_enabled=1;dtcb(s,02223u);
 return EXEC_OK;
}

/* 2PHSCHNG 5327, PHASCHNG 5353 and PHSCHNGA 5357 (phase word in A), through
 * PHASJUMP's DTCB to PHSCHNG2. */
enum { ENTRY_PHASCHNG, ENTRY_2PHSCHNG, ENTRY_PHSCHNGA };
static enum exec_status phaschng_entry(const agc_native_program *p,apollo_overlay_state *s,void *io,int entry){
 p66_bias_io *x=ctx(s,io);agc_word w;
 if(!x)return EXEC_OUT_OF_SCOPE;
 x->interrupt_enabled=0;            /* INHINT */
 if(entry==ENTRY_2PHSCHNG){
  if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
  ai_caf(s,w);ai_incr(s,Q);         /* 5330-5332 */
  if(ai_ts(s,TEMPP2))return EXEC_OUT_OF_SCOPE;
  ai_maskf(s,OCT7);ai_double(s);
  if(ai_ts(s,TEMPG2))return EXEC_OUT_OF_SCOPE;
  ai_ca(s,TEMPP2);ai_maskf(s,OCT17770);ai_mpf(s,BIT12);ai_xch(s,TEMPP2);ai_maskf(s,BIT15);
  if(ai_ts(s,TEMPSW2))return EXEC_OUT_OF_SCOPE; /* 5345 */
  if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
  ai_caf(s,w);ai_incr(s,Q);         /* 5346-5350 */
  if(ai_ts(s,TEMPSW))return EXEC_OUT_OF_SCOPE; /* 5351; 5352 TCF PHASJUMP */
 }else{
  if(entry==ENTRY_PHASCHNG){
   if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
   ai_caf(s,w);ai_incr(s,Q);        /* 5354-5356 */
  }
  /* PHSCHNGA 5357: INHINT */
  if(ai_ts(s,TEMPSW))return EXEC_OUT_OF_SCOPE; /* 5360: no producer overflows A */
  ai_caf(s,ONE);                    /* 5361 */
  if(ai_ts(s,TEMPSW2))return EXEC_OUT_OF_SCOPE;
 }
 ai_dcaf(s,adrpchn2);dtcb(s,05365u); /* PHASJUMP 5363-5365 */
 return EXEC_OK;
}
static enum exec_status two_phschng(const agc_native_program *p,apollo_overlay_state *s,void *io){return phaschng_entry(p,s,io,ENTRY_2PHSCHNG);}
static enum exec_status phaschng(const agc_native_program *p,apollo_overlay_state *s,void *io){return phaschng_entry(p,s,io,ENTRY_PHASCHNG);}
static enum exec_status phschnga(const agc_native_program *p,apollo_overlay_state *s,void *io){return phaschng_entry(p,s,io,ENTRY_PHSCHNGA);}

/* BELOW1 10,2247 through BELOW4's final DTCB back to the caller. */
static enum exec_status below1(apollo_overlay_state *s,p66_bias_io *x){
 /* TEMPSW2 is 1 (PHASCHNG), +0 or BIT15 (2PHSCHNG); TEMPSW is a multiple
  * of 020000. The -0 branches of these CCSs have no producer. */
 uint8_t c=ai_ccs(s,TEMPSW2);       /* 2247 */
 if(c==3u)return EXEC_OUT_OF_SCOPE;
 if(c==0u)goto L2264;               /* 2250 TCF BELOW2 */
 ai_cs(s,TEMPP2);                   /* 2251 TCF +1; 2252 CS TEMPP2 */
 ai_lxch(s,TEMPP2);                 /* 2253 */
 ai_dxch(s,MPHASE1M2+ai_word(s,TEMPG2)); /* 2254 NDX TEMPG2; 2255 DXCH -PHASE1 -2 */
 c=ai_ccs(s,TEMPSW2);               /* 2256 */
 if(c<2u)goto L2264;                /* 2257 NOOP; 2260 TCF BELOW2 */
 ai_cs(s,TIME1);                    /* 2261 */
 if(ai_ts(s,TBASE1M2+ai_word(s,TEMPG2)))return EXEC_OUT_OF_SCOPE; /* 2262-2263 */
L2264:
 c=ai_ccs(s,TEMPSW);                /* BELOW2 */
 if(c==0u)goto L2300;
 if(c==1u)goto L2303;
 if(c==3u)return EXEC_OUT_OF_SCOPE;
 ai_cs(s,TIME1);                    /* 2267 */
 if(ai_ts(s,TBASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE; /* 2270-2271 */
 ai_ca(s,TEMPSW);ai_adf(s,BIT14COM);/* 2272-2273 */
 c=ai_ccs(s,0u);                    /* 2274 */
 if(c<2u)return EXEC_OUT_OF_SCOPE;  /* 2275-2276: "CAN'T GET HERE" */
 if(c==2u)goto L2303;               /* 2277 TCF BELOW4 */
L2300:
 ai_dca(s,TIME2);ai_dxch(s,LONGBASE); /* BELOW3 */
L2303:
 ai_cs(s,TEMPP);                    /* BELOW4 */
 if(ai_ts(s,MPHASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,Q);ai_lxch(s,TEMPBBCN);x->interrupt_enabled=1;dtcb(s,02311u); /* 2306-2311 */
 return EXEC_OK;
}

/* PHSCHNG2 10,2224: decode TEMPSW; type C stores the phase, types A/B go
 * through ONEORTWO (fixed-fixed) to the DTCB at TOCON2. */
static enum exec_status phschng2(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);agc_word w;uint8_t c;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 ai_lxch(s,TEMPBBCN);ai_ca(s,TEMPSW);ai_maskf(s,OCT7);ai_double(s);   /* 2224-2227 */
 if(ai_ts(s,TEMPG))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPSW);ai_maskf(s,OCT17770);ai_mpf(s,BIT12);                 /* 2231-2234 */
 if(ai_ts(s,TEMPP))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPSW);ai_maskf(s,OCT60000);ai_xch(s,TEMPSW);ai_maskf(s,OCT14000); /* 2236-2241 */
 c=ai_ccs(s,0u);                                                       /* 2242: A >= +0 */
 if(c==1u){                                                            /* type C */
  ai_ca(s,TEMPP);                                                      /* 2244 */
  if(ai_ts(s,PHASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;      /* 2245-2246 */
  return below1(s,x);
 }
 /* ONEORTWO 5370 */
 ai_lxch(s,TEMPBBCN);ai_lxch(s,BB);ai_lxch(s,TEMPBBCN);ai_maskf(s,OCT14000);
 c=ai_ccs(s,0u);                                                       /* 5374: A >= +0 */
 if(c==0u)goto L7750;                                                  /* 5375 TCF CHECKB */
 ai_ca(s,TEMPP);ai_maskf(s,BIT7);                                      /* 5376-5377 */
 c=ai_ccs(s,0u);                                                       /* 5400 */
 if(c==0u)goto L5423;                                                  /* 5401 TCF GETPRIO */
L5402:
 ai_ca(s,PHSPRDT1M2+ai_word(s,TEMPG));                                 /* OLDPRIO */
L5404:
 if(ai_ts(s,TEMPPR))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPP);ai_maskf(s,BIT8);                                      /* CON1 */
 c=ai_ccs(s,0u);
 if(c==0u){                                                            /* GETNEWNM 5427 */
  agc_word pair[2];
  if(!operand(p,s,x,0u,&pair[0])||!operand(p,s,x,1u,&pair[1]))return EXEC_OUT_OF_SCOPE;
  ai_dcaf(s,pair);ai_dxch(s,TEMPNM);ai_caf(s,TWO);ai_ads(s,Q);
 }else{
  ai_ca(s,Q);                                                          /* 5411 */
  if(ai_ts(s,TEMPNM))return EXEC_OUT_OF_SCOPE;                         /* 5412 */
  ai_ca(s,BB);W[0]=(agc_register)(W[0]|x->superbank);                  /* 5413-5415 ROR SUPERBNK */
  if(ai_ts(s,TEMPBB))return EXEC_OUT_OF_SCOPE;
 }
 ai_caf(s,LM_PHASE_CON2ADR);ai_lxch(s,TEMPBBCN);dtcb(s,05421u);        /* TOCON2 */
 return EXEC_OK;
L5423:
 if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;                    /* GETPRIO */
 ai_caf(s,w);ai_incr(s,Q);goto L5404;
L7750:
 ai_maskf(s,BIT12);c=ai_ccs(s,0u);                                     /* CHECKB */
 if(c==0u)goto L5423;
 goto L5402;
}

/* CON2 10,2312: store phase, priority and 2CADR, then BELOW1. */
static enum exec_status con2(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);(void)p;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 ai_lxch(s,TEMPBBCN);ai_ca(s,TEMPP);
 if(ai_ts(s,PHASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPPR);
 if(ai_ts(s,PHSPRDT1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;
 ai_dca(s,TEMPNM);ai_dxch(s,PHSNAME1M2+ai_word(s,TEMPG));
 return below1(s,x);
}

static const agc_routine_entry entries[LM_PT_ROUTINES]={
 {FF(LM_PT_NEWMODEX),newmodex},{FF(LM_PT_NEWMODEA),newmodea},{FF(LM_PT_MMDSPLAY),mmdsplay},
 {FF(LM_PT_CHECKMM),checkmm},{FF(LM_PT_2PHSCHNG),two_phschng},{FF(LM_PT_PHASCHNG),phaschng},
 {FF(LM_PT_PHSCHNGA),phschnga},
 {B10(LM_PT_SETUPDSP),setupdsp},{B10(LM_PT_SETUPDSP_RETURN),setupdsp_return},
 {B10(LM_PT_PHSCHNG2),phschng2},{B10(LM_PT_CON2),con2}};
const agc_routine_table lm_phase_table_routines={entries,LM_PT_ROUTINES};
const agc_site_operand *const lm_phase_table_operands=lm_phase_sites;
