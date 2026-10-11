/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_task_interrupt_check.h"
#include "cm_task_interrupt_cases.h"
static apollo_overlay_state s,before;static cm_native_io io,oldio;static cm_task_interrupt_state irq,oldirq;
int main(void){unsigned fail=cm_task_interrupt_check(),n;
 for(n=0;n<3u;++n){cm_task_interrupt_input(&s,&io,&irq,0);if(n==0)io.interrupt_enabled=0;else if(n==1)irq.in_isr=1;else s.words[0]=040000;before=s;oldio=io;oldirq=irq;
  if(cm_task_interrupt_enter(&s,&io,&irq)!=CM_TASK_IRQ_DEFERRED||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io)||memcmp(&irq,&oldirq,sizeof irq))++fail;}
 printf("1024 original CM automatic interrupt/RESUME full-image states and deferral guards: %s\n",fail?"FAIL":"PASS");return fail?1:0;
}
