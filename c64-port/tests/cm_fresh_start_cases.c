/* SPDX-License-Identifier: MIT */
#include "cm_fresh_start_cases.h"
#include "cm_interbank_routines.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
void cm_fs_input(apollo_overlay_state *s,cm_fresh_io *io,uint16_t n){
 unsigned i,k=n%CM_FS_ROUTINES,eb,g;uint32_t seed=UINT32_C(0x2342)+(uint32_t)n*UINT32_C(7907);agc_word fb,z=cm_fresh_entries[k];
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=023;++i)s->words[i]=0;for(i=026;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {cm_fresh_io zero={{0},0,0,0};*io=zero;}
 io->control.interrupt_enabled=(agc_word)upto(&seed,2);io->control.channel11=word(&seed);
 io->channel5=word(&seed);io->channel6=word(&seed);io->control.channel12=word(&seed);
 io->control.channel13=(agc_word)(word(&seed)&02767u);io->control.channel14=(agc_word)(word(&seed)&0777u);
 io->control.main_keys=upto(&seed,4)?(agc_word)upto(&seed,040):022u;
 io->control.navigation_keys=upto(&seed,4)?word(&seed):(agc_word)((word(&seed)&~037u)|0122u);
 io->channel33=word(&seed);
 io->control.trap31a=(agc_word)upto(&seed,2);io->control.trap31b=(agc_word)upto(&seed,2);io->control.trap32=(agc_word)upto(&seed,2);
 /* Restart groups 1-6: PHASEn and -PHASEn agree, n inactive (+0), active or negative. */
 for(g=0;g<6u;++g){unsigned m=upto(&seed,4);agc_word ph=m==0u?0u:m==1u?(agc_word)(1u+upto(&seed,040)):m==2u?word(&seed):(agc_word)(AGC_MASK-upto(&seed,040));
  s->words[0753u+2u*g]=ph;s->words[0752u+2u*g]=(agc_word)(~ph&AGC_MASK);}
 /* A disagreeing pair: -PHASEn with one bit flipped, or equal to PHASEn (RXOR +0). */
 if(upto(&seed,8)==0u){g=upto(&seed,6);if(upto(&seed,2))s->words[0752u+2u*g]^=(agc_word)(1u<<upto(&seed,15));else s->words[0752u+2u*g]=s->words[0753u+2u*g];}
 /* The group scan's counters, as PINACT finds them after RESTARTS. */
 s->words[0161]=(agc_word)upto(&seed,6);s->words[0162]=(agc_word)upto(&seed,7);
 /* ERESTORE: +0, -0, the address under test (SKEEP7 equal) or corrupt. */
 {unsigned m=upto(&seed,4);agc_word a=(agc_word)(061u+upto(&seed,01776u-061u+1u));
  if(m==0u)s->words[CM_FS_ERESTORE]=0;else if(m==1u)s->words[CM_FS_ERESTORE]=AGC_MASK;
  else if(m==2u)s->words[CM_FS_ERESTORE]=s->words[CM_FS_SKEEP7]=a;}
 s->words[CM_FS_SKEEP4]=(agc_word)(upto(&seed,8)<<8);
 eb=upto(&seed,8);
 if(z>=04000u){fb=(agc_word)((4u+upto(&seed,032u))<<10);io->control.superbank=(agc_word)(upto(&seed,2)?0060u:0u);}
 else{fb=(agc_word)(05u<<10);io->control.superbank=(agc_word)(upto(&seed,4)<<4);}
 /* A BANKCALL'd KLEAN entry returns through SWRETURN to its bank 04 caller,
  * or to another fixed-fixed word. */
 if(z==02501u||z==02505u||z==02513u){
  if(upto(&seed,2)){s->words[2]=04675u;s->words[0133]=(agc_word)(02000u+upto(&seed,02000u));s->words[0134]=(agc_word)(04u<<10);}
  else s->words[2]=(agc_word)(07000u+upto(&seed,01000u));}
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[5]=z;
}
enum exec_status cm_fs_run(apollo_overlay_state *s,cm_fresh_io *io,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=cm_fresh_start_routines;tables[1]=cm_interbank_routines;
 program.basic=tables;program.basic_count=2;program.operands=cm_interbank_site_table(&program.operand_count);
 cm_fs_input(s,io,n);
 /* It ends at the first point no table owns, or at SWRETURN. */
 st=agc_transfer_run(&program,s,io,&io->control.superbank,CM_FS_SWRETURN_POINT,24,steps);
 return st==EXEC_UNMAPPED||st==EXEC_OK?EXEC_OK:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_fs_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){cm_fresh_io io;uint16_t steps=0;unsigned i;
 *status=cm_fs_run(s,&io,n,&steps);sum_a=sum_b=0;
 mix(io.control.interrupt_enabled);mix(io.control.superbank);mix(io.control.channel11);mix(io.channel5);mix(io.channel6);
 mix(io.control.channel12);mix(io.control.channel13);mix(io.control.channel14);mix(io.channel33);
 mix(io.control.trap31a);mix(io.control.trap31b);mix(io.control.trap32);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
