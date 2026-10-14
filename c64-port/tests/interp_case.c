/* SPDX-License-Identifier: MIT */
#include "interp_case.h"
#include "cm_native_image.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
const uint8_t irc_ch_num[IRC_NCH]={05,06,011,012,013,014,030,031,032,033};
agc_word irc_ch[IRC_NCH],irc_trap[3];
uint8_t irc_ch_on;
#if IRC_IO && !defined(IRC_BUILD_ONLY) && !defined(IRC_LM_ONLY)
/* Comanche ('channels': 'all'): 011-014 and the traps in the run's
 * cm_native_io, 5, 6 and 030-033 in the program's agc_hardware. */
void irc_cm_io(void *cm_io,agc_hardware *h,unsigned save){cm_native_io *io=(cm_native_io *)cm_io;
 agc_word *f[IRC_NCH];unsigned i;
 f[0]=&h->channel5;f[1]=&h->channel6;f[2]=&io->channel11;f[3]=&io->channel12;f[4]=&io->channel13;
 f[5]=&io->channel14;f[6]=&h->channel30;f[7]=&h->channel31;f[8]=&h->channel32;f[9]=&h->channel33;
 for(i=0;i<IRC_NCH;++i){if(save)irc_ch[i]=*f[i];else *f[i]=irc_ch[i];}
 if(save){irc_trap[0]=io->trap31a;irc_trap[1]=io->trap31b;irc_trap[2]=io->trap32;}
 else{io->trap31a=irc_trap[0];io->trap31b=irc_trap[1];io->trap32=irc_trap[2];}
}
/* A CM run's program and channels: zeroed agc_hardware (the harness's when
 * the program has none: q, a copy of p, takes it), then the case's
 * channels. Returns the program to run. */
const agc_native_program *irc_cm_begin(const agc_native_program *p,agc_native_program *q,void *cm_io){
 static agc_hardware cm_hw;agc_hardware z={0};
 if(!p->hardware){*q=*p;q->hardware=&cm_hw;p=q;}
 *p->hardware=z;if(irc_ch_on)irc_cm_io(cm_io,p->hardware,0);
 return p;
}
#endif
#if IRC_IO && !defined(IRC_BUILD_ONLY) && !defined(IRC_CM_ONLY)
/* Luminary ('channels': 'all', include/agc_channel_ops.h): the outputs in
 * the program's agc_hardware (h may be NULL), 030-033 in the run's latched
 * inputs, the traps in channel 013's bits 12-14 there. */
static void irc_lm_io(agc_hardware *h,agc_input_channels *in,unsigned save){
 static agc_word none[6];agc_word *f[IRC_NCH];unsigned i;
 for(i=0;i<6u;++i)f[i]=&none[i];
 if(h){f[0]=&h->channel5;f[1]=&h->channel6;f[2]=&h->channel11;f[3]=&h->channel12;f[4]=&h->channel13;f[5]=&h->channel14;}
 f[6]=&in->channel30;f[7]=&in->channel31;f[8]=&in->channel32;f[9]=&in->channel33;
 for(i=0;i<IRC_NCH;++i){if(save)irc_ch[i]=*f[i];else *f[i]=irc_ch[i];}
 if(save){for(i=0;i<3u;++i)irc_trap[i]=(agc_word)((irc_ch[4]>>(11u+i))&1u);irc_ch[4]&=043777u;}
 else for(i=0;i<3u;++i)if(irc_trap[i])*f[4]|=(agc_word)(1u<<(11u+i));
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
#if IRC_INPUTS_ON
void irc_inputs(const irc_case *c,agc_input_channels *in){uint32_t seed=c->seed^0x5ca1e5u;
 in->channel3=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 in->channel4=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 if(((rnd(&seed)>>20)&3u)==0u)in->channel4=037777u;
 in->channel30=(agc_word)((rnd(&seed)>>12)&AGC_MASK);in->channel31=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 in->channel32=(agc_word)((rnd(&seed)>>12)&AGC_MASK);in->channel33=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 /* (Drawn last, so the others are as before channel 16 was added.) */
 in->channel16=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
}
#endif
#ifndef IRC_BUILD_ONLY
unsigned irc_run(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t max_steps,uint16_t *ie){
 lm_channel_io lio;cm_channel_io cio;agc_native_program q;void *io;agc_word superbank=c->superbank;enum exec_status st;uint16_t steps;unsigned i;
 (void)q;lio.io.superbank=c->superbank;lio.io.interrupt_enabled=c->interrupt_enabled;
 {uint16_t *w=(uint16_t *)&cio.io;for(i=0;i<sizeof cio.io/sizeof *w;++i)w[i]=0;}
 cio.io.superbank=c->superbank;cio.io.interrupt_enabled=c->interrupt_enabled;
#if IRC_INPUTS_ON
 irc_inputs(c,&lio.in);cio.in=lio.in;
#endif
#if IRC_IO && !defined(IRC_LM_ONLY)
 if(c->cm)p=irc_cm_begin(p,&q,&cio.io);
#endif
#if IRC_IO && !defined(IRC_CM_ONLY)
 if(!c->cm){if(p->hardware){agc_hardware z={0};*p->hardware=z;}if(irc_ch_on)irc_lm_io(p->hardware,&lio.in,0);}
#endif
 io=c->cm?(void *)&cio:(void *)&lio;
 st=agc_transfer_run(p,s,io,&superbank,0xfffeu,max_steps,&steps);
#if IRC_IO && !defined(IRC_LM_ONLY)
 if(irc_ch_on&&c->cm)irc_cm_io(&cio.io,p->hardware,1);
#endif
#if IRC_IO && !defined(IRC_CM_ONLY)
 if(irc_ch_on&&!c->cm)irc_lm_io(p->hardware,&lio.in,1);
#endif
 if(ie)*ie=c->cm?cio.io.interrupt_enabled:lio.io.interrupt_enabled;
 return st==EXEC_UNMAPPED||st==EXEC_IDLE?IRC_STOPPED:st==EXEC_OUT_OF_SCOPE&&steps==0u?IRC_REFUSED:IRC_FAILED;
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
