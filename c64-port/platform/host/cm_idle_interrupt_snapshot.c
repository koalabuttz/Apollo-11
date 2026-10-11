/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_idle_interrupt_cases.h"
#include "../../tools/cm_idle_interrupt_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_IDLE_INTERRUPT_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_idle_interrupt_state irq;enum cm_idle_interrupt_status status;cm_idle_interrupt_input(&s,&io,&irq,n);status=n<2048u?cm_idle_interrupt_enter(&s,&io,&irq):cm_idle_interrupt_resume(&s,&io,&irq);if(status>CM_IDLE_IRQ_RESUMED)return 2;cm_idle_interrupt_dump(n,(uint16_t)status,&s,&io,&irq);}return 0;}
