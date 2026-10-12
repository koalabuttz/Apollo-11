/* SPDX-License-Identifier: MIT */
/* Comanche055 PHASE_TABLE_MAINTENANCE: NEWMODEX/NEWMODEA/MMDSPLAY/PREBJUMP,
 * CHECKMM, SETUPDSP, 2PHSCHNG, PHASCHNG, PHSCHNG2/BELOW1-4, ONEORTWO,
 * OLDPRIO/CON1/TOCON2/GETPRIO/GETNEWNM, CON2 and CHECKB. One agc_image_ops
 * call per original instruction, under labels L<address>; INDEX applied at
 * compile time from the image; inline operands from cm_phase_sites. */
#include "cm_phase_table.h"
#include "agc_image_ops.h"
#include "cm_native_image.h"
#include "cm_phase_sites.inc"

#define W s->words
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
#define LONGBASE 01135u
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
static const agc_word adrpchn2[2]={CM_PHASE_ADRPCHN2};

static cm_native_io *ctx(const apollo_overlay_state *s,void *io){
 cm_native_io *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
/* NDX Q; CA 0 (or CS 0, DCA 0): the caller's inline operand. */
static int operand(const agc_native_program *p,const apollo_overlay_state *s,cm_native_io *x,unsigned k,agc_word *w){
 return agc_transfer_operand(p,s,x->superbank,(agc_word)(W[Q]+k),w);
}
/* DTCB (DXCH Z) at address `at`: Z <- A, BB <- L; A, L <- next Z, old BB. */
static void dtcb(apollo_overlay_state *s,agc_word at){W[5]=(agc_word)(at+1u);ai_dxch(s,5u);}

/* NEWMODEX 5354 / NEWMODEA 5357 / MMDSPLAY 5360 through PREBJUMP's TCF BANKJUMP. */
static enum exec_status mode(const agc_native_program *p,apollo_overlay_state *s,void *io,unsigned entry){
 cm_native_io *x=ctx(s,io);agc_word w;
 if(!x)return EXEC_OUT_OF_SCOPE;
 if(entry==0u){
  if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
  ai_caf(s,w);                     /* 5354 INDEX Q; 5355 CAF 0 */
  ai_incr(s,Q);                    /* 5356 INCR Q */
 }
 if(entry<=1u&&ai_ts(s,MODREG))return EXEC_OUT_OF_SCOPE; /* 5357 TS MODREG: no caller overflows */
 ai_caf(s,CM_PHASE_SETUPDSP_CADR); /* 5360 CAF +3 */
 ai_lxch(s,BB);                    /* 5361 LXCH BBANK */
 W[5]=CM_PT_BANKJUMP;              /* 5362 TCF BANKJUMP */
 return EXEC_OK;
}
static enum exec_status newmodex(const agc_native_program *p,apollo_overlay_state *s,void *io){return mode(p,s,io,0u);}
static enum exec_status newmodea(const agc_native_program *p,apollo_overlay_state *s,void *io){return mode(p,s,io,1u);}
static enum exec_status mmdsplay(const agc_native_program *p,apollo_overlay_state *s,void *io){return mode(p,s,io,2u);}

/* CHECKMM 5364: INDEX Q; CS 0; AD MODREG; EXTEND; BZF Q+2; TCF Q+1. */
static enum exec_status checkmm(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_native_io *x=ctx(s,io);agc_word w;
 if(!x||!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
 ai_csf(s,w);ai_ad(s,MODREG);
 W[5]=ai_bzf(s)?CM_PT_Q_PLUS_2:CM_PT_Q_PLUS_1;
 return EXEC_OK;
}

/* SETUPDSP 10,2440: INHINT; DXCH RUPTREG1; CAF PRIO30; TC NOVAC (2CADR DSPMMJOB). */
static enum exec_status setupdsp(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_native_io *x=ctx(s,io);(void)p;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 x->interrupt_enabled=0;ai_dxch(s,070u);ai_caf(s,PRIO30);ai_tc(s,02444u);W[5]=CM_PT_NOVAC;
 return EXEC_OK;
}
/* 10,2446, NOVAC's return: DXCH RUPTREG1; RELINT; DXCH Z. */
static enum exec_status setupdsp_return(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_native_io *x=ctx(s,io);(void)p;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,070u);x->interrupt_enabled=1;dtcb(s,02450u);
 return EXEC_OK;
}

/* 2PHSCHNG 5372 and PHASCHNG 5412, through the DTCB to PHSCHNG2. */
static enum exec_status phaschng_entry(const agc_native_program *p,apollo_overlay_state *s,void *io,int two){
 cm_native_io *x=ctx(s,io);agc_word w;
 if(!x)return EXEC_OUT_OF_SCOPE;
 x->interrupt_enabled=0;            /* INHINT */
 if(two){
  if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
  ai_caf(s,w);ai_incr(s,Q);         /* 5373-5375 */
  if(ai_ts(s,TEMPP2))return EXEC_OUT_OF_SCOPE;
  ai_maskf(s,OCT7);ai_double(s);
  if(ai_ts(s,TEMPG2))return EXEC_OUT_OF_SCOPE;
  ai_ca(s,TEMPP2);ai_maskf(s,OCT17770);ai_mpf(s,BIT12);ai_xch(s,TEMPP2);ai_maskf(s,BIT15);
  if(ai_ts(s,TEMPSW2))return EXEC_OUT_OF_SCOPE; /* 5410; 5411 TCF PHASCHNG +3 */
 }else{
  ai_caf(s,ONE);                    /* 5413 */
  if(ai_ts(s,TEMPSW2))return EXEC_OUT_OF_SCOPE;
 }
 if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,w);ai_incr(s,Q);          /* 5415-5417 */
 if(ai_ts(s,TEMPSW))return EXEC_OUT_OF_SCOPE;
 ai_dcaf(s,adrpchn2);dtcb(s,05423u); /* 5421-5423 */
 return EXEC_OK;
}
static enum exec_status two_phschng(const agc_native_program *p,apollo_overlay_state *s,void *io){return phaschng_entry(p,s,io,1);}
static enum exec_status phaschng(const agc_native_program *p,apollo_overlay_state *s,void *io){return phaschng_entry(p,s,io,0);}

