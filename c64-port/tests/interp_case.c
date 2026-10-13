/* SPDX-License-Identifier: MIT */
#include "interp_case.h"
#include "cm_native_image.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
const uint8_t irc_ch_num[IRC_NCH]={05,06,011,012,013,014,030,031,032,033};
agc_word irc_ch[IRC_NCH],irc_trap[3];
uint8_t irc_ch_on;
#if IRC_IO && !defined(IRC_BUILD_ONLY) && !defined(IRC_LM_ONLY)
/* (cm_channels: src/cm_interp_orders.c, which LM-only C64 builds leave out;
 * LM host tests link no CM code, so they take this weak copy.) */
__attribute__((weak)) cm_native_channels cm_channels;
void irc_cm_io(void *cm_io,unsigned save){cm_native_io *io=(cm_native_io *)cm_io;
 agc_word *f[IRC_NCH];unsigned i;
 f[0]=&cm_channels.channel05;f[1]=&cm_channels.channel06;f[2]=&io->channel11;f[3]=&io->channel12;f[4]=&io->channel13;
 f[5]=&io->channel14;f[6]=&cm_channels.channel30;f[7]=&cm_channels.channel31;f[8]=&cm_channels.channel32;f[9]=&cm_channels.channel33;
 for(i=0;i<IRC_NCH;++i){if(save)irc_ch[i]=*f[i];else *f[i]=irc_ch[i];}
 if(save){irc_trap[0]=io->trap31a;irc_trap[1]=io->trap31b;irc_trap[2]=io->trap32;}
 else{io->trap31a=irc_trap[0];io->trap31b=irc_trap[1];io->trap32=irc_trap[2];}
}
#endif
void irc_build(apollo_overlay_state *m,const irc_case *c){uint32_t seed=c->seed;unsigned i,k;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[7]=0;for(i=010;i<=061u;++i)m->words[i]=0;
#if IRC_IO
 for(i=0;i<c->nedits;++i)if(c->edits[i].addr==IRC_COUNTERS)for(k=010;k<=061u;++k)m->words[k]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 irc_ch_on=0;for(k=0;k<IRC_NCH;++k)irc_ch[k]=0;for(k=0;k<3u;++k)irc_trap[k]=0;
 for(i=0;i<c->nedits;++i){
  if(c->edits[i].addr&IRC_CHANNEL){
   for(k=0;k<IRC_NCH;++k)if(irc_ch_num[k]==(c->edits[i].addr&0777u)){irc_ch[k]=c->edits[i].word;irc_ch_on=1;}
  }else if(c->edits[i].addr!=IRC_COUNTERS)m->words[c->edits[i].addr&03777u]=c->edits[i].word;
 }
#else
 (void)k;for(i=0;i<c->nedits;++i)m->words[c->edits[i].addr&03777u]=c->edits[i].word;
#endif
}
#ifndef IRC_BUILD_ONLY
unsigned irc_run(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t max_steps,uint16_t *ie){
 p66_bias_io lio;cm_native_io cio;void *io;agc_word superbank=c->superbank;enum exec_status st;uint16_t steps;unsigned i;
 lio.superbank=c->superbank;lio.interrupt_enabled=c->interrupt_enabled;
 {uint16_t *w=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *w;++i)w[i]=0;}
 cio.superbank=c->superbank;cio.interrupt_enabled=c->interrupt_enabled;
#if IRC_IO && !defined(IRC_LM_ONLY)
 {uint16_t *w=(uint16_t *)&cm_channels;for(i=0;i<sizeof cm_channels/sizeof *w;++i)w[i]=0;}
 if(irc_ch_on)irc_cm_io(&cio,0);
#endif
 io=c->cm?(void *)&cio:(void *)&lio;
 st=agc_transfer_run(p,s,io,&superbank,0xfffeu,max_steps,&steps);
#if IRC_IO && !defined(IRC_LM_ONLY)
 if(irc_ch_on)irc_cm_io(&cio,1);
#endif
 if(ie)*ie=c->cm?cio.interrupt_enabled:lio.interrupt_enabled;
 return st==EXEC_UNMAPPED?IRC_STOPPED:st==EXEC_OUT_OF_SCOPE&&steps==0u?IRC_REFUSED:IRC_FAILED;
}
#endif
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t irc_digest(unsigned kind,const apollo_overlay_state *s,uint16_t ie){unsigned i;
 sum_a=sum_b=0;mix((uint16_t)kind);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(ie);
#if IRC_IO
 if(irc_ch_on){for(i=0;i<IRC_NCH;++i)mix(irc_ch[i]);for(i=0;i<3u;++i)mix(irc_trap[i]);}
#endif
 return (uint32_t)sum_b<<16|sum_a;
}
