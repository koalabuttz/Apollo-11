/* SPDX-License-Identifier: MIT */
#include "cm_idle_recheck_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_recheck_fixture *f=CM_IDLE_RECHECK_FIXTURE;if(selector)return 1;f->status=(uint16_t)cm_idle_recheck(s,CM_IDLE_RECHECK_IO,&f->next);return f->status!=EXEC_OK;}
