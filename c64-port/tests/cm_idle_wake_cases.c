/* SPDX-License-Identifier: MIT */
#include "cm_idle_wake_cases.h"
#include "cm_idle_interrupt_cases.h"
#include "cm_waitlist.h"
void cm_idle_wake_input(apollo_overlay_state *s,cm_native_io *io,cm_idle_interrupt_state *irq,uint32_t n){cm_waitlist w;unsigned i;agc_word slot=(agc_word)(n&7u);
 cm_idle_interrupt_input(s,io,irq,n);s->words[3]=03000;s->words[4]=0;s->words[6]=6;s->words[5]=05175;s->words[0]=agc_extend(051332);s->words[2]=03761;s->words[012]=(agc_word)(0100u+n);s->words[0067]=0;s->words[0167]=023110;s->words[0166]=0110;
 for(i=1;i<7u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;
 io->superbank=060;io->executive_alarm=io->waitlist_alarm=0;io->interrupt_enabled=(agc_word)(n&1u);
 cm_wl_init(&w);w.time3=0;w.task[0]=(exec_address){03761,slot};cm_native_commit_waitlist(s,io,&w);s->words[01141u+slot]=051332;
}
