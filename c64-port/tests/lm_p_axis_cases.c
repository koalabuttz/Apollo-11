/* SPDX-License-Identifier: MIT */
#include "lm_p_axis_cases.h"
#define SWRETURN 04631u
/* Physical word of an EBANK6 address. */
#define EB6(a) (03000u+((a)-01400u))
const agc_word lm_pa_entry_z[LM_PA_ENTRIES]={LM_PA_PAXIS,LM_PA_CHEKBITS2,LM_PA_AFTERCHEK,LM_PA_BACKP,LM_PA_AFTERACDT,
 LM_PA_SUPERJOB,LM_PA_ABORTYZ2,LM_PA_ZATTRET,LM_PA_WRITEPRET,LM_PA_JTLSTRET,LM_PA_JETSOFFRET,LM_PA_CALCPERR,LM_PA_SPSRET,
 LM_PA_TJETRET,LM_PA_SELALARM,LM_PA_BACKP};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word raw(uint32_t *s){return (agc_word)((rnd(s)>>12)&077777u);}
/* A signed value below 2^w for a random w (0-14), +0 or -0 one in eight
 * each. */
static agc_word width(uint32_t *s){unsigned w=(rnd(s)>>20)%15u,k=(rnd(s)>>20)%16u;agc_word v=(agc_word)((rnd(s)>>12)&((1u<<w)-1u));
 if(k<2u)return 0u;
 if(k<4u)return 077777u;
 return (rnd(s)>>24)&1u?(agc_word)(~v&077777u):v;}
static int inside(agc_word q){return q>=LM_PA_FIRST&&q<=LM_PA_LAST;}
void lm_pa_input(apollo_overlay_state *m,lm_flight_io *io,uint16_t n){uint32_t seed=0x2213u+n;unsigned i,v=n/LM_PA_ENTRIES,e=n%LM_PA_ENTRIES;
 agc_word q;
 for(i=0;i<2048u;++i)m->words[i]=raw(&seed);
 if(v%4u!=0u)for(i=060u;i<2048u;++i)m->words[i]=width(&seed);
 /* One cohort in eight: every word +0 or -0 in three quarters. */
 if(v%8u==3u)for(i=060u;i<2048u;++i)if((rnd(&seed)>>20)%4u)m->words[i]=(rnd(&seed)>>24)&1u?077777u:0u;
 /* Registers and counters zero, except the interrupt registers, TIME5,
  * the CDUs and the RHC counters. */
 m->words[7]=0;for(i=020;i<=061u;++i)m->words[i]=0;
 m->words[030]=raw(&seed);
 for(i=032;i<=034u;++i)m->words[i]=v%4u?width(&seed):raw(&seed);
 for(i=042;i<=044u;++i)m->words[i]=v%4u?width(&seed):raw(&seed);
 io->bias.superbank=0;io->bias.interrupt_enabled=(uint16_t)((rnd(&seed)>>20)&1u);
 io->chan14=0;io->chan5=io->chan6=io->chan30=0;
 io->chan12=raw(&seed);io->chan13=(agc_word)(raw(&seed)&043777u); /* the bits channel 13 reads back */io->chan31=raw(&seed);
 if(e==8u&&v%8u!=7u)m->words[EB6(01744u)]=(agc_word)(v%2u);
 /* One BACKP cohort in 32 (in the +-0 cohort) with the rate command inputs
  * that make PEGI's EDOTP +0: OMEGAP and DAPTEMP1 +0 (so JETRATE is +0),
  * 1JACC positive, the trap thresholds -POSMAX (no trap update), the RHC
  * counter +0 and STIKSENS negative (PLAST -0), on the path through
  * CHKVISFZ (RCSFLAGS bit 12, no translation in channel 31 bits 8-11) and
  * RHCMOVED (channel 31 bit 13 set, bit 15 clear; DAPBOOLS OURRCBIT set,
  * XOVINHIB and CSMDOCKD clear) to RATERROR. */
 if((e==3u||e==15u)&&v%32u==19u){
  m->words[EB6(01421u)]=0u;m->words[EB6(01737u)]=0u;m->words[EB6(01530u)]=(agc_word)(1u+(rnd(&seed)>>20)%037777u);
  m->words[EB6(01403u)]=m->words[EB6(01406u)]=040000u;m->words[043]=0u;
  m->words[EB6(01444u)]=(agc_word)(040000u|(rnd(&seed)>>20)%037777u);
  m->words[01273u]|=04000u;m->words[0111u]=(agc_word)((m->words[0111u]&~010400u)|04000u);
  io->chan31=(agc_word)((io->chan31|010000u|07400u)&~040000u);}
 /* OLDPMIN -0 (refused) only in one cohort in 32. */
 if(m->words[EB6(01460u)]==077777u&&v%32u!=31u)m->words[EB6(01460u)]=0u;
 /* Return in Q: SWRETURN, fixed-fixed 06000-07777, or bank 16 outside
  * this file. */
 if(v%3u==0u)q=SWRETURN;
 else if(v%3u==1u)q=(agc_word)(06000u+(rnd(&seed)>>17)%02000u);
 else do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(inside(q));
 m->words[2]=q;
 m->words[3]=LM_PA_EB;m->words[4]=LM_PA_FB16;m->words[6]=(agc_word)(LM_PA_FB16|(LM_PA_EB>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[5]=lm_pa_entry_z[e];
}
unsigned lm_pa_run(apollo_overlay_state *s,lm_flight_io *io,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;agc_word superbank=0;
 program.basic=&lm_p_axis_routines;program.basic_count=1;program.operands=lm_p_axis_sites;program.operand_count=LM_PA_SITES;
 lm_pa_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&superbank,(uint16_t)(AGC_TRANSFER_NONE-1u),4,steps);
 if(st==EXEC_UNMAPPED&&*steps==1u&&(!inside(s->words[5])||(s->words[4]&076000u)!=LM_PA_FB16))return LM_PA_LEFT;
 if(st==EXEC_OUT_OF_SCOPE&&*steps==1u)return LM_PA_REFUSED;
 return LM_PA_NONE;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_pa_digest(apollo_overlay_state *s,uint16_t n){lm_flight_io io;uint16_t steps;unsigned i,kind=lm_pa_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);mix(io.chan12);mix(io.chan13);mix(io.bias.interrupt_enabled);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
