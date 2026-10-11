/* SPDX-License-Identifier: MIT */
#include "cm_priority_complete_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_prio_fixture *f=CM_PRIO_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_priority_complete(s,CM_PRIO_IO,&f->next);f->status=(uint16_t)status;return status!=EXEC_OK;
}
