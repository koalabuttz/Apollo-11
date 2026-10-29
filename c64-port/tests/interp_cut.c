/* SPDX-License-Identifier: MIT */
#include "interp_cut.h"
#include "cm_native_image.h"
/* (The io records, input latches and channels irc_run gives the routines;
 * a one-mission C64 build carries only its mission's.) */
#if !defined(IRC_CM_ONLY)
/* (An LM cut seeds and saves the case's channels only on the host and in
 * C64 builds made with -DIRC_LM_HW, as LLGE's are: irc_lm_io costs about
 * 1 KB, which the P40-P47 groups don't have below the mailbox.) */
#if IRC_IO && (!defined(__mos__) || defined(IRC_LM_HW))
#define IRC_LM_CUT_IO 1
#else
#define IRC_LM_CUT_IO 0
#endif
static enum exec_status lm_cut(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,agc_word *sb,
                               uint16_t cut,uint16_t max_steps,uint16_t *ie){
 lm_channel_io lio;enum exec_status st;
 lio.io.superbank=c->superbank;lio.io.interrupt_enabled=c->interrupt_enabled;
#if IRC_INPUTS_ON
 irc_inputs(c,&lio.in);
#endif
#if IRC_LM_CUT_IO
 if(p->hardware){agc_hardware z={0};*p->hardware=z;}if(irc_ch_on)irc_lm_io(p->hardware,&lio.in,0);
#endif
#if IRC_ISR_ON
 if(irc_isr_on&&p->hardware)p->hardware->in_interrupt=irc_isr;
#endif
 st=agc_transfer_run(p,s,&lio,sb,cut,max_steps,0);
#if IRC_ISR_ON
 if(irc_isr_on&&p->hardware)irc_isr=p->hardware->in_interrupt;
#endif
#if IRC_LM_CUT_IO
 if(irc_ch_on)irc_lm_io(p->hardware,&lio.in,1);
#endif
 if(ie)*ie=lio.io.interrupt_enabled;
 return st;
}
#endif
#if !defined(IRC_LM_ONLY)
static enum exec_status cm_cut(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,agc_word *sb,
                               uint16_t cut,uint16_t max_steps,uint16_t *ie){
 cm_channel_io cio;agc_native_program q;enum exec_status st;unsigned i;
 (void)q;{uint16_t *w=(uint16_t *)&cio.io;for(i=0;i<sizeof cio.io/sizeof *w;++i)w[i]=0;}
 cio.io.superbank=c->superbank;cio.io.interrupt_enabled=c->interrupt_enabled;
#if IRC_INPUTS_ON
 irc_inputs(c,&cio.in);
#endif
#if IRC_IO
 p=irc_cm_begin(p,&q,&cio.io);
#endif
 st=agc_transfer_run(p,s,&cio,sb,cut,max_steps,0);
#if IRC_IO
 if(irc_ch_on)irc_cm_io(&cio.io,p->hardware,1);
#endif
 if(ie)*ie=cio.io.interrupt_enabled;
 return st;
}
#endif
unsigned irc_run_cut(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t cut,uint16_t max_steps,uint16_t *ie){
 agc_word superbank=c->superbank;enum exec_status st;
 if(!cut)return irc_run(p,s,c,max_steps,ie);
#if defined(IRC_LM_ONLY)
 st=lm_cut(p,s,c,&superbank,cut,max_steps,ie);
#elif defined(IRC_CM_ONLY)
 st=cm_cut(p,s,c,&superbank,cut,max_steps,ie);
#else
 st=c->cm?cm_cut(p,s,c,&superbank,cut,max_steps,ie):lm_cut(p,s,c,&superbank,cut,max_steps,ie);
#endif
 /* (EXEC_OK: the trampoline stopped with the point at the cut) */
 return st==EXEC_OK&&agc_fixed_physical(s->words[5],s->words[4],superbank)==p->interpreter_fetch?IRC_STOPPED:IRC_FAILED;
}
