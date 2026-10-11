/* SPDX-License-Identifier: MIT */
#include "cm_priolarm_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_priolarm_fixture *f=CM_PRIOLARM_FIXTURE;enum cm_priolarm_result status;
 if(selector!=0u)return 1;status=cm_priolarm(s,CM_PRIOLARM_IO);f->status=(uint16_t)status;return status==CM_PRIOLARM_OUT_OF_SCOPE;
}
