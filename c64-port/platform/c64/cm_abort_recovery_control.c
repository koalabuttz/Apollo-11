/* SPDX-License-Identifier: MIT */
#include "cm_abort_recovery_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_recovery_fixture *f=CM_RECOVERY_FIXTURE;if(selector!=f->phase||selector>6u)return 1;f->status=(uint16_t)cm_abort_recovery_stage(s,CM_RECOVERY_IO,(uint8_t)selector);return f->status!=EXEC_OK;}
