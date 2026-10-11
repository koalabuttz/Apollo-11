/* SPDX-License-Identifier: MIT */
#include "cm_idle_wake_binding.h"
#include "cm_idle_wake_cases.h"
#define CM_IDLE_WAKE_INDEX_ONLY 1
#include "cm_idle_wake_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_wake_fixture *f=CM_IDLE_WAKE_FIXTURE;if(selector>=8u)return 1;f->run=selector;f->phase=f->failures=0;f->source_case=cm_idle_wake_indices[selector];cm_idle_wake_input(s,CM_IDLE_WAKE_IO,&f->irq,f->source_case);return 0;}
