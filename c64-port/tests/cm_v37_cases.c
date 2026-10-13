/* SPDX-License-Identifier: MIT */
#include "cm_v37_cases.h"
#include "cm_interbank_routines.h"
#include "../src/cm_v37_operands.inc"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
/* A major mode: one PREMM1 lists, P00, P20, or any of 0-0177. */
static agc_word mode(uint32_t *seed){unsigned m=upto(seed,8);
 if(m<4u)return (agc_word)(cm_v37_premm1[upto(seed,041u)]&0177u);
 if(m==4u)return 0u;
 if(m==5u)return 024u;
 return (agc_word)upto(seed,0200u);}
void cm_v37_input(apollo_overlay_state *s,cm_fresh_io *io,uint16_t n){
 unsigned i,k=n%CM_V37_ROUTINES,eb,g;uint32_t seed=UINT32_C(0x2011)+(uint32_t)n*UINT32_C(7927);agc_word z=cm_v37_entries[k].z;
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=023;++i)s->words[i]=0;for(i=026;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {cm_fresh_io zero={{0},0,0,0,0};*io=zero;}
 io->control.interrupt_enabled=(agc_word)upto(&seed,2);io->control.channel11=word(&seed);
 io->channel5=word(&seed);io->channel6=word(&seed);io->control.channel12=word(&seed);
 io->control.channel13=(agc_word)(word(&seed)&02767u);io->control.channel14=(agc_word)(word(&seed)&0777u);
 io->control.main_keys=upto(&seed,4)?(agc_word)upto(&seed,040):022u;
 io->control.navigation_keys=word(&seed);io->channel33=word(&seed);
 io->control.trap31a=(agc_word)upto(&seed,2);io->control.trap31b=(agc_word)upto(&seed,2);io->control.trap32=(agc_word)upto(&seed,2);
 /* Restart groups 1-6: PHASEn and -PHASEn agree, n inactive (+0), active or negative. */
 for(g=0;g<6u;++g){unsigned m=upto(&seed,4);agc_word ph=m==0u?0u:m==1u?(agc_word)(1u+upto(&seed,040)):m==2u?word(&seed):(agc_word)(AGC_MASK-upto(&seed,040));
  s->words[0753u+2u*g]=ph;s->words[0752u+2u*g]=(agc_word)(~ph&AGC_MASK);}
 s->words[0161]=(agc_word)upto(&seed,6);s->words[0162]=(agc_word)upto(&seed,7);
 s->words[CM_FS_ERESTORE]=upto(&seed,2)?0u:AGC_MASK;
 s->words[CM_FS_SKEEP4]=(agc_word)(upto(&seed,8)<<8);
 /* V37's request (A), MMNUMBER, MINDEX, MODREG. */
 s->words[CM_V37_MMNUMBER]=mode(&seed);
 if(z==02011u){s->words[0]=mode(&seed);
  /* Mostly an IMU not being initialized and NODOFLAG clear, so V37 gets past CANTR00. */
  if(upto(&seed,8))s->words[01320]&=(agc_word)~CM_V37_K_BIT6;
  if(upto(&seed,8))s->words[076]&=(agc_word)~CM_V37_K_NODOBIT;}
 s->words[CM_V37_MINDEX]=(agc_word)upto(&seed,041u);
 {unsigned m=upto(&seed,4);if(m<2u)s->words[01011]=m?024u:0u;else if(m==2u)s->words[01011]=mode(&seed);}
 /* RAKE's TC MPAC +1 returns where INITSUB was called from. */
 if(z==02364u)s->words[0155]=upto(&seed,2)?02001u:02146u;
 eb=upto(&seed,8);io->control.superbank=(agc_word)(upto(&seed,4)<<4);
 s->words[4]=(agc_word)(cm_v37_entries[k].bank<<10);s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(s->words[4]|eb);s->words[5]=z;
}
/* cm_interbank_routines but ISWRETRN: a KLEAN entry under IBNKCALL returns
 * there with Q = 3, which cm_interbank_complete does not take, so the run ends. */
static agc_routine_entry interbank[16];
static agc_routine_table interbank_table(void){unsigned i;uint16_t n=0;
 for(i=0;i<cm_interbank_routines.count&&n<16u;++i)if(cm_interbank_routines.entries[i].physical!=CM_V37_ISWRETRN_POINT)interbank[n++]=cm_interbank_routines.entries[i];
 {agc_routine_table t;t.entries=interbank;t.count=n;return t;}}
enum exec_status cm_v37_run(apollo_overlay_state *s,cm_fresh_io *io,uint16_t n,uint16_t *steps){
 agc_routine_table tables[3];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=cm_v37_routines;tables[1]=cm_fresh_start_routines;tables[2]=interbank_table();
 program.basic=tables;program.basic_count=3;program.operands=cm_interbank_site_table(&program.operand_count);
 cm_v37_input(s,io,n);
 /* It ends at the first point no table owns, or at SWRETURN. */
 st=agc_transfer_run(&program,s,io,&io->control.superbank,CM_V37_SWRETURN_POINT,32,steps);
 return st==EXEC_UNMAPPED||st==EXEC_OK?EXEC_OK:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_v37_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){cm_fresh_io io;uint16_t steps=0;unsigned i;
 *status=cm_v37_run(s,&io,n,&steps);sum_a=sum_b=0;
 mix(io.control.interrupt_enabled);mix(io.control.superbank);mix(io.control.channel11);mix(io.channel5);mix(io.channel6);
 mix(io.control.channel12);mix(io.control.channel13);mix(io.control.channel14);mix(io.channel33);
 mix(io.control.trap31a);mix(io.control.trap31b);mix(io.control.trap32);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
