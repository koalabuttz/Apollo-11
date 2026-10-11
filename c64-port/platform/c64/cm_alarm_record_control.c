/* SPDX-License-Identifier: MIT */
#include "cm_alarm_record_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_alarm_fixture *f=CM_ALARM_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_alarm_record(s,CM_ALARM_IO,(enum cm_alarm_record_entry)(f->source_case%4u),f->literal);f->status=(uint16_t)status;return status!=EXEC_OK;
}
