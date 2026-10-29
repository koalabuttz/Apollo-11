/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <string.h>
#include "cm_idle_interrupt_cases.h"
static void same(const apollo_overlay_state *s,const apollo_overlay_state *old,const cm_native_io *io,const cm_native_io *oldio,const cm_idle_interrupt_state *irq,const cm_idle_interrupt_state *oldirq){assert(memcmp(s,old,sizeof *s)==0);assert(memcmp(io,oldio,sizeof *io)==0);assert(memcmp(irq,oldirq,sizeof *irq)==0);}
int main(void){apollo_overlay_state s,old;cm_native_io io,oldio;cm_idle_interrupt_state irq,oldirq;unsigned k;
 for(k=0;k<3u;++k){cm_idle_interrupt_input(&s,&io,&irq,0);if(k==0u)io.interrupt_enabled=0;else if(k==1u)irq.in_isr=1;else s.words[0]=040000;old=s;oldio=io;oldirq=irq;assert(cm_idle_interrupt_enter(&s,&io,&irq)==CM_IDLE_IRQ_DEFERRED);same(&s,&old,&io,&oldio,&irq,&oldirq);}
 for(k=0;k<5u;++k){cm_idle_interrupt_input(&s,&io,&irq,2048);if(k==0u)s.words[4]=0;else if(k==1u)s.words[017]^=1;else if(k==2u)s.words[015]^=1;else if(k==3u)irq.pending_idle=1;else irq.in_isr=0;old=s;oldio=io;oldirq=irq;assert(cm_idle_interrupt_resume(&s,&io,&irq)==CM_IDLE_IRQ_INVALID);same(&s,&old,&io,&oldio,&irq,&oldirq);}
 assert(cm_idle_interrupt_enter(0,&io,&irq)==CM_IDLE_IRQ_INVALID);assert(cm_idle_interrupt_enter(&s,0,&irq)==CM_IDLE_IRQ_INVALID);assert(cm_idle_interrupt_enter(&s,&io,0)==CM_IDLE_IRQ_INVALID);return 0;}