/* BELOW1 10,2474 through BELOW4's final DTCB back to the caller. */
static enum exec_status below1(apollo_overlay_state *s,cm_native_io *x){
 /* TEMPSW2 is 1 (PHASCHNG), +0 or BIT15 (2PHSCHNG); TEMPSW is a multiple
  * of 020000. The -0 branches of these CCSs have no producer. */
 uint8_t c=ai_ccs(s,TEMPSW2);       /* 2474 */
 if(c==3u)return EXEC_OUT_OF_SCOPE;
 if(c==0u)goto L2511;               /* 2475 TCF BELOW2 */
 ai_cs(s,TEMPP2);                   /* 2476 TCF +1; 2477 CS TEMPP2 */
 ai_lxch(s,TEMPP2);                 /* 2500 */
 ai_dxch(s,MPHASE1M2+ai_word(s,TEMPG2)); /* 2501 NDX TEMPG2; 2502 DXCH -PHASE1 -2 */
 c=ai_ccs(s,TEMPSW2);               /* 2503 */
 if(c<2u)goto L2511;                /* 2504 NOOP; 2505 TCF BELOW2 */
 ai_cs(s,TIME1);                    /* 2506 */
 if(ai_ts(s,TBASE1M2+ai_word(s,TEMPG2)))return EXEC_OUT_OF_SCOPE; /* 2507-2510 */
L2511:
 c=ai_ccs(s,TEMPSW);                /* BELOW2 */
 if(c==0u)goto L2525;
 if(c==1u)goto L2530;
 if(c==3u)return EXEC_OUT_OF_SCOPE;
 ai_cs(s,TIME1);                    /* 2514 */
 if(ai_ts(s,TBASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE; /* 2515-2516 */
 ai_ca(s,TEMPSW);ai_adf(s,BIT14COM);/* 2517-2520 */
 c=ai_ccs(s,0u);                    /* 2521 */
 if(c<2u)return EXEC_OUT_OF_SCOPE;  /* 2522-2523: "CAN'T GET HERE" */
 if(c==2u)goto L2530;               /* 2524 TCF BELOW4 */
L2525:
 ai_dca(s,TIME2);ai_dxch(s,LONGBASE); /* BELOW3 */
L2530:
 ai_cs(s,TEMPP);                    /* BELOW4 */
 if(ai_ts(s,MPHASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,Q);ai_lxch(s,TEMPBBCN);x->interrupt_enabled=1;dtcb(s,02536u); /* 2533-2536 */
 return EXEC_OK;
}

/* PHSCHNG2 10,2451: decode TEMPSW; type C stores the phase, types A/B go
 * through ONEORTWO (fixed-fixed) to the DTCB at TOCON2. */
static enum exec_status phschng2(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_native_io *x=ctx(s,io);agc_word w;uint8_t c;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 ai_lxch(s,TEMPBBCN);ai_ca(s,TEMPSW);ai_maskf(s,OCT7);ai_double(s);   /* 2451-2454 */
 if(ai_ts(s,TEMPG))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPSW);ai_maskf(s,OCT17770);ai_mpf(s,BIT12);                 /* 2456-2461 */
 if(ai_ts(s,TEMPP))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPSW);ai_maskf(s,OCT60000);ai_xch(s,TEMPSW);ai_maskf(s,OCT14000); /* 2463-2466 */
 c=ai_ccs(s,0u);                                                       /* 2467: A >= +0 */
 if(c==1u){                                                            /* type C */
  ai_ca(s,TEMPP);                                                      /* 2471 */
  if(ai_ts(s,PHASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;      /* 2472-2473 */
  return below1(s,x);
 }
 /* ONEORTWO 5426 */
 ai_lxch(s,TEMPBBCN);ai_lxch(s,BB);ai_lxch(s,TEMPBBCN);ai_maskf(s,OCT14000);
 c=ai_ccs(s,0u);                                                       /* 5432: A >= +0 */
 if(c==0u)goto L5474;                                                  /* 5433 TCF CHECKB */
 ai_ca(s,TEMPP);ai_maskf(s,BIT7);                                      /* 5434-5435 */
 c=ai_ccs(s,0u);                                                       /* 5436 */
 if(c==0u)goto L5461;                                                  /* 5437 TCF GETPRIO */
L5440:
 ai_ca(s,PHSPRDT1M2+ai_word(s,TEMPG));                                 /* OLDPRIO */
L5442:
 if(ai_ts(s,TEMPPR))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPP);ai_maskf(s,BIT8);                                      /* CON1 */
 c=ai_ccs(s,0u);
 if(c==0u){                                                            /* GETNEWNM 5465 */
  agc_word pair[2];
  if(!operand(p,s,x,0u,&pair[0])||!operand(p,s,x,1u,&pair[1]))return EXEC_OUT_OF_SCOPE;
  ai_dcaf(s,pair);ai_dxch(s,TEMPNM);ai_caf(s,TWO);ai_ads(s,Q);
 }else{
  ai_ca(s,Q);                                                          /* 5447 */
  if(ai_ts(s,TEMPNM))return EXEC_OUT_OF_SCOPE;                         /* 5450 */
  ai_ca(s,BB);W[0]=(agc_register)(W[0]|x->superbank);                  /* 5451-5453 ROR SUPERBNK */
  if(ai_ts(s,TEMPBB))return EXEC_OUT_OF_SCOPE;
 }
 ai_caf(s,CM_PHASE_CON2ADR);ai_lxch(s,TEMPBBCN);dtcb(s,05457u);        /* TOCON2 */
 return EXEC_OK;
L5461:
 if(!operand(p,s,x,0u,&w))return EXEC_OUT_OF_SCOPE;                    /* GETPRIO */
 ai_caf(s,w);ai_incr(s,Q);goto L5442;
L5474:
 ai_maskf(s,BIT12);c=ai_ccs(s,0u);                                     /* CHECKB */
 if(c==0u)goto L5461;
 goto L5440;
}

/* CON2 10,2537: store phase, priority and 2CADR, then BELOW1. */
static enum exec_status con2(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_native_io *x=ctx(s,io);(void)p;
 if(!x||W[4]!=FB10)return EXEC_OUT_OF_SCOPE;
 ai_lxch(s,TEMPBBCN);ai_ca(s,TEMPP);
 if(ai_ts(s,PHASE1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TEMPPR);
 if(ai_ts(s,PHSPRDT1M2+ai_word(s,TEMPG)))return EXEC_OUT_OF_SCOPE;
 ai_dca(s,TEMPNM);ai_dxch(s,PHSNAME1M2+ai_word(s,TEMPG));
 return below1(s,x);
}

static const agc_routine_entry entries[CM_PT_ROUTINES]={
 {FF(CM_PT_NEWMODEX),newmodex},{FF(CM_PT_NEWMODEA),newmodea},{FF(CM_PT_MMDSPLAY),mmdsplay},
 {FF(CM_PT_CHECKMM),checkmm},{FF(CM_PT_2PHSCHNG),two_phschng},{FF(CM_PT_PHASCHNG),phaschng},
 {B10(CM_PT_SETUPDSP),setupdsp},{B10(CM_PT_SETUPDSP_RETURN),setupdsp_return},
 {B10(CM_PT_PHSCHNG2),phschng2},{B10(CM_PT_CON2),con2}};
const agc_routine_table cm_phase_table_routines={entries,CM_PT_ROUTINES};
const agc_site_operand *const cm_phase_table_operands=cm_phase_sites;
