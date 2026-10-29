/* SPDX-License-Identifier: MIT */
#include "cm_idle_interrupt_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_fixture *f=CM_IDLE_FIXTURE;enum cm_idle_interrupt_status status;
 if(selector!=0u)return 1;status=f->source_case<2048u?cm_idle_interrupt_enter(s,CM_IDLE_IO,&f->irq):cm_idle_interrupt_resume(s,CM_IDLE_IO,&f->irq);f->status=(uint16_t)status;return status>CM_IDLE_IRQ_RESUMED;
}
