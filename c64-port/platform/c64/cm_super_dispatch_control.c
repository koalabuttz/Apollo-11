/* SPDX-License-Identifier: MIT */
#include "cm_super_dispatch_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_super_fixture *f=CM_SUPER_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_super_dispatch(s,CM_SUPER_IO,&f->next);f->status=(uint16_t)status;return status!=EXEC_OK&&status!=EXEC_IDLE;
}
