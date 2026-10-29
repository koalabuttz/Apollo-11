/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_task_interrupt_cases.h"
static apollo_overlay_state s;static cm_native_io io;static cm_task_interrupt_state irq;
int main(void){unsigned n,i;for(n=0;n<CM_TASK_INTERRUPT_CASES;++n){enum cm_task_interrupt_status status;cm_task_interrupt_input(&s,&io,&irq,(uint16_t)n);status=n<512u?cm_task_interrupt_enter(&s,&io,&irq):cm_task_interrupt_resume(&s,&io,&irq);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,irq.in_isr,irq.pending_danzig);}return 0;}
