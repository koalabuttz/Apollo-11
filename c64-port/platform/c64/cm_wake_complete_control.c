/* SPDX-License-Identifier: MIT */
#include "cm_wake_complete_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_wake_fixture *f=CM_WAKE_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_wake_complete(s,CM_WAKE_IO,&f->next);f->status=(uint16_t)status;return status!=EXEC_OK&&status!=EXEC_NOT_FOUND;
}
