/* SPDX-License-Identifier: MIT */
#include "cm_admission_complete_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_admit_fixture *f=CM_ADMIT_FIXTURE;enum exec_status status;
 if(selector>=4u)return 1;status=cm_admission_complete(s,CM_ADMIT_IO,(enum cm_admission_entry)selector,f->job);f->status=(uint16_t)status;return status!=EXEC_OK&&status!=EXEC_NO_CORE&&status!=EXEC_NO_VAC;
}
