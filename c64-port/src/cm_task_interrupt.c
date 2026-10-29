/* SPDX-License-Identifier: MIT */
#include "cm_task_interrupt.h"
static int valid(const apollo_overlay_state *s,const cm_native_io *io,const cm_task_interrupt_state *irq){return s!=0&&io!=0&&irq!=0&&s->words[7]==0&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&io->interrupt_enabled<=1u&&(io->superbank&~0160u)==0&&irq->in_isr<=1u&&irq->pending_danzig<=1u;}
enum cm_task_interrupt_status cm_task_interrupt_enter(apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq){agc_word old_lr,old_bb,old_q;
 if(!valid(s,io,irq)||s->words[5]!=06030u||s->words[2]>07777u||s->words[012]>07777u||irq->pending_danzig)return CM_TASK_IRQ_INVALID;
 if(irq->in_isr||!io->interrupt_enabled||agc_overflow(s->words[0]))return CM_TASK_IRQ_DEFERRED;
 old_lr=s->words[011];old_bb=s->words[6];old_q=s->words[2];
 s->words[015]=06031;s->words[017]=030165;s->words[010]=agc_correct(s->words[0]);s->words[011]=agc_correct(s->words[1]);
 s->words[0]=agc_extend((agc_word)(old_bb|io->superbank));s->words[1]=agc_extend(old_lr);s->words[016]=agc_correct(s->words[0]);
 s->words[2]=s->words[012];s->words[012]=old_q;s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=03423;irq->in_isr=1;
 return CM_TASK_IRQ_ENTERED;
}
enum cm_task_interrupt_status cm_task_interrupt_resume(apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq){
 if(!valid(s,io,irq)||!irq->in_isr||irq->pending_danzig||s->words[5]!=05341u||s->words[015]!=06031u||s->words[017]!=030165u)return CM_TASK_IRQ_INVALID;
 s->words[1]=agc_extend(agc_correct(s->words[1]));s->words[5]=06030;irq->in_isr=0;irq->pending_danzig=1;return CM_TASK_IRQ_RESUMED;
}
