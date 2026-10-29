/* SPDX-License-Identifier: MIT */
#include "cm_priority_full_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_prio_full_fixture *f=CM_PRIO_FULL_FIXTURE;if(selector)return 1;f->status=(uint16_t)cm_priority_full(s,CM_PRIO_FULL_IO,&f->next);return f->status!=EXEC_OK;}
