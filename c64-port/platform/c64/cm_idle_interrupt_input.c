/* SPDX-License-Identifier: MIT */
#include "cm_idle_interrupt_binding.h"
#include "cm_idle_interrupt_cases.h"
#define CM_IDLE_INDEX_ONLY 1
#include "cm_idle_interrupt_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_fixture *f=CM_IDLE_FIXTURE;
 if(selector>=CM_IDLE_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_idle_indices[selector];f->status=f->step=f->failures=0;cm_idle_interrupt_input(s,CM_IDLE_IO,&f->irq,f->source_case);return 0;
}
