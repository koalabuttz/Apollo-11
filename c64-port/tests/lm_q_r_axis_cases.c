/* SPDX-License-Identifier: MIT */
#include "lm_q_r_axis_cases.h"
#define SWRETURN 04631u
/* Physical word of an EBANK6 address. */
#define EB6(a) (03000u+((a)-01400u))
#define C LM_QR_CALLQERR
const agc_word lm_qr_entry_z[LM_QR_ENTRIES]={C,LM_QR_ALARMRET,LM_QR_MINRET,LM_QR_ENTERUVRET,LM_QR_TIMEGMBLRET,
 LM_QR_QERRCALC,LM_QR_TJETRET,LM_QR_SPSRET,LM_QR_WRITEPRET,LM_QR_NOROTATRET,LM_QR_NOTMINRET,LM_QR_WRITEURET,
 LM_QR_WRITEVRET,LM_QR_JTLST,LM_QR_ROTTOUV,LM_QR_SELALARMRET,LM_QR_CLOSEOUT,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word raw(uint32_t *s){return (agc_word)((rnd(s)>>12)&077777u);}
/* A signed value below 2^w for a random w (0-14), +0 or -0 one in eight
 * each. */
static agc_word width(uint32_t *s){unsigned w=(rnd(s)>>20)%15u,k=(rnd(s)>>20)%16u;agc_word v=(agc_word)((rnd(s)>>12)&((1u<<w)-1u));
 if(k<2u)return 0u;
 if(k<4u)return 077777u;
 return (rnd(s)>>24)&1u?(agc_word)(~v&077777u):v;}
static int inside(agc_word q){return (q>=LM_QR_FIRST&&q<=LM_QR_LAST)||q==LM_QR_MAKERUPT||q==LM_QR_MAKERUPT+1u;}
void lm_qr_input(apollo_overlay_state *m,lm_flight_io *io,uint16_t n){uint32_t seed=0x2072u+n;unsigned i,v=n/LM_QR_ENTRIES,e=n%LM_QR_ENTRIES;
 static const agc_word retjadr[3]={02273u,02532u,02671u}; /* MINRTN, BACKHAND, TJLAW +3 */
 agc_word q;
 for(i=0;i<2048u;++i)m->words[i]=raw(&seed);
 if(v%4u!=0u)for(i=060u;i<2048u;++i)m->words[i]=width(&seed);
 /* One cohort in eight: every word +0 or -0 in three quarters. */
 if(v%8u==3u)for(i=060u;i<2048u;++i)if((rnd(&seed)>>20)%4u)m->words[i]=(rnd(&seed)>>24)&1u?077777u:0u;
 /* Registers and counters zero, except the interrupt registers, TIME6 and
  * the CDUs. */
 m->words[7]=0;for(i=020;i<=061u;++i)m->words[i]=0;
 m->words[031]=v%4u?width(&seed):raw(&seed);
 for(i=032;i<=034u;++i)m->words[i]=v%4u?width(&seed):raw(&seed);
 io->bias.superbank=0;io->bias.interrupt_enabled=(uint16_t)((rnd(&seed)>>20)&1u);
 io->chan14=0;io->chan6=io->chan12=io->chan30=0;io->in_isr=0;
 io->chan5=(agc_word)((rnd(&seed)>>20)&0377u);io->chan4=raw(&seed);
 io->chan13=(agc_word)(raw(&seed)&043777u); /* the bits channel 13 reads back */io->chan31=raw(&seed);
 /* The file's state words, in the values its producers store, except in
  * one cohort in 32 (refused where the original would leave the domain). */
 if(v%32u!=31u){
  m->words[EB6(01535u)]=(rnd(&seed)>>20)&1u?4u:0u;m->words[EB6(01536u)]=(rnd(&seed)>>20)&1u?4u:0u;
  {unsigned r=(rnd(&seed)>>20)%16u;if(r<15u)m->words[EB6(01477u)]=retjadr[r%3u];}
  if(m->words[EB6(01502u)]==077777u)m->words[EB6(01502u)]=0u; /* ALLOWGTS */
  if(e<5u||e>16u){
   /* CALLQERR and the returns that store AXISCTR and NUMBERT before reading
    * them: also P-AXIS PURGENCY's AXISCTR -1, GTS's SR in AXISCTR (SAVESR)
    * and PJETSLEC's NUMBERT 5 or 6. */
   unsigned k=(rnd(&seed)>>20)%8u;
   m->words[EB6(01505u)]=k<4u?(agc_word)(k&1u):k<6u?077776u:k==6u?077777u:raw(&seed);
   m->words[EB6(01743u)]=(agc_word)((rnd(&seed)>>20)%7u);
  }else{
   m->words[EB6(01505u)]=(agc_word)((rnd(&seed)>>20)&1u);
   m->words[EB6(01743u)]=(agc_word)((rnd(&seed)>>20)%5u);
  }
  if((v/2u)%4u!=3u)m->words[EB6(01500u)]=(agc_word)((rnd(&seed)>>20)%3u);
 }else if(v%64u==31u)io->chan5=raw(&seed);
 /* Return in Q: SWRETURN, fixed-fixed 06000-07777, or bank 17 outside
  * this file. */
 if(v%3u==0u)q=SWRETURN;
 else if(v%3u==1u)do q=(agc_word)(06000u+(rnd(&seed)>>17)%02000u);while(inside(q));
 else do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(inside(q));
 m->words[2]=q;
 m->words[3]=LM_QR_EB;m->words[4]=LM_QR_FB17;m->words[6]=(agc_word)(LM_QR_FB17|(LM_QR_EB>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[5]=lm_qr_entry_z[e];
}
unsigned lm_qr_run(apollo_overlay_state *s,lm_flight_io *io,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;agc_word superbank=0;
 program.basic=&lm_q_r_axis_routines;program.basic_count=1;program.operands=lm_q_r_axis_sites;program.operand_count=LM_QR_SITES;
 lm_qr_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&superbank,(uint16_t)(AGC_TRANSFER_NONE-1u),4,steps);
 if(st==EXEC_UNMAPPED&&*steps==1u&&(!inside(s->words[5])||(s->words[4]&076000u)!=LM_QR_FB17))return LM_QR_LEFT;
 if(st==EXEC_OUT_OF_SCOPE&&*steps==1u)return LM_QR_REFUSED;
 return LM_QR_NONE;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_qr_digest(apollo_overlay_state *s,uint16_t n){lm_flight_io io;uint16_t steps;unsigned i,kind=lm_qr_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);mix(io.chan13);mix(io.bias.interrupt_enabled);mix(io.in_isr);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
