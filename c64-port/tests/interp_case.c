/* SPDX-License-Identifier: MIT */
#include "interp_case.h"
#include "cm_native_image.h"
#include "lm_channel_io.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void irc_build(apollo_overlay_state *m,const irc_case *c){uint32_t seed=c->seed;unsigned i;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[7]=0;for(i=010;i<=061u;++i)m->words[i]=0;
 for(i=0;i<c->nedits;++i)if(!(c->edits[i].addr&IRC_CHANNEL_EDIT))m->words[c->edits[i].addr&03777u]=c->edits[i].word;
}
#ifndef IRC_BUILD_ONLY
unsigned irc_run(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t max_steps,uint16_t *ie){
 lm_channel_io lio;cm_native_io cio;void *io;agc_word superbank=c->superbank;enum exec_status st;uint16_t steps;unsigned i;
 lio.control.superbank=c->superbank;lio.control.interrupt_enabled=c->interrupt_enabled;lio.chan30=037777u;lio.chan31=077777u;
 for(i=0;i<c->nedits;++i)if(c->edits[i].addr&IRC_CHANNEL_EDIT){
  if((c->edits[i].addr&0777u)==030u)lio.chan30=c->edits[i].word;
  if((c->edits[i].addr&0777u)==031u)lio.chan31=c->edits[i].word;}
 {uint16_t *w=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *w;++i)w[i]=0;}
 cio.superbank=c->superbank;cio.interrupt_enabled=c->interrupt_enabled;
 io=c->cm?(void *)&cio:(void *)&lio;
 st=agc_transfer_run(p,s,io,&superbank,0xfffeu,max_steps,&steps);
 if(ie)*ie=c->cm?cio.interrupt_enabled:lio.control.interrupt_enabled;
 return st==EXEC_UNMAPPED?IRC_STOPPED:st==EXEC_OUT_OF_SCOPE&&steps==0u?IRC_REFUSED:IRC_FAILED;
}
#endif
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t irc_digest(unsigned kind,const apollo_overlay_state *s,uint16_t ie){unsigned i;
 sum_a=sum_b=0;mix((uint16_t)kind);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(ie);
 return (uint32_t)sum_b<<16|sum_a;
}
