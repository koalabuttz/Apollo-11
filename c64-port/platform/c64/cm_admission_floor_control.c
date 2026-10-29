/* SPDX-License-Identifier: MIT */
#include "cm_admission_floor_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_floor_fixture *f=CM_FLOOR_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_admission_floor(s,CM_FLOOR_IO,(enum cm_admission_entry)(f->source_case%4u),(exec_address){s->words[0705],s->words[0706]});f->status=(uint16_t)status;return status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE;
}
