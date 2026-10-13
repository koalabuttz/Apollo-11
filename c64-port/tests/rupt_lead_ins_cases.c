/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "rupt_lead_ins_cases.h"
#define RESUME_ROPE_CM
#include "resume_rope_words.h"
static const agc_word lm_z[9]={04000,04010,04020,04024,04030,04034,04040,04044,04050};
static const agc_word cm_z[15]={04000,04004,04010,04020,04024,04030,04034,04040,04044,04050,04065,05333,05335,05336,05337};
unsigned rli_cases(unsigned mission){return mission?RLI_CM_CASES:RLI_LM_CASES;}
agc_word rli_entry(unsigned mission,uint16_t n){return mission?cm_z[n/RLI_PER_ENTRY]:lm_z[n/RLI_PER_ENTRY];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
/* A fixed-memory CADR off every lead-in and RESUME path. */
static agc_word cadr(uint32_t *r){uint32_t x=rnd(r);return (agc_word)(x&1u?06000u+(x>>1)%02000u:02000u+(x>>1)%02000u);}
static agc_word bbcon(uint32_t *r){return (agc_word)((rnd(r)%32u)*02000u|((rnd(r)%8u)*020u)|(rnd(r)%8u));}
void rli_input(unsigned mission,apollo_overlay_state *m,rli_io *io,uint16_t n){uint32_t r=0x04000011u+mission*0x9e3779b9u+n*2654435761u;unsigned i;agc_word bb,z=rli_entry(mission,n),*sb,*irq;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=0;m->words[035]=0;m->words[036]=0;m->words[047]=0;for(i=050;i<=054;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 bb=bbcon(&r);m->words[6]=(agc_word)(bb&076007u);m->words[3]=(agc_word)((bb&7u)<<8);m->words[4]=(agc_word)(bb&076000u);m->words[5]=z;
 /* The 2CADRs the DTCB vectors read: LM T5ADR, CM T6LOC and T5LOC. */
 m->words[01274]=cadr(&r);m->words[01275]=bbcon(&r);m->words[01310]=cadr(&r);m->words[01311]=bbcon(&r);m->words[01312]=cadr(&r);m->words[01313]=bbcon(&r);
 /* TIME5 around half a second (062) from overflow, either side. */
 m->words[030]=(agc_word)(rnd(&r)&1u?rnd(&r)%0144u:rnd(&r)&AGC_MASK);
 m->words[015]=(agc_word)(1u+rnd(&r)%010000u);m->words[016]=bbcon(&r);
 memset(io,0,sizeof *io);
 sb=mission?&io->cm.superbank:&io->lm.superbank;irq=mission?&io->cm.interrupt_enabled:&io->lm.interrupt_enabled;
 *sb=(agc_word)((rnd(&r)%8u)*020u);*irq=(agc_word)(rnd(&r)&1u);
 io->hw.in_interrupt=(uint16_t)(z!=04000u);
 /* The interrupt saved the instruction at ZRUPT-1 in BRUPT (#144); in
  * erasable, under the EB the RESUME family restores (BANKRUPT's, A's at
  * 05336, else the current one). */
 {agc_word at=(agc_word)(m->words[015]-1u),eb=z==05333u||z==05335u?(agc_word)(m->words[016]&7u):z==05336u?(agc_word)(agc_correct(m->words[0])&7u):(agc_word)(m->words[3]>>8);
  if(at<01400u)m->words[017]=m->words[at];else if(at<02000u)m->words[017]=m->words[eb*0400u+(at-01400u)];}
 /* A quarter of the CM RESUME family interrupted in banks 030-033 (SUPERBNK 0060) or 040-043 (0100):
  * the restored BB (BANKRUPT, A at 05336, the current one at NOQBRSM) a plain BB, which never holds
  * superbank bits, and BRUPT the rope word at ZRUPT-1 (one in eight not), read through the program's
  * fixed_data as the resident kernel does (#198). */
 if(mission&&z>=05333u&&n%4u==3u){uint16_t i=(uint16_t)(rnd(&r)%RESUME_ROPE_WORDS),ph=cm_resume_rope_physical[i],b=(uint16_t)(ph/1024u);
  agc_word fbx=(agc_word)((b>=040u?b-8u:b)<<10),eb=(agc_word)(rnd(&r)%8u);
  *sb=b>=040u?0100u:0060u;m->words[015]=(agc_word)(02000u+ph%1024u+1u);
  if(z==05337u){m->words[4]=fbx;m->words[6]=(agc_word)(fbx|(m->words[3]>>8));}
  else if(z==05336u)m->words[0]=agc_extend((agc_word)(fbx|eb));
  else m->words[016]=(agc_word)(fbx|eb);
  m->words[017]=(agc_word)(cm_resume_rope_word[i]^(rnd(&r)%8u?0u:1u));}
}
/* The program's fixed words: only the CM RESUME cases' rope words. */
static bool rope(const agc_native_program *p,uint16_t physical,agc_word *word){uint16_t i;(void)p;
 for(i=0;i<RESUME_ROPE_WORDS;++i)if(cm_resume_rope_physical[i]==physical){*word=cm_resume_rope_word[i];return true;}
 return false;}
unsigned rli_run(unsigned mission,apollo_overlay_state *s,rli_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;void *v;
 program.basic=mission?&cm_lead_in_routines:&lm_lead_in_routines;program.basic_count=1;program.hardware=&io->hw;if(mission)program.fixed_data=rope;
 rli_input(mission,s,io,n);v=mission?(void *)&io->cm:(void *)&io->lm;
 call=agc_transfer_resolve(&program,s,mission?io->cm.superbank:io->lm.superbank);
 if(!call)return 3u;
 return call(&program,s,v)==EXEC_OK?RLI_RETURNED:RLI_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t rli_digest(unsigned mission,apollo_overlay_state *s,uint16_t n){static rli_io io;unsigned i,kind=rli_run(mission,s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=RLI_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(mission?io.cm.superbank:io.lm.superbank);mix(mission?io.cm.interrupt_enabled:io.lm.interrupt_enabled);mix(io.hw.in_interrupt);}
 return (uint32_t)sum_b<<16|sum_a;
}
