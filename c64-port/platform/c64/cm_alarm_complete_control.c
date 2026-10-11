/* SPDX-License-Identifier: MIT */
#include "cm_alarm_complete_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_alarm_complete_fixture *f=CM_ALARM_COMPLETE_FIXTURE;enum exec_status status;
 if(selector==0u){f->fixed_word=0;f->needs_data=(uint16_t)cm_alarm_fixed_address(s,CM_ALARM_COMPLETE_IO,&f->fixed_address);return 0;}
 if(selector!=1u)return 1;status=cm_alarm_complete(s,CM_ALARM_COMPLETE_IO,(enum cm_alarm_record_entry)(f->source_case%4u),f->fixed_word);f->status=(uint16_t)status;return status==EXEC_OUT_OF_SCOPE;
}
