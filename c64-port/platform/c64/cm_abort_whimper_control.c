/* SPDX-License-Identifier: MIT */
#include "cm_abort_whimper_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_whimper_fixture *f=CM_WHIMPER_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_abort_whimper(s,CM_WHIMPER_IO,&f->in_isr);f->status=(uint16_t)status;return status!=EXEC_OK;
}
