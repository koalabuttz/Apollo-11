/* SPDX-License-Identifier: MIT */
#include "cm_phase_cleanup_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_cleanup_fixture *f=CM_CLEANUP_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_phase_cleanup(s,CM_CLEANUP_IO);f->status=(uint16_t)status;return status!=EXEC_OK;
}
