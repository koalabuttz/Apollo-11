/* SPDX-License-Identifier: MIT */
#include "cm_r52_priority_alarm_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){exec_transfer next;enum exec_status status=cm_r52_priority_alarm_step(s,CM_R52_PRIORITY_ALARM_IO,selector,&next);CM_R52_PRIORITY_ALARM_FIXTURE->status=(uint16_t)status;return status==EXEC_OUT_OF_SCOPE;}
