/* SPDX-License-Identifier: MIT */
#include "lm_phase_table_cases.h"
#include "lm_interbank_routines.h"
#define LM_PHASE_SITE_LIST
#include "../src/lm_phase_sites.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
uint16_t lm_pt_site_count(void){return (uint16_t)(LM_PHASE_SITE_COUNT+2u);}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static const agc_word entry[5]={LM_PT_NEWMODEX,LM_PT_CHECKMM,LM_PT_PHASCHNG,LM_PT_2PHSCHNG,LM_PT_PHSCHNGA};
/* The phase words FRESH_START_AND_RESTART's V37RET hands PHSCHNGA in A:
 * 2.0SPT (OCT37667+BIT5+OCT40072), 2.11SPT (BIT5+OCT40072) and 2.7SPT. */
static const agc_word phschnga_a[3]={02u,0140112u,0140072u};
void lm_pt_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 unsigned i,k=n%lm_pt_site_count(),bank,eb;uint32_t seed=0x5412u+n*7u;agc_word fb,q;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;for(i=010;i<=061;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {p66_bias_io z={0};*io=z;}
 io->interrupt_enabled=(agc_word)((rnd(&seed)>>20)&1u);
 if(k<LM_PHASE_SITE_COUNT){bank=lm_phase_site_list[k].physical/1024u;q=(agc_word)(02000u+lm_phase_site_list[k].physical%1024u);s->words[5]=entry[lm_phase_site_list[k].callee];}
 else{bank=2u+(rnd(&seed)>>20)%036u;q=(agc_word)(02000u+(rnd(&seed)>>16)%01776u);s->words[5]=k==LM_PHASE_SITE_COUNT?LM_PT_NEWMODEA:LM_PT_MMDSPLAY;}
 if(bank>=040u){fb=(agc_word)((bank-010u)<<10);io->superbank=0100;}
 else{fb=(agc_word)(bank<<10);if(bank>=030u)io->superbank=(agc_word)(((rnd(&seed)>>20)&1u)?0060u:0u);}
 eb=(rnd(&seed)>>20)&7u;
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[2]=q;
 /* NEWMODEA and PHSCHNGA take the mode or phase word in A; CHECKMM half the time finds it in MODREG. */
 if(s->words[5]==LM_PT_NEWMODEA)s->words[0]=(agc_word)((rnd(&seed)>>20)&0177u);
 if(s->words[5]==LM_PT_PHSCHNGA)s->words[0]=phschnga_a[(n/lm_pt_site_count())%3u];
 if(s->words[5]==LM_PT_CHECKMM&&(n&1u)){agc_word mode=0;agc_native_program p={0,0,0,0,lm_phase_sites,LM_PHASE_SITE_WORDS,AGC_TRANSFER_NONE,0,0,0,0,0};
  (void)agc_transfer_operand(&p,s,io->superbank,q,&mode);s->words[01011]=mode;}
}
static enum exec_status novac(const agc_native_program *p,apollo_overlay_state *s,void *io){lm_pt_log *log=p->user;agc_word q=s->words[2];
 if(log->calls)return EXEC_OUT_OF_SCOPE;
 log->calls=1;log->a=s->words[0];log->l=s->words[1];log->q=q;
 if(!agc_transfer_operand(p,s,((p66_bias_io *)io)->superbank,q,&log->cadr)||!agc_transfer_operand(p,s,((p66_bias_io *)io)->superbank,(agc_word)(q+1u),&log->bbcon))return EXEC_OUT_OF_SCOPE;
 s->words[5]=(agc_word)(q+2u);return EXEC_OK;
}
static const agc_routine_entry stubs[1]={{FF(LM_PT_NOVAC),novac}};
enum exec_status lm_pt_run(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,lm_pt_log *log,uint16_t *steps){
 agc_routine_table tables[3];agc_native_program program={0,0,0,0,lm_phase_sites,LM_PHASE_SITE_WORDS,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=lm_phase_table_routines;tables[1]=lm_interbank_routines;tables[2].entries=stubs;tables[2].count=1;
 program.basic=tables;program.basic_count=3;program.user=log;
 lm_pt_input(s,io,n);log->calls=0;log->a=log->l=log->q=log->cadr=log->bbcon=0;
 /* No stop inside the run: it ends at the first point no table owns. */
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,16,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_pt_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){lm_pt_log log;p66_bias_io io;uint16_t steps=0;unsigned i;
 *status=lm_pt_run(s,&io,n,&log,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(log.calls);if(log.calls){mix(log.a);mix(log.l);mix(log.q);mix(log.cadr);mix(log.bbcon);}
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
