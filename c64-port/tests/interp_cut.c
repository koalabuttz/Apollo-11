/* SPDX-License-Identifier: MIT */
#include "interp_cut.h"
#include "cm_native_image.h"
unsigned irc_run_cut(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t cut,uint16_t max_steps,uint16_t *ie){
 p66_bias_io lio;cm_native_io cio;void *io;agc_word superbank=c->superbank;enum exec_status st;unsigned i;
 if(!cut)return irc_run(p,s,c,max_steps,ie);
 lio.superbank=c->superbank;lio.interrupt_enabled=c->interrupt_enabled;
 {uint16_t *w=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *w;++i)w[i]=0;}
 cio.superbank=c->superbank;cio.interrupt_enabled=c->interrupt_enabled;
#if IRC_IO && !defined(IRC_LM_ONLY)
 {uint16_t *w=(uint16_t *)&cm_channels;for(i=0;i<sizeof cm_channels/sizeof *w;++i)w[i]=0;}
 if(irc_ch_on)irc_cm_io(&cio,0);
#endif
 io=c->cm?(void *)&cio:(void *)&lio;
 st=agc_transfer_run(p,s,io,&superbank,cut,max_steps,0);
#if IRC_IO && !defined(IRC_LM_ONLY)
 if(irc_ch_on)irc_cm_io(&cio,1);
#endif
 if(ie)*ie=c->cm?cio.interrupt_enabled:lio.interrupt_enabled;
 /* (EXEC_OK: the trampoline stopped with the point at the cut) */
 return st==EXEC_OK&&agc_fixed_physical(s->words[5],s->words[4],superbank)==p->interpreter_fetch?IRC_STOPPED:IRC_FAILED;
}
