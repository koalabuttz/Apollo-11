/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_lifecycle_fixture *f=CM_LIFECYCLE_FIXTURE;enum exec_status status;if(selector<5u||selector>11u)return 1;status=cm_abort_recovery_stage(s,CM_LIFECYCLE_IO,(uint8_t)(selector-5u));f->status=(uint16_t)status;return status!=EXEC_OK;}
