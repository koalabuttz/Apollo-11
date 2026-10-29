/* SPDX-License-Identifier: MIT */
#include "cm_vac5_store_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_vac5_fixture *f=CM_VAC5_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_vac5_store(s,CM_VAC5_IO);f->status=(uint16_t)status;return status!=EXEC_OK;
}
