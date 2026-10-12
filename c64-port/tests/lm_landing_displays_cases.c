/* SPDX-License-Identifier: MIT */
#include "lm_landing_displays_cases.h"
/* Physical word of an EBANK7 address. */
#define EB7(a) (03400u+((a)-01400u))
#define L LM_LAD_LANDISP
#define R LM_LAD_VARRET
const agc_word lm_lad_entry_z[LM_LAD_ENTRIES]={L,LM_LAD_INTLZE,LM_LAD_TWIDRET,R,L,R,L,R};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word raw(uint32_t *s){return (agc_word)((rnd(s)>>12)&077777u);}
/* A signed value below 2^w for a random w (0-14), +0 or -0 one in eight
 * each. */
static agc_word width(uint32_t *s){unsigned w=(rnd(s)>>20)%15u,k=(rnd(s)>>20)%16u;agc_word v=(agc_word)((rnd(s)>>12)&((1u<<w)-1u));
 if(k<2u)return 0u;
 if(k<4u)return 077777u;
 return (rnd(s)>>24)&1u?(agc_word)(~v&077777u):v;}
static agc_word bit(uint32_t *s,agc_word w,agc_word b){return (rnd(s)>>20)&1u?(agc_word)(w|b):(agc_word)(w&~b&077777u);}
void lm_lad_input(apollo_overlay_state *m,lm_flight_io *io,uint16_t n){uint32_t seed=0x2154u+n;unsigned i,v=n/LM_LAD_ENTRIES,e=n%LM_LAD_ENTRIES;
 for(i=0;i<2048u;++i)m->words[i]=raw(&seed);
 if(v%4u!=0u)for(i=062u;i<2048u;++i)m->words[i]=width(&seed);
 /* Registers and counters zero, except TIME1, the PIPAs and ITEMP1. */
 m->words[7]=0;for(i=020;i<=061u;++i)if(i!=025u&&(i<037u||i>041u))m->words[i]=0;
 m->words[074]=bit(&seed,m->words[074],02u);       /* FLAGWRD0 R10FLBIT */
 m->words[075]=bit(&seed,m->words[075],020000u);   /* FLAGWRD1 DIDFLBIT */
 m->words[0103]=bit(&seed,m->words[0103],02000u);  /* FLAGWRD7 SWANDBIT */
 m->words[01303]=bit(&seed,bit(&seed,m->words[01303],0100u),0200u); /* IMODES33 */
 io->bias.superbank=0;io->bias.interrupt_enabled=0;
 io->chan5=io->chan6=io->chan13=io->chan31=io->chan4=0;io->in_isr=0;
 io->chan12=raw(&seed);io->chan30=bit(&seed,raw(&seed),040u);
 io->chan14=(agc_word)(raw(&seed)&006037u); /* no CDU X/Y/Z or gyro drive bits */
 if((rnd(&seed)>>20)%4u){m->words[EB7(LM_LAD_LADQSAVE)]=(rnd(&seed)>>20)&1u?LM_LAD_ALTROUT+1u:LM_LAD_ALTOUT+1u;}
 m->words[3]=LM_LAD_EB;m->words[4]=LM_LAD_FB21;m->words[6]=(agc_word)(LM_LAD_FB21|(LM_LAD_EB>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[5]=lm_lad_entry_z[e];
}
static int inside(agc_word z){return z>=LM_LAD_FIRST&&z<=LM_LAD_LAST;}
unsigned lm_lad_run(apollo_overlay_state *s,lm_flight_io *io,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;agc_word superbank=0;
 program.basic=&lm_landing_displays_routines;program.basic_count=1;
 program.operands=lm_landing_displays_sites;program.operand_count=LM_LAD_SITES;
 lm_lad_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&superbank,(uint16_t)(AGC_TRANSFER_NONE-1u),4,steps);
 if(st==EXEC_UNMAPPED&&*steps==1u&&(!inside(s->words[5])||(s->words[4]&076000u)!=LM_LAD_FB21))return LM_LAD_LEFT;
 if(st==EXEC_OUT_OF_SCOPE&&*steps==1u)return LM_LAD_REFUSED;
 return LM_LAD_NONE;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_lad_digest(apollo_overlay_state *s,uint16_t n){lm_flight_io io;uint16_t steps;unsigned i,kind=lm_lad_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);mix(io.chan12);mix(io.chan14);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
