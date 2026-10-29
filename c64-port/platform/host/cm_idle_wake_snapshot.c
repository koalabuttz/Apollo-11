/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_idle_wake_cases.h"
#include "../../tools/cm_idle_interrupt_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_IDLE_WAKE_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_idle_interrupt_state irq;exec_transfer next;unsigned p;cm_idle_wake_input(&s,&io,&irq,n);for(p=0;p<9u;++p){if(!cm_idle_wake_step(&s,&io,&irq,&next,(uint16_t)p))return 2;cm_idle_interrupt_dump(n*9u+p,0,&s,&io,&irq);}}return 0;}
