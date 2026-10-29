/* SPDX-License-Identifier: MIT */
#include "cm_admission_zero_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_admission_zero_fixture *f=CM_ADMISSION_ZERO_FIXTURE;enum exec_status status;if(selector)return 1;status=cm_admission_zero(s,CM_ADMISSION_ZERO_IO,(enum cm_admission_entry)(f->source_case%4u),f->job);f->status=(uint16_t)status;return status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE;}
