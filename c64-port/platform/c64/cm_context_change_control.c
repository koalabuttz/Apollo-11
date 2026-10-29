/* SPDX-License-Identifier: MIT */
#include "cm_context_change_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_ctx_fixture *f=CM_CTX_FIXTURE;enum exec_status status;
 if(selector>=2u)return 1;status=cm_context_change(s,CM_CTX_IO,(uint8_t)selector,&f->next);f->status=(uint16_t)status;return status!=EXEC_OK;
}
